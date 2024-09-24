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

#ifndef ZWAVE_FRAME_GENERATOR_HPP
#define ZWAVE_FRAME_GENERATOR_HPP

#ifdef __cplusplus

// Unify includes
#include "attribute_store.h"
#include "sl_status.h"

// Cpp includes
#include <vector>

/**
 * @class zwave_frame_generator
 * @brief Generate frames for Z-Wave commands based on attribute store values
 * 
 * Mainly used to generate Set or Get frame to send to Z-Wave devices
 * 
 * You can either set raw bytes, or let the function get the value from the attribute store for 
 * you. You are able to specify if you want a Desired or Reported value.
 * 
 * @code{.cpp}
 * // Only needed to be instantiated once
 * static zwave_frame_generator frame_generator(COMMAND_CLASS_SWITCH_BINARY);
 * 
 * // On a frame callback : 
 * static sl_status_t zwave_command_class_set(
 *    attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length) {
 *
 *  // We don't use frame_length here since we need to set it manually
 *  constexpr uint8_t expected_frame_size = 3;
 *  try {
 *    frame_generator.initialize_frame(SWITCH_BINARY_SET, frame, expected_frame_size);
 *    frame_generator.add_raw_byte(0x01);
 *    frame_generator.add_value(my_binary_node, REPORTED_ATTRIBUTE);
 * 
 *    // Will take the DESIRED_ATTRIBUTE value from my_node and shift it 2 bits to the left,
 *    // then add 0b1 shifted 7 bits to the left. 
 *    // Result value (if desired value is 0b11) : 0b10001100
 *    std::vector<zwave_frame_generator::shifted_value> values_mix = {
 *      {.left_shift       = 2,
 *        .node             = my_node,
 *        .node_value_state = DESIRED_ATTRIBUTE},
 *      {.left_shift = 7, .raw_value = 0b1},
 *    };
 *    frame_generator.add_shifted_values(values_mix);
 * 
 *    frame_generator.validate_frame(frame_length);
 * 
 *  } catch (const std::exception &e) {
 *   sl_log_error(LOG_TAG, "Error while generating frame : %s", e.what());
 *   return SL_STATUS_FAIL;
 *  }
 * 
 *  return SL_STATUS_OK;
 * }
 * 
 * @endcode
 */
class zwave_frame_generator
{
  public:
  /**
   * @brief Represent a value that needs to be shifted before being added to the frame.
   * 
   * You can either specify the node and value state to get the value from the attribute store
   * or provide a raw value to be shifted.
   * 
   * @note If you provide a node, it will read a uint8_t value from it.
   */
  struct shifted_value {
    /**
     * @brief The number of bits to shift the value (left)
     */
    uint8_t left_shift = 0;
    /**
     * @brief Node to get the value from (uint8_t). If ATTRIBUTE_STORE_INVALID_NODE, use raw_value
     */
    attribute_store_node_t node = ATTRIBUTE_STORE_INVALID_NODE;
    /**
     * @brief State of the value to get from the node. Only used if node is not ATTRIBUTE_STORE_INVALID_NODE
     */
    attribute_store_node_value_state_t node_value_state = REPORTED_ATTRIBUTE;
    /**
     * @brief Raw value to shift. Only used if node is ATTRIBUTE_STORE_INVALID_NODE
     */
    uint8_t raw_value = 0;
  };

  /**
   * @brief Constructor
   * 
   * @param zwave_command_class The Z-Wave command class to use in the header of all generated commands
   */
  explicit zwave_frame_generator(uint8_t zwave_command_class);
  ~zwave_frame_generator() = default;

  /**
   * @brief Initialize a new Z-Wave frame on the given data section.
   * 
   * @note This will reset the internal counter to 0 and update the data section provided with other functions.
   * 
   * After calling this function your frame will look like : 
   * 0: zwave_command_class (from constructor)
   * 1: zwave_command_id (from this function)
   * 
   * @param zwave_command_id The Z-Wave command ID to use in the header of the frame
   * @param raw_data The data section of the frame (must be allowed an valid from this address to data_size bytes after)
   * @param data_size The size of the data section (we use uint16_t to match Unify API)
   */
  void initialize_frame(uint8_t zwave_command_id,
                        uint8_t *raw_data,
                        uint16_t data_size);

  /**
   * @brief Add a raw byte to the Z-Wave frame
   * 
   * @param byte The byte to add to the frame
   */
  void add_raw_byte(uint8_t byte);

  /**
   * @brief Add the value contained in the given node to the Z-Wave frame
   * 
   * @throws std::runtime_error if the node is invalid or if the value can't be read
   * 
   * @note The size of the value is automatically determined by the attribute store.
   *       Numerical values will be stored in big-endian (MSB first LSB last).
   *       Other formats will keep their original order. 
   * 
   * @param node The node to get the value from
   * @param node_value_state The state of the value to get from the node
   * 
   */
  void add_value(attribute_store_node_t node,
                 attribute_store_node_value_state_t node_value_state
                 = REPORTED_ATTRIBUTE);
  /**
   * @brief Add a shifted value to the Z-Wave frame
   * 
   * You can either specify a raw value to be shifted, or directly pass the attribute
   * store node.
   * 
   * @see shifted_value
   * 
   * @param shifted_values The shifted value to add to the frame
   */
  void add_shifted_values(const std::vector<shifted_value> &shifted_values);
  /**
   * @brief Add a shifted value to the Z-Wave frame (single value version)
   * 
   * Convenience function to add a single shifted value to the frame.
   * 
   * @see shifted_value
   * 
   * @param shifted_values The shifted value to add to the frame
   */
  void add_shifted_values(const shifted_value &shifted_values);

  /**
   * @brief Validate the frame length and throw an error if it is not the expected length
   * 
   * @note We don't use bool here since the all the function throw an error if anything goes wrong
   * 
   * @param frame_length Will set the frame length if current frame length is equal to expected length 
   */
  void validate_frame(uint16_t *frame_length) const;

  /**
  * @brief Generate a Z-Wave frame with no arguments
  * 
  * This function is used to generate a Z-Wave frame with no arguments like a simple Get command.
  * Since it is used for convenience, this method doesn't throw an exception and return a status instead.
  * 
  * @param zwave_command_id The Z-Wave command ID to use in the header of the frame
  * @param raw_data The data section of the frame (must be able to write 2 byte to this address)
  * @param frame_length Frame length pointer (set to 2)
  * 
  * @return SL_STATUS_OK if the frame was generated successfully, SL_STATUS_FAIL otherwise
  */
  sl_status_t generate_no_args_frame(uint8_t zwave_command_id,
                                     uint8_t *raw_data,
                                     uint16_t *frame_length);

  private:
  /**
   * @brief Helper function to get the raw data from the attribute store
   * 
   * @note Number will be returned in little-endian (LSB first MSB last)
   * 
   * @param node The node to get the value from
   * @param node_value_state The state of the value to get from the node
   * 
   * @return The raw data from the attribute store
   */
  std::vector<uint8_t> helper_get_raw_data(
    attribute_store_node_t node,
    attribute_store_node_value_state_t node_value_state) const;

  // Current Z-Wave command class used in the header of all generated commands
  const uint8_t current_command_class;
  // Vector wrapper around raw frame data. See initialize_frame()
  uint8_t *current_zwave_frame;
  // Current Z-Wave frame size
  uint16_t current_zwave_frame_size = 0;
  // Current Z-Wave frame index (we use uint16_t to match Unify API)
  uint16_t current_zwave_frame_index = 0;
};

#endif  // __cplusplus
#endif  // ZWAVE_FRAME_GENERATOR_HPP