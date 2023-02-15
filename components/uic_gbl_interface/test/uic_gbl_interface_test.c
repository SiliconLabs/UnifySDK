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
#include "unity.h"
#include "sl_log.h"
#include <stdbool.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LOG_TAG "uic_gbl_interface_test"

//#define DEBUG_PRINT 1

/*
const char* gbl_menu[] =
"Gecko Bootloader v1.5.1\n"
"1. upload gbl\n"
"2. run\n"
"3. ebl info\n"
"BL >\n"
""
;
*/

typedef enum {
  INIT,
  WRITING_BOOTLOADER_NAME,
  WRITING_BOOTLOADER_NAME_DONE,
  WRITING_WRONG_BOOTLOADER_NAME,
  TRANSFER_IMAGE,
  TRANSFER_C,
  TRANSFER_IMAGE_DONE,
  TRANSFER_EXIT,
} test_state_t;

typedef enum {
  TEST_RIGHT_BOOTLOADER,
  TEST_WRONG_BOOTLOADER,
  TEST_RIGHT_BOOTLOADER_FAIL_3,
} bootloader_test_type_t;

typedef enum {
  TEST_TRANSFER_GBL,
  TEST_TRANSFER_GBL_NAK,
  TEST_TRANSFER_GBL_CAN,
  TEST_TRANSFER_GBL_C,
  TEST_TRANSFER_GBL_WRITE_LESS_DATA,
  TEST_TRANSFER_GBL_FAIL_IN_BL_RUN,
  TEST_TRANSFER_GBL_FAIL_IN_EOT,
  TEST_TRANSFER_GBL_SEND_WEIRD_CHAR_IN_WAIT_FOR_C,
} transfer_test_type_t;

test_state_t state                          = INIT;
bootloader_test_type_t bootloader_test_type = TEST_RIGHT_BOOTLOADER;
transfer_test_type_t transfer_test          = TEST_TRANSFER_GBL;

static int read_offset        = 0;
int fdp                       = 0xff;
int gtransferred              = 0;
int greceived                 = 0;
int gbl_remaining_to_transfer = 0;
int gcount                    = 0;
int gbl_size                  = 0;
static int nack_sent          = 0;
const char *gbl_file          = "zwave_ncp_serial_api_controller_BRD4201A.gbl";
const char *wrong_gbl_file
  = "wrong_zwave_ncp_serial_api_controller_BRD4201A.gbl";

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  FILE *fp = fopen(gbl_file, "r");
  if (fp == NULL) {
    sl_log_error(LOG_TAG, "File: %s could not be opened\n", gbl_file);
    TEST_ABORT();
  } else {
    fseek(fp, 0, SEEK_END);  // seek to end of file
    gbl_remaining_to_transfer = gbl_size
      = ftell(fp);           // get current file pointer
    fseek(fp, 0, SEEK_SET);  // seek back to beginning of file
    fclose(fp);
  }
}
void setUp()
{
  gbl_remaining_to_transfer = gbl_size;
  state                     = INIT;
}
// read is called for both serial device and GBL file. fd == 0xff tells that its
// serial device and the other one is GBL device
ssize_t read(int fd, void *buf, size_t count)
{
#if DEBUG_PRINT 
  if (fd == fdp) {
    sl_log_debug(LOG_TAG,
                 "Reading serial device state: %d, count: %d\n",
                 state,
                 count);
  } else {
    sl_log_debug(LOG_TAG, "Reading GBL state: %d, count: %d\n", state, count);
  }
#endif
  char *str            = (char *)buf;
  const char *bl       = "Gecko Bootloader v1.5.1";
  const char *wrong_bl = "Wrong Bootloader vWhoCares";

  if ((state == WRITING_BOOTLOADER_NAME)
      || (state == WRITING_WRONG_BOOTLOADER_NAME)) {
    TEST_ASSERT_EQUAL(count, 1);
    if (state == WRITING_BOOTLOADER_NAME) {
      str[0] = bl[read_offset];
    } else if (state == WRITING_WRONG_BOOTLOADER_NAME) {
      str[0] = wrong_bl[read_offset];
    }
    read_offset++;
    if (read_offset > strlen(bl)) {
      read_offset = 0;
      state       = WRITING_BOOTLOADER_NAME_DONE;
    }
    return 1;
  } else if ((state == WRITING_BOOTLOADER_NAME_DONE)
             || (state == TRANSFER_EXIT)) {
    TEST_FAIL();  // This should not happen. As poll() will always return 0 and read() should not be called
    return 0;
  } else if (state == TRANSFER_C) {
    if (transfer_test == TEST_TRANSFER_GBL_SEND_WEIRD_CHAR_IN_WAIT_FOR_C) {
      sl_log_warning(LOG_TAG, "sending weird char\n");
      str[0] = 'W';
      return count;
    }
    str[0] = 'C';
    TEST_ASSERT_EQUAL(count, 1);
    state = TRANSFER_IMAGE;
    return count;
  } else if (state == TRANSFER_IMAGE) {
    if (fd == fdp) {  //the serial port read
      str[0] = 0x06;
      TEST_ASSERT_EQUAL(count, 1);
    } else {  //the xmodem is now Reading gbl file
      sl_log_debug(LOG_TAG, "Transferring Image\n");
      TEST_ASSERT_EQUAL(count, 8191);
      if (gbl_remaining_to_transfer < count) {
        count = gbl_remaining_to_transfer;
      }
      gbl_remaining_to_transfer -= count;
      if (gbl_remaining_to_transfer <= 0) {
        sl_log_debug(LOG_TAG, "End of file");
      }
      return count;
    }
  } else if (state == TRANSFER_IMAGE_DONE) {
    if (fd != fdp) {  //the xmodem is now Reading gbl file
      return 0;
    } else {  //the serial port read
      if (transfer_test == TEST_TRANSFER_GBL_NAK) {
        if (nack_sent == 3) {
          state
            = TRANSFER_EXIT;  //If NAK is sent three times, exit and tell poll to say no data to read
          nack_sent = 0;
        }
        str[0] = 0x15;
        nack_sent++;
      } else if (
        transfer_test
        == TEST_TRANSFER_GBL_CAN) {  //If CAN is sent exit and tell poll to say no data to read
        str[0] = 0x18;
        state  = TRANSFER_EXIT;
      } else if (
        transfer_test
        == TEST_TRANSFER_GBL_C) {  //If C is sent exit and tell poll to say no data to read
        str[0] = 0x43;
        state  = TRANSFER_EXIT;
      } else {
        str[0] = 0x06;
        state  = TRANSFER_EXIT;
      }
      return 1;
    }
  }
  return 1;
}

