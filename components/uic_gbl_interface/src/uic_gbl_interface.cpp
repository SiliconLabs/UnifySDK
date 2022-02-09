/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "uic_gbl_interface.h"
#include "sl_log.h"
#include "clock.h"

#include <unistd.h>
#include <fstream>
#include <sstream>
#include <poll.h>

constexpr const char *LOG_TAG = "uic_gbl_interface";

constexpr uint8_t SOH = 0x01; /*   Start of Header  */
constexpr uint8_t EOT = 0x04; /*   End of Transmission */
constexpr uint8_t ACK = 0x06; /*   Acknowledge  */
constexpr uint8_t NAK = 0x15; /*   Not Acknowledge */
constexpr uint8_t CAN
  = 0x18; /*   Cancel (Force receiver to start sending C's) */
constexpr uint8_t C      = 0x43; /*   ASCII "C" */
constexpr int block_size = 128;

/**
 * @brief Check if there is data on the file descriptor
 * 
 * @param fd file descriptor to check
 * @param timeout timeout in milliseconds
 * @return true Data is available
 * @return false timeout
 */
static bool has_data(int fd, int timeout)
{
  struct pollfd p;

  p.fd     = fd;
  p.events = POLLIN;
  poll(&p, 1, timeout);
  return p.revents & POLLIN;
}

/**
 * @brief Get all data from the stream until there is a break defined by a timeout
 * 
 * @param serial_fd File descriptor to check
 * @param timeout   Timeout in milliseconds
 * @return std::string 
 */
static std::string read_timeout(int serial_fd, int timeout)
{
  std::stringstream ss;
  while (has_data(serial_fd, timeout)) {
    char c;
    read(serial_fd, &c, 1);
    ss << c;
  }
  return ss.str();
}

/* Calculate the CITT checksum from the XModem protocol */
static uint16_t calcrc(const char *ptr, int count)
{
  int crc;
  char i;
  crc = 0;
  while (--count >= 0) {
    crc = crc ^ (int)*ptr++ << 8;
    i   = 8;
    do {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc = crc << 1;
    } while (--i);
  }
  return crc;
}

/**
 * @brief Send a single block of data
 * 
 * This functions sends a block of 128 bytes from th gbl_file
 * Please note that the data is padded in the end to the 128 block size.
 * the padding data will be random, but this is just how the XMODEM protocol works
 * 
 * @param serial_fd Serial file descriptor
 * @param block_n Block number to send
 * @param gbl_file file to read the 128 byte block from 
 * @return size_t returns how many bytes has actually been sent, 0 if we are at the end of the file.
 */
static size_t send_block(int serial_fd, int block_n, std::ifstream &gbl_file)
{
  if (gbl_file.eof()) {
    return 0;
  }

  char data[block_size + 5];
  data[0] = SOH;
  data[1] = (block_n)&0xff;
  data[2] = (~block_n) & 0xff;
  gbl_file.read(&data[3], block_size);
  uint16_t crc             = calcrc(&data[3], block_size);
  data[3 + block_size]     = (crc >> 8) & 0xff;
  data[3 + block_size + 1] = (crc >> 0) & 0xff;
  write(serial_fd, data, block_size + 5);
  size_t data_len = gbl_file.gcount();
  return data_len;
}

/**
 * @brief Perform an xmodem transfers
 * 
 * @param gbl_filename File to send
 * @param serial_fd     serial file handle
 * @param progress_callback Progress callback which is called every time we have have sent a 1kb 
 * @return sl_status_t SL_STATUS_OK on succsess
 */

