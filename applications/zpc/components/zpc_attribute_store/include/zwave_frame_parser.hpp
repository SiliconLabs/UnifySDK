/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @defgroup zwave_frame_parser C++ Z-Wave Frame Handler Helper
 * @brief C++ definitions for handling Z-Wave Frames
 *
 * This group is used to handle Z-Wave frame and link their contents with the attribute store.
 *
 * @{
 */

#ifndef ZWAVE_FRAME_PARSER_HPP
#define ZWAVE_FRAME_PARSER_HPP

#ifdef __cplusplus

// Attribute store
#include "attribute_store.h"
#include "attribute.hpp"

// Unify
#include "sl_log.h"

// Generic C++ includes
#include <vector>
#include <string>
#include <bitset>
#include <map>

// Typedef
using zwave_report_bitmask_t = uint32_t;

/**
 * @class zwave_frame_parser
 * @brief Parses Z-Wave frames and extracts data from them.
 * 
 * Mainly used to parse report frame from Z-Wave devices.
 * 
 * This class is used to parse Z-Wave frames and store the data in the attribute store. 
 * It will store the value as reported in the attribute store.
 * 
 * You can also get the raw data from the frame without storage in the attribute store if needed.
 * 
 * This class has an internal index that will be incremented each time a value is read from the frame.
 * This index starts at 2. The first two bytes are the command class and the command ID and should be valid by the time you use this class.
 * 
 * Each time you read something, the index will be incremented by the number of bytes read.
 *
 * @note Each function (when possible) has an overload to store the value in the attribute store. We don't use default arguments to make sure the user is warned if their node is invalid.
 * 
 * @code{.cpp} 
  
  sl_status_t zwave_command_class_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
  {

    // Report Frame structure : 
    // 0x00 : COMMAND_CLASS
    // 0x01 : COMMAND_ID
    // 0x02 : REPORT_TYPE
    // 0x03 : REPORT_VALUE (MSB)
    // 0x04 : REPORT_VALUE (LSB)

    // Use try to catch read errors
    try {
      zwave_frame_parser parser(frame_data, frame_length);
      
      if (!parser.is_frame_size_valid(5)) {
        // No need to specify the details of size here it is handled by is_frame_size_valid
        sl_log_error("zwave_command_class_report", "Invalid frame size");
        return SL_STATUS_FAIL;
      }

      // Read the report type (0x02)
      uint8_t report_type = parser.read_byte();
      
      // Alternatively you can use the put the value directly in a node
      // attribute_store_node_t report_type_node = attribute_store_add_node(ATTRIBUTE_REPORT_TYPE, parent_node);
      // uint8_t report_type = parser.read_byte(report_type_node);

      // Read the report value (0x03 and 0x04)
      uint16_t report_value = parser.read_sequential<uint16_t>(2);

      // Alternatively you can use the put the value directly in a node
      // attribute_store_node_t report_value_node = attribute_store_add_node(ATTRIBUTE_REPORT_VALUE, parent_node);
      // uint16_t report_value = parser.read_sequential<uint16_t>(2, report_value_node);

      // Will trigger an exception since we are out of range
      // uint8_t out_of_range = parser.read_byte();

    } catch(const std::exception& e) {
      SL_LOG_ERROR("zwave_command_class_report", "Error while parsing the frame : %s", e.what());
      return SL_STATUS_FAIL;
    }

    return SL_STATUS_OK;
  } 
 * @endcode 
 */
class zwave_frame_parser
{
  public:
  using zwave_parser_bitmask_result = std::map<uint8_t, uint8_t>;

  /**
   * @brief Contains the bitmask to apply to a byte read from the frame [and the destination node in the attribute store]
   */
  struct bitmask_data {
    /**
     * @brief The bitmask to apply to the byte read from the frame
     */
    uint8_t bitmask = 0b00000000;
    /**
     * @brief The destination node in the attribute store where to store the value.
     * Can be ATTRIBUTE_STORE_INVALID_NODE if you don't want to store the value in the attribute store.
     */
    attribute_store_node_t destination_node = ATTRIBUTE_STORE_INVALID_NODE;
  };

  /**
   * @brief Constructor
   * 
   * @param data The Z-Wave frame to parse. Doesn't take ownership of the data.
   * @param length The length of the frame. We use uint16_t to match C API
   */
  explicit zwave_frame_parser(const uint8_t *data, uint16_t length);
  ~zwave_frame_parser() = default;

  /**
   * @brief Check if the frame size equal to the expected size
   * 
   * Good practice is too use this function right after the constructor 
   * to make sure the frame is valid.
   * 
   * @param min_size Minimal size of the frame. If this is the only parameter, the frame size should be equal to this value.
   * @param max_size Maximal size of the frame. If this parameter is set, the frame size should be between min_size and max_size. Default to 0 and will ignore this parameter.
   * 
   * @return true if the frame size is equal to the expected size
   * @return false if the frame size is not equal to the expected size
   */
  bool is_frame_size_valid(uint8_t min_size, uint8_t max_size = 0) const;

  /**
   * @brief Read the next byte from the frame
   * 
   * @note Calling this function will read the current value in the frame (starting index = 2) and increment it.
   * 
   * @exception std::out_of_range if the current index is out of range
   * 
   * @return The byte read from the frame
   */
  uint8_t read_byte();

  /**
   * @brief Read the next byte from the frame and store it in the attribute store
   * 
   * @note Calling this function will read the current value in the frame (starting index = 2) and increment it.
   * 
   * @param node The node in the attribute store where to store the value.
   * 
   * @exception std::out_of_range if the current index is out of range
   * @exception std::invalid_argument if something goes wrong when storing the value in the attribute store
   * 
   * @return The byte read from the frame
   */
  uint8_t read_byte(attribute_store_node_t node);

  /**
 * @brief Read a byte from the frame and applies given bitmasks [and store it in the attribute store]
 * 
 * @param bitmask_data The bitmask to apply to the byte read
 * 
 * @warning This function will not differentiate between an invalid node and no node provided. No warning will be issued if the node is invalid.
 * 
 * @code{.cpp} 
  // Example of usage
  const uint8_t tested_frame[] = {0x01, 0x02, 0b10100111};
  uint8_t expected_frame_size = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);
  
  // Create node
  attribute_store_node_t size_value_node = attribute_store_add_node(ATTRIBUTE_SIZE, endpoint_node);

  // Read first value
  // Store the first value in size_value_node
  constexpr uint8_t BITMASK_SIZE = 0b11000000;
  constexpr uint8_t BITMASK_PRECISION = 0b00111000;
  constexpr uint8_t BITMASK_SCALE = 0b00000011;

  std::vector<zwave_frame_parser::bitmask_data> data = {
    {.bitmask = BITMASK_SIZE, .destination_node = size_value_node},
    {.bitmask = BITMASK_PRECISION},
    {.bitmask = BITMASK_SCALE}
  };
  // The value of values will be : {BITMASK_SIZE => 0b10, BITMASK_PRECISION => 0b100, BITMASK_SCALE => 0b11}
  zwave_parser_bitmask_result bitmask_values = parser.read_byte_with_bitmask(data);    
  uint8_t precision_value = bitmask_values[BITMASK_PRECISION]; // 0b100
  uint8_t scale_value = bitmask_values[BITMASK_SCALE]; // 0b11
  uint8_t size_value = bitmask_values[BITMASK_SIZE]; // 0b10
 * @endcode 
 * 
 * @exception std::out_of_range if the current index is out of range
 * @exception std::invalid_argument if something goes wrong when storing the value in the attribute store
 * 
 * @return The byte read from the frame
 */
  zwave_parser_bitmask_result
    read_byte_with_bitmask(const std::vector<bitmask_data> &bitmask_data);

  /**
   * @brief Read a byte from the frame and applies given bitmask [and store it in the attribute store]
   * 
   * @note This is a convenience function to avoid creating a vector of bitmask_data
   * 
   * @param bitmask_data The bitmask to apply to the bytes read
   * 
   * @exception std::out_of_range if the current index is out of range
   * @exception std::invalid_argument if something goes wrong when storing the value in the attribute store
   * 
   * @return The bytes read from the frame 
 */
  zwave_parser_bitmask_result
    read_byte_with_bitmask(const bitmask_data &bitmask_data);

  /**
   * @brief Read a bitmask (adaptive size) from the frame 
   * 
   * This helper function will read the current value in the frame to determine the bitmask length, then read the given
   * bytes and convert it to zwave_report_bitmask_t.
   * 
   * Excepted frame structure :
   *  current_index     : Bitmask length (N=1-4)
   *  current_index + 1 : Bitmask value 1
   *  ...
   *  current_index + N : Bitmask value N
   * 
   * @note Calling this function will read the current value in the frame (starting index = 2) and increment it.
   * 
   * @exception std::out_of_range if the current index is out of range
   * @exception std::invalid_argument if bitmask length is greater than 4
   * 
   * @return The bitmask read from the frame
   */
  zwave_report_bitmask_t read_bitmask();

  /**
   * @brief Read a bitmask (adaptive size) from the frame and store it in the attribute store
   * 
   * This helper function will read the current value in the frame to determine the bitmask length, then read the given
   * bytes and convert it to zwave_report_bitmask_t.
   * 
   * Excepted frame structure :
   *  current_index     : Bitmask length (N=1-4)
   *  current_index + 1 : Bitmask value 1
   *  ...
   *  current_index + N : Bitmask value N
   * 
   * @note Calling this function will read the current value in the frame (starting index = 2) and increment it (to 1 + bitmask size)
   * 
   * @param node The node in the attribute store where to store the value.
   * 
   * @exception std::out_of_range if the current index is out of range
   * @exception std::invalid_argument if something goes wrong when storing the value in the attribute store
   * @exception std::invalid_argument if bitmask length is greater than 4
   * 
   * @return The bitmask read from the frame
   */
  zwave_report_bitmask_t read_bitmask(attribute_store_node_t node);

  /**
   * @brief Convenience function to read a string (ASCII) from the frame
   * 
   * This helper function will read the first byte to determine the string length, then parse the 
   * string from the frame.
   * 
   * Excepted frame structure :
   *  current_index     : String length
   *  current_index + 1 : Char 1
   *  ...
   *  current_index + N : Char N 
   * 
   * @note Calling this function will read the current value in the frame (starting index = 2) and increment it (to 1 + string size).
   *
   * @exception std::out_of_range if the current index is out of range
   * 
   * @return The string read from the frame
   */
  std::string read_string();
    /**
   * @brief Convenience function to read a string (ASCII) from the frame and store in in the attribute store
   * 
   * This helper function will read the first byte to determine the string length, then parse the 
   * string from the frame.
   * 
   * Excepted frame structure :
   *  current_index     : String length
   *  current_index + 1 : Char 1
   *  ...
   *  current_index + N : Char N 
   * 
   * @note Calling this function will read the current value in the frame (starting index = 2) and increment it (to 1 + string size).
   *
   *  @param node The node in the attribute store where to store the value.
   *
   * @exception std::out_of_range if the current index is out of range
   * @exception std::invalid_argument if something goes wrong when storing the value in the attribute store
   * @return The string read from the frame
   */
  std::string read_string(attribute_store_node_t node);


  /**
   * @brief Read a sequence of bytes from the frame
   * 
   * @tparam T The type of the value to read. Supported types are int (and unsigned) of any size, std::string, std::vector<uint8_t>
   * 
   * @note Calling this function with a numeric type will assume that the MSB is first and LSB is last.
   * @note Calling this function will read the current value in the frame (starting index = 2) and increment it by the number of read bytes.
   * 
   * @param bytes_to_read The number of bytes to read
   * 
   * @exception std::out_of_range if the current index is out of range
   * 
   * @return The value read from the frame
   */
  template<typename T> T read_sequential(uint8_t bytes_to_read)
  {
    static_assert(std::is_integral<T>::value
                    || std::is_same<T, std::string>::value
                    || std::is_same<T, std::vector<uint8_t>>::value,
                  "Unsupported type");

    T value_from_frame;
    if constexpr (std::is_integral<T>::value) {
      value_from_frame = 0;

      for (uint8_t i = 0; i < bytes_to_read; i++) {
        T current_byte = zwave_report_frame.at(current_index);
        // Z-Wave always have MSB first and LSB last
        // If we have read 2 bytes : 0x34,0x56 the result should be 0x3456
        // First iteration  : 0x34 << 8 = 0x3400
        // Second iteration : 0x56 << 0 = 0x3456
        uint8_t offset = (bytes_to_read - 1 - i) * 8;
        value_from_frame |= current_byte << offset;

        current_index++;
      }

      // Convert value to signed if needed
      // This is used in the case of a signed value that is smaller than the T type
      // e.g. int8_t stored in a int32_t.
      if constexpr (std::is_signed<T>::value) {
        switch (bytes_to_read) {
          case 1:
            value_from_frame = static_cast<int8_t>(value_from_frame);
            break;
          case 2:
            value_from_frame = static_cast<int16_t>(value_from_frame);
            break;
          case 4:
            value_from_frame = static_cast<int32_t>(value_from_frame);
            break;
          default:
            throw std::runtime_error("Unsupported size for signed value, only "
                                     "1, 2 or 4 are supported");
        }
      }
    } else if constexpr (std::is_same<T, std::string>::value
                         || std::is_same<T, std::vector<uint8_t>>::value) {
      value_from_frame.resize(bytes_to_read);
      for (uint8_t i = 0; i < bytes_to_read; i++) {
        value_from_frame[i] = read_byte();
      }
    }
    return value_from_frame;
  }

  /**
   * @brief Read a sequence of bytes from the frame and store it in the attribute store
   * 
   * @tparam T The type of the value to read. Supported types are int (and unsigned) of any size, std::string, std::vector<uint8_t>
   * 
   * @note Calling this function with a numeric type will assume that the MSB is first and LSB is last.
   * @note Calling this function will read the current value in the frame (starting index = 2) and increment it by the number of read bytes.
   * 
   * @param bytes_to_read The number of bytes to read
   * @param node The node in the attribute store where to store the value.
   * 
   * @exception std::out_of_range if the current index is out of range
   * @exception std::invalid_argument if something goes wrong when storing the value in the attribute store
   * 
   * @return The value read from the frame
   */
  template<typename T>
  T read_sequential(uint8_t bytes_to_read, attribute_store_node_t node)
  {
    T value = read_sequential<T>(bytes_to_read);
    helper_store_value(node, value);
    return value;
  }

  private:
  /**
   * @brief Helper function to store a value in the attribute store
   * 
   * @tparam T The type of the value to store
   * 
   * Will check if node is invalid, and do nothing if it is.
   * 
   * @param node The node in the attribute store where to store the value
   * @param value The value to store
   */
  template<typename T>
  void helper_store_value(attribute_store_node_t node, T value)
  {
    attribute_store::attribute current_attribute(node);

    if (current_attribute.is_valid()) {
      current_attribute.set_reported<T>(value);
    } else {
      sl_log_warning(
        "zwave_frame_parser",
        "Node does not exist in the attribute store. Not updating value");
    }
  }

  /**
   * @brief Get trailing zeroes of a bitset
   * 
   * @note Can use C++20 bit manipulation instead, but for now we are in C++17. (https://en.cppreference.com/w/cpp/numeric/countr_zero)
   * 
   * @param bitset The bitset to get the trailing zeroes from
   * 
   * @code{.cpp}
   *  std::bitset<8> bitset = 0b00001100;
   *  uint8_t trailing_zeroes = get_trailing_zeroes(bitset); // 2
   * @endcode
   * 
   * @return The number of trailing zeroes
   */
  uint8_t get_trailing_zeroes(const std::bitset<8> &bitset) const;

  // Contains the frame data
  std::vector<uint8_t> zwave_report_frame;
  // The first two bytes contains the command class and the command ID
  // Those informations are supposed to be valid when we parse the frame.
  uint8_t current_index = 2;
};

#endif  // #ifdef __cplusplus
#endif  // #ifndef ZWAVE_FRAME_PARSER_HPP