// Write is only done on serial device
ssize_t write(int fd, const void *buf, size_t count)
{
  TEST_ASSERT_EQUAL(fd, 0xff);
  const uint8_t *str = (const uint8_t *)buf;
#if DEBUG_PRINT 
  sl_log_debug(LOG_TAG, "my write state: %d count:%d\n", state, count);
  if (count < 3) {
    for (int i = 0; i < count; i++) {
      sl_log_debug(LOG_TAG, "%c(0x%02x)", str[i], str[i]);
    }
  }
#endif
  switch (str[0]) {
    case 0x01:
      TEST_ASSERT_EQUAL(count, 133);
      if (transfer_test == TEST_TRANSFER_GBL_WRITE_LESS_DATA) {
        return (count - 1);
      }
      break;
    case 0x04:
      TEST_ASSERT_EQUAL(count, 1);
      if (transfer_test == TEST_TRANSFER_GBL_FAIL_IN_EOT) {
        return (count - 1);
      }
      sl_log_info(LOG_TAG, "Received EOT\n");
      state = TRANSFER_IMAGE_DONE;
      break;
    case '3':
      TEST_ASSERT_EQUAL_CHAR(str[1], '\n');  //check there is newline after 3
      TEST_ASSERT_EQUAL(count, 2);
      if (bootloader_test_type == TEST_RIGHT_BOOTLOADER) {
        sl_log_debug(LOG_TAG, "Writing Gecko Bootloader\n");
        state
          = WRITING_BOOTLOADER_NAME;  //Just change the state here. read() mock above will do the magic
      } else if (bootloader_test_type == TEST_RIGHT_BOOTLOADER_FAIL_3) {
        return 1;  //Return as less number of bytes written
      } else {
        sl_log_debug(LOG_TAG, "Writing wrong Gecko Bootloader\n");
        state
          = WRITING_WRONG_BOOTLOADER_NAME;  //Just change the state here. read() mock above will do the magic
      }
      break;
    case '1':
      TEST_ASSERT_EQUAL_CHAR(str[1], '\n');  //check there is newline after 1
      TEST_ASSERT_EQUAL(count, 2);
      state = TRANSFER_C;
      break;
    case '2':
      TEST_ASSERT_EQUAL_CHAR(str[1], '\n');  //check there is newline after 1
      TEST_ASSERT_EQUAL(count, 2);
      if (transfer_test == TEST_TRANSFER_GBL_FAIL_IN_BL_RUN) {
        return count - 1;
      }
      break;
    default:  //nothing else should be written on serial fd
      TEST_FAIL();
  }
  return count;
}

int poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
  if ((state == WRITING_BOOTLOADER_NAME)
      || (state == WRITING_WRONG_BOOTLOADER_NAME) || (state == TRANSFER_IMAGE)
      || (state == TRANSFER_IMAGE_DONE) || (state == TRANSFER_C)) {
    fds[0].revents = 1;  //data is availble to read
  } else if ((state == WRITING_BOOTLOADER_NAME_DONE) || (state == INIT)
             || (state == TRANSFER_EXIT)) {
    fds[0].revents = 0;  //No data to read
  }
  return 1;
}

/* Test State machine:
 * uic_gbl_interface_detect_bootloader calls write(3\n)
 *  - write() sets state to WRITING_BOOTLOADER_NAME 
 *  - poll() will return 1 everytime called in WRITING_BOOTLOADER_NAME
 *  - read() will return "Gecko Bootloader v1.5.1" string one chracter each time called
 *  - read() will set state to WRITING_BOOTLOADER_NAME_DONE on last character of "Gecko Bootloader v1.5.1"
 *  - poll() return 0 in state WRITING_BOOTLOADER_NAME_DONE and no more read() calls will be done by
 *      uic_gbl_interface_detect_bootloader() again
 *  - uic_gbl_interface_detect_bootloader() will return SL_STATUS_OK on receiving
 *    "Gecko Bootloader v1.5.1" 
 * */
void test_uic_gbl_interfaces_correct_bootloader_name()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  TEST_ASSERT_EQUAL(uic_gbl_interface_detect_bootloader(fdp), SL_STATUS_OK);
}

/* Test State machine:
 * uic_gbl_interface_detect_bootloader calls write(3\n)
 *  - write() returns 1 instead of 2 for write("3\n") call 
 *  - uic_gbl_interface_detect_bootloader() will return SL_STATUS_NOT_FOUND
 * */

void test_uic_gbl_interfaces_correct_bootloader_name_fail_in_writing_3()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER_FAIL_3;
  TEST_ASSERT_EQUAL(uic_gbl_interface_detect_bootloader(fdp),
                    SL_STATUS_NOT_FOUND);
}

/* Test State machine:
 * uic_gbl_interface_detect_bootloader calls write(3\n)
 *  - write() sets state to WRITING_WRONG_BOOTLOADER_NAME 
 *  - poll() will return 1 everytime called in WRITING_WRONG_BOOTLOADER_NAME
 *  - read() will return "Wrong Bootloader vWhoCares" string one chracter each time called
 *  - read() will set state to WRITING_BOOTLOADER_NAME_DONE on last character of "Wrong Bootloader vWhoCares"
 *  - poll() return 0 in state WRITING_BOOTLOADER_NAME_DONE and no more read() calls will be done by
 *      uic_gbl_interface_detect_bootloader() again
 *  - uic_gbl_interface_detect_bootloader() will return SL_STATUS_NOT_FOUND on receiving
 *      "Wrong Bootloader vWhoCares"
 * */

void test_uic_gbl_interfaces_wrong_bootloader_name()
{
  bootloader_test_type = TEST_WRONG_BOOTLOADER;
  TEST_ASSERT_EQUAL(uic_gbl_interface_detect_bootloader(fdp),
                    SL_STATUS_NOT_FOUND);
}

void progress_callback(int offset, int size)
{
  sl_log_info(LOG_TAG, "Progress: %d of %d\n", offset, size);
  TEST_ASSERT_GREATER_THAN(offset, size);
}

/* Test State machine:
 * Do same stuff as test_uic_gbl_interfaces_correct_bootloader_name
 * When uic_gbl_interface_transfer_image() writes "1\n" set state = TRANSFER_C
 * poll() will return 1 in state TRANSFER_C
 * read() on serial fd will return 'C' in state TRANSFER_C and move to state TRANSFER_IMAGE
 * poll() will return 1 in state TRANSFER_IMAGE
 * read() on serial fd will return ACK in state TRANSFER_IMAGE
 * read() on gbl will return number of asked bytes but 
 *    for last read() call on will read() return remaining bytes and 
 *    and set state to TRANSFER_IMAGE_DONE
 * write() on serial fd meanwhile will return number of bytes written
 * uic_gbl_interface_transfer_image() will write() EOT and then write() will
 *    set state to TRANSFER_IMAGE_DONE
 * <MARK> read() on serial fd then will return ACK in state TRANSFER_IMAGE_DONE and set state to TRANSFER_EXIT
 * poll() return 0 in state TRANSFER_EXIT and no more read() calls will be done by
 *   uic_gbl_interface_transfer_image() again
 * uic_gbl_interface_transfer_image() will return SL_STATUS_OK
 */
