/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file zigpc_common_zigbee.h
 * @defgroup zigpc_common_zigbee Zigbee Types
 * @ingroup zigpc_common
 * @brief This header contains definitions that represent the Zigbee data types
 * used in the Geko SDK,
 *
 * @{
 */

#ifndef ZIGPC_COMMON_ZIGBEE_H
#define ZIGPC_COMMON_ZIGBEE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Size of EUI64 (an IEEE address) in bytes (8)
 */
#define ZIGBEE_EUI64_SIZE 8

/**
 * @brief Size of EUI64 string buffer
 *
 */
#define ZIGBEE_EUI64_HEX_STR_LENGTH ((ZIGBEE_EUI64_SIZE * 2) + 1)

/**
 * @brief Size of an extended PAN identifier in bytes (8).
 */
#define ZIGBEE_EXTENDED_PAN_ID_SIZE 8

/**
 * Zigbee Install Code length
 *  - 6, 8, 12, or 16 bytes plus two-byte CRC
 */
#define ZIGBEE_INSTALL_CODE_LENGTH (16 + 2)

/**
 * Zigbee Max Endpoint Number
 * the maximum number of endpoints supported in the Zigbee protocol
**/
#define ZIGBEE_MAX_ENDPOINT_NUMBER 256

/**
 * @brief The default identifier for non-application endpoint identifier
 * also known as the ZDO endpoint.
 *
 */
#define ZIGBEE_INVALID_APP_ENDPOINT_ID 0

/**
 * @brief Default string length for strings used in ZCL
**/
#define ZCL_DEFAULT_STR_LENGTH 256

/**
 * @brief Default string length for strings used in ZCL
**/
#define ZCL_DEFAULT_ARR_LENGTH 16

/**
 * ZCL Cluster ID Standard Ranges
 * Definitions representing the standard range of ZCL
 * Cluster ID's
**/
#define ZCL_MIN_CLUSTER_ID 0x0000
#define ZCL_MAX_CLUSTER_ID 0x7fff
#define ZCL_MIN_MFCTR_ID   0xfc00
#define ZCL_MAX_MFCTR_ID   0xffff

/**
 * ZCL Command ID Standard Range
 * Definitions representing the standard range of ZCL
 * Command ID's
**/
#define ZCL_MIN_COMMAND_ID 0x00
#define ZCL_MAX_COMMAND_ID 0x7f

/**
 * ZCL Attribute ID Standard Ranges
 * Definitions representing the standard range of ZCL
 * Attribute ID's
**/
#define ZCL_MIN_STD_ATTR_ID  0x0000
#define ZCL_MAX_STD_ATTR_ID  0x4fff
#define ZCL_MIN_GLOB_ATTR_ID 0xf000
#define ZCL_MAX_GLOB_ATTR_ID 0xfffe

/**
 * ZCL Invalid Cluster ID
 * A number outside the recognized range of cluster ID's
 * The expected range of ZCL cluster ID's is 0x0000 - 0x7fff
 * (0xfc00 - 0xffff is manufacturer reserved)
**/
#define ZCL_INVALID_CLUSTER_ID (ZCL_MAX_CLUSTER_ID + 1)

/**
 * ZCL Invalid Command ID
 * A number outside the recognized range of command ID's
 * The expected range of ZCL command ID's is 0x00 - 0x7f
**/

/**
 * ZCL Invalid Attribute ID
 * A number outside the recognized range of attribute ID's
**/
#define ZCL_INVALID_ATTR_ID    (ZCL_MAX_STD_ATTR_ID + 1)
#define ZCL_INVALID_COMMAND_ID 0x80

#define ZCL_MIN_CLUSTER_ID 0x0000
#define ZCL_MAX_CLUSTER_ID 0x7fff
#define ZCL_MIN_MFCTR_ID   0xfc00
#define ZCL_MAX_MFCTR_ID   0xffff
#define ZCL_MIN_COMMAND_ID 0x00
#define ZCL_MAX_COMMAND_ID 0x7f

/**
 * Maximum size of ZCL frame supported by Z3gateway API.
 *
 */
#define ZCL_FRAME_BUFFER_SIZE_MAX 255

/**
 * @brief ZCL frame will be set as being a ZCL global command frame type.
 *
 */
#define ZCL_FRAME_CONTROL_FRAME_TYPE_GLOBAL_COMMAND ((uint8_t)(0x0 << 0))

/**
 * @brief ZCL frame will be set as being a cluster frame type.
 *
 */
#define ZCL_FRAME_CONTROL_FRAME_TYPE_CLUSTER ((uint8_t)(0x1 << 0))

/**
 * @brief ZCL frame will be sent as a client to server message.
 *
 */
#define ZCL_FRAME_CONTROL_DIRECTION_CLIENT_TO_SERVER ((uint8_t)(0x0 << 3))

#define ZCL_GLOBAL_COMMAND_ID_READ_ATTRIBUTES  (0x00)
#define ZCL_GLOBAL_COMMAND_ID_WRITE_ATTRIBUTES (0x02)

#define ZCL_ATTRIBUTE_RESPONSE_RECORDS_MAX 20

/**
 * @brief Zigbee Device EUI64 ID
 *
 * NOTE: This is stored in big-endian format
 *
 */
typedef uint8_t zigbee_eui64_t[ZIGBEE_EUI64_SIZE];

/**
 * @brief Zigbee Device Identifier (stored as integer)
 *
 */
typedef uint64_t zigbee_eui64_uint_t;

/**
 * @brief Zigbee 3.0 Install Code
 *
 * NOTE: This is stored in big-endian format
 */
typedef uint8_t zigbee_install_code_t[ZIGBEE_INSTALL_CODE_LENGTH];

/**
 * @brief Zigbee Network PAN ID
 *
 */
typedef uint16_t zigbee_panid_t;

/**
 * @brief Zigbee Network Extended PAN ID
 *
 */
typedef uint8_t zigbee_ext_panid_t[ZIGBEE_EXTENDED_PAN_ID_SIZE];

/**
 * @brief Zigbee Network Radio Channel
 *
 */
typedef uint8_t zigbee_radio_channel_t;

/**
 * @brief Zigbee Group Id
 * The internal representation of a group_id
 */
typedef uint16_t zigbee_group_id_t;

/**
 *@brief ZIGBEE_NULL_GROUP_ID
 * The NULL group id, usually used for unicast zigbee operations
 */
static const zigbee_group_id_t ZIGBEE_NULL_GROUP_ID = 0;

/*
 * @brief Zigbee Network Multicast identifier
 *
 */
typedef uint16_t zigbee_multicast_id_t;

/**
 * zigbee_endpoint_id_t
 * @brief A representation of the endpoint id used on the Zigbee protocol
**/
typedef uint8_t zigbee_endpoint_id_t;

/**
 * zcl_cluster_id_t
 * @brief A representation of the endpoint cluster id used in ZCL
**/
typedef uint16_t zcl_cluster_id_t;

/**
 * @brief A representation of the endpoint cluster attribute ID used in ZCL
 *
 */
typedef uint16_t zcl_attribute_id_t;

static const bool ATTR_DIRECTION_REPORTED = true;
static const bool ATTR_DIRECTION_DESIRED  = !ATTR_DIRECTION_REPORTED;

typedef uint8_t zcl_command_id_t;

typedef enum {
  ZCL_CLUSTER_GENERATED_COMMAND = 1, /**< (Out) Command generated by cluster */
  ZCL_CLUSTER_RECEIVED_COMMAND  = 2, /**< (In) Command parsed by cluster */
} zcl_command_type_t;

typedef uint8_t zcl_command_field_t;

enum zcl_status_type {
  ZCL_OP_STATUS_SUCCESS = 0,
};

enum zcl_frame_data_type {
  ZCL_DATA_TYPE_UNKNOWN = 0,
  ZCL_DATA_TYPE_BOOL    = 0x10,
  ZCL_DATA_TYPE_MAP8    = 0x18,
  ZCL_DATA_TYPE_UINT8   = 0x20,
  ZCL_DATA_TYPE_UINT16  = 0x21,
  ZCL_DATA_TYPE_UINT32  = 0x23,
  ZCL_DATA_TYPE_INT8    = 0x28,
  ZCL_DATA_TYPE_INT16   = 0x29,
  ZCL_DATA_TYPE_INT32   = 0x2A,
  ZCL_DATA_TYPE_ENUM8   = 0x30,
  ZCL_DATA_TYPE_ENUM16  = 0x31,
  ZCL_DATA_TYPE_MAX_VAL,
};

typedef struct {
  size_t size;
  size_t offset_sequence_id;
  uint8_t buffer[ZCL_FRAME_BUFFER_SIZE_MAX];
} zcl_frame_t;

/**
 * zcl_command_t
**/
typedef struct zcl_command {
  uint16_t cluster_id;
  zcl_command_id_t command_id;
  size_t field_count;
} zcl_command_t;

/**
 * null_command
 * @brief a static data structure representing an NULL or INVALID
 * value for ZCL commands
**/
static const zcl_command_t null_command = {
  .cluster_id = ZCL_INVALID_CLUSTER_ID, .command_id = ZCL_INVALID_COMMAND_ID};

/**
 * e_zcl_cluster_role
 * @brief An enum representing the role of the cluster on the network
**/
typedef enum e_zcl_cluster_side {
  ZCL_CLUSTER_SERVER_SIDE = 1,
  ZCL_CLUSTER_CLIENT_SIDE = 2
} zcl_cluster_side_t;

enum e_zcl_attribute_type {
  ZCL_ATTR_TYPE_INVALID = -1,
  ZCL_ATTR_TYPE_BOOL    = 0x10,
  ZCL_ATTR_TYPE_MAP8    = 0x18,
  ZCL_ATTR_TYPE_UINT8   = 0x20,
  ZCL_ATTR_TYPE_UINT16  = 0x21,
  ZCL_ATTR_TYPE_INT16   = 0x29,
  ZCL_ATTR_TYPE_ENUM8   = 0x30,
  ZCL_ATTR_TYPE_FLOAT   = 0x38
};

/**
 * zcl_attribute_t
**/
typedef struct zcl_attribute {
  zcl_cluster_id_t cluster_id;
  zcl_attribute_id_t attribute_id;
  enum e_zcl_attribute_type type;
  bool is_reportable; /**< Flag to indicate if attribute is reportable */
  char attribute_name[ZCL_DEFAULT_STR_LENGTH];
  char desired_payload[ZCL_DEFAULT_STR_LENGTH];
  char reported_payload[ZCL_DEFAULT_STR_LENGTH];
} zcl_attribute_t;

/**
 * Zigbee Cluster Type
 * @ A model for a ZCL Cluster Type
**/
typedef struct {
  zcl_cluster_id_t cluster_id;
  enum e_zcl_cluster_side role;
} zcl_cluster_type_t;

/**
 * Zigbee Endpoint
 * @brief a model for a zigbee ZCL endpoint
**/
typedef struct {
  zigbee_endpoint_id_t endpoint_id;
  zcl_cluster_type_t cluster_list[ZCL_DEFAULT_ARR_LENGTH];
  unsigned int cluster_count;
  zcl_cluster_type_t client_cluster_list[ZCL_DEFAULT_ARR_LENGTH];
  unsigned int client_cluster_count;
} zigbee_endpoint_t;

typedef struct zcl_profile {
  char cluster_name[ZCL_DEFAULT_STR_LENGTH];
  zcl_cluster_id_t cluster_id;

  const zcl_attribute_t *attribute_list;
  unsigned int num_attributes;

} zcl_profile_t;
/**
 * @brief The possible states that a Zigbee node can be in.
 *
 */
typedef enum zigbee_node_network_status {
  ZIGBEE_NODE_STATUS_INVALID = 0,

  /**
   * @brief This state corresponds to when the node has completed joining the
   * protocol controller network.
   *
   */
  ZIGBEE_NODE_STATUS_NODEID_ASSIGNED,

  /**
   * @brief This state corresponds to when the node has completed interviewing
   * and ready to be serviced by the protocol controller.
   *
   */
  ZIGBEE_NODE_STATUS_INCLUDED,

  /**
   * @brief This state corresponds to when the node has timed out from the
   * interview process.
   *
   */
  ZIGBEE_NODE_STATUS_INTERVIEW_FAIL,

  /**
   * @brief This state corresponds to when the node is in the process of being
   * interviewed by the protocol controller.
   *
   */
  ZIGBEE_NODE_STATUS_INTERVIEWING,

  /**
   * @brief This state corresponds to when the node has not been reachable by
   * the protocol controller for a long time.
   *
   */
  ZIGBEE_NODE_STATUS_OFFLINE,

  /**
   * @brief This state corresponds to when the node is not accepting any
   * commands from the protocol controller.
   *
   */
  ZIGBEE_NODE_STATUS_UNAVAILABLE,

  ZIGBEE_NODE_STATUS_MAX_VAL,
} zigbee_node_network_status_t;

/**
 * Zigbee Node
 * @brief A model for a Zigbee End Node
 *
**/
typedef struct {
  zigbee_eui64_t eui64;
  zigbee_endpoint_t endpoint_list[ZCL_DEFAULT_ARR_LENGTH];
  unsigned int endpoint_count;
  bool is_active;
  bool is_sleepy;
  enum zigbee_node_network_status network_status;
} zigbee_node_t;

/**
 * @brief Convert the array EUI64 representation into 64-bit uint
 * representation.
 *
 * @param eui64             EUI64 array to convert
 * @return zigbee_eui64_uint_t 0xFFFFFFFFFFFFFFFF if the conversion is
 * unsuccessful, else the int representation.
 */
zigbee_eui64_uint_t zigbee_eui64_to_uint(const zigbee_eui64_t eui64);

/**
 * @brief Convert the 64-bit uint EUI64 representation into array
 * representation.
 *
 * @param src           EUI64 uint representation.
 * @param dest          EUI64 array to populate.
 * @return sl_status_t  SL_STATUS_OK if successful, SL_STATUS_NULL_POINTER if
 * invalid destination pointer is given.
 */
sl_status_t zigbee_uint_to_eui64(zigbee_eui64_uint_t src, zigbee_eui64_t dest);

/**
 * @brief Copy EUI64 and switch endianness (BE<->LE)
 * If the src EUI64 is LE, the populated dst EUI64 will be BE
 * If the src EUI64 is BE, the populated dst EUI64 will be LE
 *
 * @param dst destination EUI64 with switched endianness
 * @param src source EUI64 to copy from
 */
void zigbee_eui64_copy_switch_endian(zigbee_eui64_t dst,
                                     const zigbee_eui64_t src);

/**
 * @brief Copy a given buffer of data to another destination, inverting
 * the endianness
 *
 * @param source    - the source data
 * @param dest      - the destination to copy
 * @param size      - the size of the destination buffer
**/
void zigpc_common_switch_endian(uint8_t *source, uint8_t *dest, size_t size);

/**
 * @brief Convert EUI64 (in BE) into string format based on the hex representation.
 *
 * For example: EUI64 of {0x0, 0x1, 0x2, 0x3, 0x4, 0x0, 0x0, 0x0} will be
 *              transformed into "0001020304000000".
 *
 *
 * @param eui64             source EUI64
 * @param str_buf           destination buffer
 * @param str_length        destination buffer length, should be equal to
 *                          ZIGBEE_EUI64_HEX_STR_LENGTH
 * @return sl_status_t      SL_STATUS_OK or SL_STATUS_WOULD_OVERFLOW if
 *                          insufficient space is in str_buf
 */
sl_status_t zigbee_eui64_to_str(const zigbee_eui64_t eui64,
                                char *str_buf,
                                const size_t str_length);

/**
 * @brief Extract the Zigbee EUI64 from a UNID string. Only the last 8 bytes of the
 * number are considered.
 *
 * For example:
 * "11111110123456789abcdef" string is converted to
 * {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef} =
 * {1, 35, 69, 103, 137, 171, 205, 239}
 *
 * @param str_buf           source string
 * @param str_length        source string length
 * @param eui64             destination EUI64 to populate
 * @return sl_status_t      SL_STATUS_OK or SL_STATUS_FAIL if string contains
 * characters that cannot be translated to an EUI64 identifier
 */
sl_status_t str_to_zigbee_eui64(const char *str_buf,
                                const size_t str_length,
                                zigbee_eui64_t eui64);

/**
 * @brief Convert the Zigbee EUI64 to a UNID string with the UNID prefix
 * configured.
 *
 * @param eui64             source EUI64
 * @param dest_unid         Destination character array
 * @param dest_unid_length  Destination array capacity
 * @return sl_status_t      SL_STATUS_OK on successful conversion,
 * SL_STATUS_NULL_POINTER on invalid inputs, or SL_STATUS_WOULD_OVERFLOW if
 * destination character array is too small to fit the UNID string.
 */
sl_status_t zigpc_common_eui64_to_unid(const zigbee_eui64_t eui64,
                                       char *dest_unid,
                                       const size_t dest_unid_length);

/**
 * is_valid_zcl_command
 * @brief check if a given zcl command is valid or not
 *
 * Checks if a a given ZCL command has a valid cluster ID and command
 * ID.
 *
 * @param command - the ZCL command to check
 *
 * @return true if the command is valid, otherwise false
**/
bool is_valid_zcl_command(const zcl_command_t command);

/**
 * copy_command
 * @brief Performs a deep copy of a ZCL Command. Does not require a "free"-ing
 * of memory, as the zcl_command_t data structure is simple
 *
 * @param source - the source zcl_command_t that we want to copy. expected to
 *                 have valid data
 * @param dest - the destination zcl_command where we are copying to
 *
 * @return the status of the operation. fails if we are trying to copy an invalid
 * ZCL command or if the destination is NULL
**/
sl_status_t copy_command(const zcl_command_t source, zcl_command_t *dest);

/**
 * is_valid_zcl_attribute
 * @brief checks if a given ZCL attribute is valid or not
 *
 * @param attribute - the attribute to check
 *
 * @return true if the attribute is valid, otherwise false
**/
bool is_valid_zcl_attribute(const zcl_attribute_t attribute);

/**
 * copy_attribute
 * @brief Performs a deep copy of a ZCL Attribute. Does not require a "free"-ing
 * of memory, as the zcl_attribute_t data structure is simple
 *
 * @param source - the source zcl_attribute_t that we want to copy. expected to
 *                 have valid data
 * @param dest - the destination zcl_attribute where we are copying to
 * @param list_size - capacity of the destination zcl_attribute list
 *
 * @return the status of the operation. fails if we are trying to copy an invalid
 * ZCL attribute or if the destination is NULL
**/

sl_status_t copy_attribute_list(const zcl_attribute_t *const source,
                                zcl_attribute_t *dest,
                                unsigned int list_size);

/**
 * is_valid_zcl_cluster
 * @brief checks if a given ZCL cluster is valid or not
 *
 * @param cluster - the cluster to check
 *
 * @return true if the cluster type is valid, otherwise false
**/
bool is_valid_zcl_cluster(const zcl_cluster_type_t cluster);

/**
 * copy_cluster
 * @brief Allocates memory and performs a deep copy of a ZCL Cluster Type. Memory
 * allocated here must be freed.
 *
 * @param source - the source zcl_cluster_type_t that we want to copy. expected to
 *                 have valid data
 * @param dest - the destination zcl_cluster_type_t where we are copying to
 * @param list_size - capacity of the destination zcl_cluster_type_t list
 *
 * @return the status of the operation. fails if we are trying to copy an invalid
 * ZCL cluster or if we run out of memory
**/
sl_status_t copy_cluster_list(const zcl_cluster_type_t *const source,
                              zcl_cluster_type_t *dest,
                              unsigned int list_size);

/**
 * is_valid_zigbee_node
 * @brief checks if a given zigbee node is valid or not.
 * A zigbee node is considered valid if it has a valid
 * EUI64, at least one valid endpoint and is active.
 *
 * @param node - the node to check
 *
 * @return true if the node is valid, otherwise false
**/
bool is_valid_zigbee_node(const zigbee_node_t node);

/**
 * copy_node
 * @brief Allocates memory and performs a deep copy of a Zigbee Node. Memory
 * allocated here must be freed.
 *
 * @param source - the source zigbee_node that we want to copy. expected to
 *                 have valid data
 * @param dest - the destination zigbee_node where we are copying to
 *
 * @return the status of the operation. fails if we are trying to copy invalid
 * data or if we run out of memory
**/
sl_status_t copy_node(const zigbee_node_t source, zigbee_node_t *dest);

/**
 * is_valid_zigbee_endpoint
 * @brief checks if a given zigbee endpoint is valid or not.
 * For now all endpoints are considered valid
 *
 * @param endpoint - the endpoint to check
 *
 * @return true if the endpoint is valid, otherwise false
**/
bool is_valid_zigbee_endpoint(const zigbee_endpoint_t endpoint);

/**
 * copy_endpoint
 * @brief Allocates memory and performs a deep copy of a Zigbee Endpoint. Memory
 * allocated here must be freed.
 *
 * @param source - the source zigbee_endpoint_t that we want to copy. expected to
 *                 have valid data
 * @param dest - the destination zigbee_endpoint_t where we are copying to
 *
 * @return the status of the operation. fails if we are trying to copy an invalid
 * Zigbee Endpoint or if we run out of memory
**/
sl_status_t copy_endpoint(const zigbee_endpoint_t source,
                          zigbee_endpoint_t *dest);

sl_status_t copy_endpoint_list(const zigbee_endpoint_t *const source,
                               zigbee_endpoint_t *dest,
                               unsigned int list_size);

/**
 * @brief Initialize the ZCL Frame for sending ZCL command.
 *
 * @param frame         Pointer to frame to be initialized.
 * @param command_id    ZCL Command ID to be populated in the frame.
 * @param frame_control ZCL frame control field to be populated.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER if
 * if invalid pointers are passed in.
 */
sl_status_t zigpc_common_zcl_frame_init_command(zcl_frame_t *const frame,
                                                zcl_command_id_t command_id,
                                                uint8_t frame_control);

/**
 * @brief Populate the given ZCL frame with data. The data copied is dictated
 * by the type passed in. Once successfully copied into the frame, the buffer
 * size is incremented based on the size of the data type passed in.
 *
 * @param frame         Pointer to the ZCL frame to be populated.
 * @param type          Type of data to be transferred to the frame.
 * @param data          Pointer to the data to be transferred.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER if
 * if invalid pointers are passed in.
 */
sl_status_t zigpc_common_zcl_frame_fill_data(zcl_frame_t *const frame,
                                             enum zcl_frame_data_type type,
                                             const void *data);

/**
 * @brief Populate the given ZCL frame with data. Copies raw data from
 * a buffer of a specified size
 *
 * @param frame         Pointer to the ZCL frame to be populated.
 * @param data          Pointer to the data to be transferred.
 * @param data_size     The size of data
 *
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_NULL_POINTER if
 * if invalid pointers are passed in. Fails if attempting to copy more data
 * than can be stored in frame
 */
sl_status_t zigpc_common_zcl_frame_fill_raw(zcl_frame_t *const frame,
                                            const uint8_t *const data,
                                            unsigned int data_size);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_COMMON_ZIGBEE_H */

/** @} end zigpc_common_zigbee */