static sl_status_t
  xmodem_transfer(const char *gbl_filename,
                  int serial_fd,
                  uic_gbl_interface_transfer_progress_t progress_callback)
{
  using namespace std;
  enum class XMODEMState { WAIT_FOR_C, SEND_BLOCK, DONE } state;
  int retry = 0;
  int block = 1;

  ifstream gbl(gbl_filename, ifstream::ate | ifstream::binary);
  if (!gbl.is_open()) {
    sl_log_error(LOG_TAG, "Unable to open GBL file");
    return SL_STATUS_NOT_FOUND;
  }

  //Get the file size, this is only for the purpose of the progress callback
  size_t file_size = gbl.tellg();
  gbl.seekg(0, gbl.beg);  //Set the file pointer to the beginning

  state = XMODEMState::WAIT_FOR_C;

  while (true) {
    char cmd;

    if (!has_data(serial_fd, 1000)) {
      sl_log_error(LOG_TAG, "Timeout waiting for character");
      return SL_STATUS_TIMEOUT;
    }

    int len = read(serial_fd, &cmd, 1);
    //Did we receive anything ?
    if (len == 0) {
      continue;
    }

    switch (cmd) {
      case C:
        if (state == XMODEMState::WAIT_FOR_C) {
          state = XMODEMState::SEND_BLOCK;
          block = 1;
        } else {
          sl_log_error(LOG_TAG, "Got unexpected C");
          return SL_STATUS_ABORT;
        }
        break;
      case CAN:
        sl_log_error(LOG_TAG, "The bootloader rejected the image");
        return SL_STATUS_ABORT;
      case NAK:
        sl_log_debug(LOG_TAG, "Target sent NACK");
        state = XMODEMState::SEND_BLOCK;
        retry++;
        break;
      case ACK:
        if (state == XMODEMState::DONE) {
          return SL_STATUS_OK;
        } else {
          state = XMODEMState::SEND_BLOCK;
          retry = 0;
          block++;
        }
        break;
      default:
        break;
    }

    if (retry > 2) {
      sl_log_error(LOG_TAG, "Maximum number for retranmissions exceeded");
      break;
    }

    if (state == XMODEMState::SEND_BLOCK) {
      //Send progress callback after 1kb of transmitted data
      if (progress_callback && ((block & 7) == 1)) {
        progress_callback((block - 1) * block_size, file_size);
      }

      if (send_block(serial_fd, block, gbl) == 0) {
        sl_log_info(LOG_TAG, "Transmission is done");

        //We are done
        char eot = EOT;
        write(serial_fd, &eot, 1);
        state = XMODEMState::DONE;
      }
    }
  }

  return SL_STATUS_FAIL;
}

sl_status_t uic_gbl_interface_detect_bootloader(int serial_fd)
{
  sl_log_debug(LOG_TAG, "Checking for bootloader");
  // Empty the serial buffer first
  read_timeout(serial_fd, 100);

  //Trigger an update
  write(serial_fd, "3\n", 2);

  // The bootload er menu looks like this
  //
  // Gecko Bootloader v1.5.1
  // 1. upload gbl
  // 2. run
  // 3. ebl info
  // BL >

  std::string line = read_timeout(serial_fd, 100);
  if (line.find("Gecko Bootloader", 0) == std::string::npos) {
    sl_log_warning(LOG_TAG, "Bootloader is not detected, aborting");
    return SL_STATUS_NOT_FOUND;
  } else {
    sl_log_debug(LOG_TAG, "Bootloader detected: %s", line.c_str());
    return SL_STATUS_OK;
  }
}

sl_status_t uic_gbl_interface_transfer_image(
  const char *gbl_filename,
  int serial_fd,
  uic_gbl_interface_transfer_progress_t progress_callback)
{
  if (SL_STATUS_OK == uic_gbl_interface_detect_bootloader(serial_fd)) {
    //Start the XMODEM transfer
    write(serial_fd, "1\n", 2);
    xmodem_transfer(gbl_filename, serial_fd, progress_callback);

    std::string line = read_timeout(serial_fd, 100);
    sl_log_debug(LOG_TAG, "Bootloader says:\n %s", line.c_str());
  }

  sl_log_debug(LOG_TAG, "Running application");

  //Reboot into application
  write(serial_fd, "2\n", 2);
  read_timeout(serial_fd, 100);

  return SL_STATUS_OK;
}