void test_uic_gbl_interface_transfer_image()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  TEST_ASSERT_EQUAL(
    uic_gbl_interface_transfer_image(gbl_file, fdp, progress_callback),
    SL_STATUS_OK);
}

/*
 * Test State machine:
 * Same as test_uic_gbl_interface_transfer_image BUT at <MARK>
 * read() on serial fd will return NACK three times
 * third read() on serial fd then will return NACK in state TRANSFER_IMAGE_DONE and set state to TRANSFER_EXIT
 * poll() return 0 in state TRANSFER_EXIT and no more read() calls will be done by
 *   uic_gbl_interface_transfer_image() again
 * uic_gbl_interface_transfer_image() will return SL_STATUS_FAIL
 *
 */
void test_uic_gbl_interface_transfer_image_nak()
{
  transfer_test        = TEST_TRANSFER_GBL_NAK;
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  TEST_ASSERT_EQUAL(
    uic_gbl_interface_transfer_image(gbl_file, fdp, progress_callback),
    SL_STATUS_FAIL);
}
/*
 * Test State machine:
 * Same as test_uic_gbl_interface_transfer_image BUT at <MARK>
 * read() on serial fd then will return CAN in state TRANSFER_IMAGE_DONE and set state to TRANSFER_EXIT
 * poll() return 0 in state TRANSFER_EXIT and no more read() calls will be done by
 *   uic_gbl_interface_transfer_image() again
 * uic_gbl_interface_transfer_image() will return SL_STATUS_FAIL
 *
 */
void test_uic_gbl_interface_transfer_image_can()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  transfer_test        = TEST_TRANSFER_GBL_CAN;
  TEST_ASSERT_EQUAL(
    uic_gbl_interface_transfer_image(gbl_file, fdp, progress_callback),
    SL_STATUS_FAIL);
}

/*
 * Test State machine:
 * Same as test_uic_gbl_interface_transfer_image BUT at <MARK>
 * read() on serial fd then will return C in state TRANSFER_IMAGE_DONE and set state to TRANSFER_EXIT
 * poll() return 0 in state TRANSFER_EXIT and no more read() calls will be done by
 *   uic_gbl_interface_transfer_image() again
 * uic_gbl_interface_transfer_image() will return SL_STATUS_FAIL
 *
 */
void test_uic_gbl_interface_transfer_image_c()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  transfer_test        = TEST_TRANSFER_GBL_C;
  TEST_ASSERT_EQUAL(
    uic_gbl_interface_transfer_image(gbl_file, fdp, progress_callback),
    SL_STATUS_FAIL);
}

/*
 * Test State machine:
 * Do same stuff as test_uic_gbl_interfaces_correct_bootloader_name
 * When uic_gbl_interface_transfer_image() writes "1\n" set state = TRANSFER_IMAGE
 * poll() will return 1 in state TRANSFER_C
 * read() on serial fd will return 'C' in state TRANSFER_C and move to state TRANSFER_IMAGE
 * poll() will return 1 in state TRANSFER_IMAGE
 * read() on serial fd will return ACK in state TRANSFER_IMAGE
 * read() on gbl will return number of asked bytes but 
 *    for last read() call on will read() return remaining bytes and 
 *    and set state to TRANSFER_IMAGE_DONE
 * write() on serial fd meanwhile will return one less in return for number of bytes written
 * uic_gbl_interface_transfer_image() will write() EOT and then write() will
 *    set state to TRANSFER_IMAGE_DONE
 * read() on serial fd then will return ACK in state TRANSFER_IMAGE_DONE and set state to TRANSFER_EXIT
 * poll() return 0 in state TRANSFER_EXIT and no more read() calls will be done by
 *   uic_gbl_interface_transfer_image() again
 * uic_gbl_interface_transfer_image() will return SL_STATUS_OK
 
 */
void test_uic_gbl_interface_transfer_image_less_block_data()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  transfer_test        = TEST_TRANSFER_GBL_WRITE_LESS_DATA;
  TEST_ASSERT_EQUAL(
    uic_gbl_interface_transfer_image(gbl_file, fdp, progress_callback),
    SL_STATUS_OK);
}

/* Just pass wrong file name and see if uic_gbl_interface_transfer_image()
 * returns SL_STATUS_FAIL
 */
void test_uic_gbl_interface_transfer_image_wrong_gbl_file()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  TEST_ASSERT_EQUAL(
    uic_gbl_interface_transfer_image(wrong_gbl_file, fdp, progress_callback),
    SL_STATUS_FAIL);
}

/*
 * Test State machine:
 * Do same stuff as test_uic_gbl_interfaces_correct_bootloader_name
 * When uic_gbl_interface_transfer_image() writes "1\n" set state = TRANSFER_C
 * poll() will return 1 in state TRANSFER_C
 * read() on serial fd will return 'C' in state TRANSFER_C and move to state TRANSFER_IMAGE
 * poll() will return 1 in state TRANSFER_IMAGE
 * read() on serial fd will return ACK in state TRANSFER_IMAGE
 * read() on gbl will return number of asked bytes but 
 *    for last read() call on will read() return remaining bytes and 
 *    and set state to TRANSFER_IMAGE_DONE
 * write() on serial fd meanwhile will return one less in return for number of bytes written
 * uic_gbl_interface_transfer_image() will write() EOT and then write() will
 *    set state to TRANSFER_IMAGE_DONE
 * read() on serial fd then will return ACK in state TRANSFER_IMAGE_DONE and set state to TRANSFER_EXIT
 * poll() return 0 in state TRANSFER_EXIT and no more read() calls will be done by
 *   uic_gbl_interface_transfer_image() again
 * uic_gbl_interface_transfer_image() will return SL_STATUS_OK
 
 */
void test_uic_gbl_interface_transfer_image_fail_in_bootloader_run()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  transfer_test        = TEST_TRANSFER_GBL_FAIL_IN_BL_RUN;
  TEST_ASSERT_EQUAL(
    uic_gbl_interface_transfer_image(gbl_file, fdp, progress_callback),
    SL_STATUS_FAIL);
}
/*
 * Test State machine:
 * Do same stuff as test_uic_gbl_interfaces_correct_bootloader_name
 * When uic_gbl_interface_transfer_image() writes "1\n" set state = TRANSFER_C
 * poll() will return 1 in state TRANSFER_C
 * read() on serial fd will return 'C' in state TRANSFER_C and move to state TRANSFER_IMAGE
 * poll() will return 1 in state TRANSFER_IMAGE
 * read() on serial fd will return ACK in state TRANSFER_IMAGE
 * read() on gbl will return number of asked bytes but 
 *    for last read() call on will read() return remaining bytes and 
 *    and set state to TRANSFER_IMAGE_DONE
 * write() on serial fd meanwhile will return one less in return for number of bytes written
 * uic_gbl_interface_transfer_image() will write() EOT and then write() will
 *    set state to TRANSFER_IMAGE_DONE
 * read() on serial fd then will return ACK in state TRANSFER_IMAGE_DONE and set state to TRANSFER_EXIT
 * poll() return 0 in state TRANSFER_EXIT and no more read() calls will be done by
 *   uic_gbl_interface_transfer_image() again
 * uic_gbl_interface_transfer_image() will return SL_STATUS_OK
 
 */
void test_uic_gbl_interface_transfer_image_fail_in_eot()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  transfer_test        = TEST_TRANSFER_GBL_FAIL_IN_EOT;
  TEST_ASSERT_EQUAL(
    uic_gbl_interface_transfer_image(gbl_file, fdp, progress_callback),
    SL_STATUS_FAIL);
}
/*
 * Test State machine:
 * Do same stuff as test_uic_gbl_interfaces_correct_bootloader_name
 * When uic_gbl_interface_transfer_image() writes "1\n" set state = TRANSFER_C
 * poll() will return 1 in state TRANSFER_C
 * read() on serial fd will return wrong character 'W' in state TRANSFER_C and move to state TRANSFER_IMAGE
 * uic_gbl_interface_transfer_image() should return SL_STATUS_FAIL here
 */
void test_uic_gbl_interface_transfer_image_send_weird_char_in_wait_for_c()
{
  bootloader_test_type = TEST_RIGHT_BOOTLOADER;
  transfer_test        = TEST_TRANSFER_GBL_SEND_WEIRD_CHAR_IN_WAIT_FOR_C;
  TEST_ASSERT_EQUAL(
    uic_gbl_interface_transfer_image(gbl_file, fdp, progress_callback),
    SL_STATUS_FAIL);
}

