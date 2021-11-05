/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#ifndef ATTRIBUTE_MANAGEMENT_INT_H
#define ATTRIBUTE_MANAGEMENT_INT_H

#include "sl_status.h"
#include "sys/ctimer.h"

#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

static const unsigned int ZIGPC_ATTR_MGMT_DELAY_READ_ATTRIBUTES
  = CLOCK_SECOND * 5;

static const unsigned int ZIGPC_ATTR_MGMT_POLL_PERIOD_MS
  = CLOCK_SECOND;  //1000ms

/**
 * @brief Limit of Read Attribute Records to send per ZCL Message
 *
 * Due to the ZCL messages only having a maximum payload of 70-80 bytes
 * (depending on added manfacuturer-specific header information), and since
 * each attribute record is 2-bytes, the maximum number of attribute read
 * records to set per ZCL message should be limited. The current limit is
 * chosen arbitrarily to allow the attribute read records to reliabily sent
 * to the device.
 *
 */
static const unsigned int ZIGPC_ATTRMGMT_READ_RECORDS_LIMIT_PER_MESSAGE = 25;

/**
 * @brief Limit of Configure Reporting Records to send per ZCL Message
 *
 * Due to the ZCL messages only having a maximum payload of 70-80 bytes
 * (depending on added manfacuturer-specific header information), and since
 * each attribute record is 8-bytes, the maximum number of attribute report
 * records to set per ZCL message should be limited. The current limit is
 * chosen arbitrarily to allow the configure report records to reliabily sent
 * to the device.
 *
 */
static const unsigned int ZIGPC_ATTRMGMT_REPORT_RECORDS_LIMIT_PER_MESSAGE = 5;

/**
 * @brief Default Configure Reporting Record Minimum Interval (Seconds).
 *
 * NOTE: This value is chosen arbitrarily and will likely be configurable
 * in the future.
 *
 */
static const uint16_t ZIGPC_ATTRMGMT_REPORT_INTERVAL_MIN_DEFAULT = 10;

/**
 * @brief Default Configure Reporting Record Maximum Interval (Seconds).
 *
 * NOTE: This value is chosen arbitrarily and will likely be configurable
 * in the future.
 *
 */
static const uint16_t ZIGPC_ATTRMGMT_REPORT_INTERVAL_MAX_DEFAULT = 60;

/**
 * @brief Default Configure Reporting Record Reportable Change to allow
 * any change to be reported.
 *
 * NOTE: This value is chosen arbitrarily and will likely be configurable
 * in the future.
 *
 */
static const uint32_t ZIGPC_ATTRMGMT_REPORT_CHANGE_DEFAULT = 0x0000;

/**
 * @brief Default poll list size. The maximum number of clusters that
 * can be stoed in the poll list
 */
static const uint32_t ZIGPC_ATTRMGMT_DEFAULT_POLL_SIZE = 256;

/**
 * attr_mgmt_evt
 *
 * @brief the enum representing the events handled by the attribute
 * management Contiki process. Entries outside of this data structure
 * are not valid
**/
enum attr_mgmt_evt {
  ZIGPC_ATTR_MGMT_EVT_INIT_NODE = 0,
  ZIGPC_ATTR_MGMT_EVT_INIT_ENDPOINT,

  /**
   * @brief Event corresponding to a new attribute update message received.
   *
   */
  ZIGPC_ATTR_MGMT_EVT_ATTR_UPDATE,
  ZIGPC_ATTR_MGMT_EVT_CONFIGURE_RESPONSE,
  ZIGPC_ATTR_MGMT_EVT_READ_ATTRIBUTES_COMMAND,
};

/**
 * node_config_report_data_t
 *
 * @brief the data structure representing all data required to handle a
 * "Configure Reports - Node" event, represented by
 * ATTR_MGMT_EVT_INIT_NODE
**/
typedef struct {
  zigbee_node_t node;
} node_config_report_data_t;

/**
 * endpoint_config_report_data_t
 *
 * @brief the data structure representing all data required to handle a
 * "Configure Reports - Endpoint" event, represented by
 * ATTR_MGMT_EVT_INIT_ENDPOINT
**/
typedef struct {
  zigbee_eui64_t eui64;
  zigbee_endpoint_t endpoint;
} endpoint_config_report_data_t;

/**
 * zigpc_attr_mgmt_read_data_t
 *
 * @brief the structure used to initialize attribute data. In order to
 * populate initial values of all attributes, it is necessary to request
 * attributes after a delay. This structure holds all information related
 * to this operation.
**/
typedef struct {
  zigbee_eui64_t eui64;
  zigbee_endpoint_id_t endpoint_id;
  zcl_cluster_id_t cluster_id;
  struct ctimer timer;
} zigpc_attr_mgmt_read_data_t;

/**
 * zigpc_attr_mgmt_write_data_t
 *
 * @brief the structure used to send a write attribute command.
 * Contains all the information from MQTT and expected to be
 * parsed into a valid ZCL frame.
**/
typedef struct {
  zigbee_eui64_t eui64;
  zigbee_endpoint_id_t endpoint_id;
  char *cluster_string;
  unsigned int cluster_string_size;
  char *attributes_string;
  unsigned int attr_string_size;
} zigpc_attr_mgmt_write_data_t;

/**
 * attribute_report_t
 *
 * @brief the structure used to represent a report record used by the
 * ZCL application framework when sending attribute/report information
**/
typedef struct {
  zigbee_eui64_t eui64;
  zigbee_endpoint_id_t endpoint_id;
  zcl_cluster_id_t cluster_id;
  zcl_attribute_id_t attribute_id;
  enum e_zcl_attribute_type type;
  uint8_t *data;
} attribute_report_t;

/**
 * @brief the structure used to represent the data received on a report
 * for a changed attribute
 **/
typedef struct {
  zigbee_eui64_t eui64;             /**< Device identifier */
  zigbee_endpoint_id_t endpoint_id; /**< Endpoint idenfitier */
  zcl_cluster_id_t cluster_id;      /**< ZCL cluster identifier */
  bool is_read_response;            /**< Is frame a read response */
  zcl_frame_t frame;                /**< ZCL frame buffer */
} zigpc_attrmgmt_on_frame_receive_t;

/**
 * @brief Send an event and associated data to the attribute management Contiki
 * process. Based on the data and data_size passed in, memory will be
 * dynamically allocated in the heap and sent in order to be processed.
 *
 * NOTE: The attribute management process will free the memory allocated for data
 * in the process loop.
 *
 * @param event         Event type to send
 * @param data          Pointer to data to be sent with the above event.
 * @param data_size     Size of the data passed in.
 * @return SL_STATUS_OK if the event is sent to the Contiki process,
 * SL_STATUS_NULL_POINTER on invalid arguments passed in, or
 * SL_STATUS_EMPTY if the data_size to send is 0.
 */
sl_status_t zigpc_attrmgmt_process_send_event(const enum attr_mgmt_evt event,
                                              const void *data,
                                              size_t data_size);

/**
 * @brief Determine the number of records to send per ZCL message.
 *
 * @param start_index Starting index of record to send.
 * @param count       Total number of records in list.
 * @param limit       Maximum number of records to send in each message.
 * @return size_t     The number of records to send.
 */
size_t zigpc_attrmgmt_get_records_to_send_in_message(size_t start_index,
                                                     size_t count,
                                                     size_t limit);

/**
 * @brief zigpc_attrmgmt_configure_reports_for_endpoint - API for sending a ZCL command
 * to the zigpc_gateway / z3gateway to configure reports for a single
 * endpoint. This is an internal API, expected to be abstracted away.
 *
 * @param eui64     - the eui64, whose endpoints should be configured for
                      reports
 * @param endpoint - the endpoint, expected to give reports
 *
 * @return SL_STATUS_OK if the command was sent successfully,
 * SL_STATUS_FAIL otherwise
**/
sl_status_t zigpc_attrmgmt_configure_reports_for_endpoint(
  const zigbee_eui64_t eui64, const zigbee_endpoint_t endpoint);

/**
 * @brief node_send_report_config_cmd - API for sending a ZCL command
 * to the zigpc_gateway / z3gateway to configure reports for a node. This
 * is an internal API, expected to be abstracted away.
 *
 * @param node - the node, whose endpoints should be configured for reports
 *
 * @return SL_STATUS_OK if the command was sent successfully,
 * SL_STATUS_FAIL otherwise
**/
sl_status_t node_send_report_config_cmd(const zigbee_node_t node);

/**
 * @brief Request to send a Read Attribute commands to the associated device,
 * endpoint, and cluster combination.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Device endpoint identifier.
 * @param cluster_id    Cluster identifer to read attributes from.
 * @return sl_status_t SL_STATUS_OK if the request is sent.
 */
sl_status_t zigpc_attribute_management_read_cluster_attributes(
  const zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id);

/**
 * @brief Callback invoked by zigpc_attrmgmt_send_delayed_read_command after
 * delay of ZIGPC_ATTR_MGMT_DELAY_READ_ATTRIBUTES. This handler will call
 * zigpc_attribute_management_read_cluster_attributes afterwards.
 *
 * @param read_data Pointer to zigpc_attr_mgmt_read_data_t
 */
void zigpc_attrmgmt_send_delayed_read_command_callback(void *read_data);

/**
 * @brief Handler for a ZCL Global Read attribute command after a delay of
 * ZIGPC_ATTR_MGMT_DELAY_READ_ATTRIBUTES. This handler will call
 * zigpc_attrmgmt_send_delayed_read_command_callback after the delay.
 *
 * @param eui64         Target device identifier.
 * @param endpoint_id   Target endpoint identifier.
 * @param cluster_id    Target ZCL cluster identifier.
 */
void zigpc_attrmgmt_send_delayed_read_command(const zigbee_eui64_t eui64,
                                              zigbee_endpoint_id_t endpoint_id,
                                              zcl_cluster_id_t cluster_id);

/**
 * @brief Send a ZCL Global Read Attribute Command to the associated device,
 * endpoint, and cluster combination.
 *
 * @param eui64         Device identifier.
 * @param endpoint_id   Device endpoint identifier.
 * @param cluster_id    Cluster identifer to read attributes from.
 * @return sl_status_t SL_STATUS_OK if the read attributes command is sent,
 * SL_STATUS_NULL_POINTER if invalid parameters are passed in,
 * SL_STATUS_NOT_SUPPORTED if there is no supported attributes for the given
 * cluster.
 */
sl_status_t
  zigpc_attrmgmt_send_read_attributes_command(const zigbee_eui64_t eui64,
                                              zigbee_endpoint_id_t endpoint_id,
                                              zcl_cluster_id_t cluster_id);

/**
 * @brief Process Attribute update response frames received from the a Device.
 * This function is capable of processing the following Response record lists:
 * - ReadAttributeResponse records
 * - ReportAttribute records
 *
 * @param eui64             Device identifier.
 * @param endpoint_id       Device endpoint identifier.
 * @param cluster_id        Cluster identifer to read attributes from.
 * @param is_read_response  If the passed frame is a ReadAttributeResponse
 * @param frame             ZCL Frame containing read attribute records.
 * @return sl_status_t      SL_STATUS_OK on successfully processing and updating
 * attributes on MQTT, SL_STATUS_NULL_POINTER if invalid arguments are supplied.
 */
sl_status_t
  zigpc_attrmgmt_receive_attribute_frame(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_id_t cluster_id,
                                         bool is_read_response,
                                         const zcl_frame_t *frame);

/**
 * @brief zigpc_attrmgmt_receive_configure_response_frame
 * Process a configure attribute response frame
 * Accepts a ZCL frame for the response to a configure
 * reports command. On a failed configure, adds the given EUI64,
 * endpoint and cluster combination to the poll lis
 *
 * @param eui64         Device identifier.
 * @param endpoint_id       Device endpoint identifier
 * @param cluster_id        Cluster identifer of the source from
 * @param frame             ZCL Frame containing configure report response
 */
sl_status_t zigpc_attrmgmt_receive_configure_response_frame(
  const zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id,
  const zcl_frame_t *frame);

#ifdef COMPILE_UNUSED_CODE

/**
 * @brief update_attribute_from_report - the API to update the internal state of
 * an attribute of a cluster on an endpoint, from the report data received from
 * the Zigbee network
 *
 * @return SL_STATUS_OK if the report was successfully read, otherwise
 * SL_STATUS_FAIL
**/
sl_status_t update_attribute_from_report(const attribute_report_t report);

#endif  // COMPILE_UNUSED_CODE

/**
 * @brief attr_mgmt_gateway_init_observer - the function that initiates all
 * observers from the zigpc_gateway. Required to process a ReadAttribute
 * Reponse, an Attribute Report or other data received asynchronously from
 * the Zigbee network.
**/
sl_status_t attr_mgmt_gateway_init_observer(void);

/**
 * @brief Publish Reported Attribute Values
 *
 * Update the attribute value for a node, ep, cluster combination. This
 * function will call the uic_dotdot_mqtt publish APIs in order to format
 * and publish the reported attribute value.
 *
 * @return SL_STATUS_OK on successful publish of a supported cluster attribute,
 * another error if not.
 */
sl_status_t
  zigpc_attrmgmt_publish_reported(const zigbee_eui64_t eui64,
                                  const zigbee_endpoint_id_t endpoint_id,
                                  const zcl_cluster_id_t cluster_id,
                                  const zcl_attribute_id_t attr_id,
                                  const uint8_t *attr_value);

/**
 * @brief zigpc_attrmgmt_get_poll_list_size
 *
 * Get the size of the current list we are polling
 * with ReadAttributes
 *
 * @return the number of clusters on the list we are polling
 **/
size_t zigpc_attrmgmt_get_poll_list_current_size();

/**
 * @brief zigpc_attrmgmt_get_poll_list_Max_size
 *
 * Get the maximum size of the polling list
 *
 * @return the maximum number of clusters the poll list can contain
 **/
size_t zigpc_attrmgmt_get_poll_list_max_size();

/**
 * @brief zigpc_attrmgmt_send_poll_attributes
 *
 * Send a ReadAttribute command to the next entry in
 * the polling list
 *
 * @return SL_STATUS_OK if the command could be sent properly
 **/
sl_status_t zigpc_attrmgmt_send_poll_attributes();

/**
 * @brief zigpc_attrmgmt_add_poll_entry
 *
 * Add a new entry to be polled by ReadAttribute commands
 * Will be put at the end of the list
 *
 * @param eui64 - the eui64 of the node
 * @param endpoint_id - the endpoint specified
 * @param cluster_id - the cluster specified
 *
 * @return SL_STATUS_OK if the new entry could be added properly
 **/
sl_status_t zigpc_attrmgmt_add_poll_entry(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t endpoint_id,
                                          zcl_cluster_id_t cluster_id);

/**
 * @briefzigpc_attrmgmt_remove_poll_entry 
 *
 * Remove a eui64, endpoint and cluster combination from the poll list
 *
 * @param eui64 - the eui64 of the node
 * @param endpoint_id - the endpoint specified
 * @param cluster_id - the cluster specified
 *
 * @return SL_STATUS_OK if the new entry could be removed properly
 **/
sl_status_t zigpc_attrmgmt_remove_poll_entry(const zigbee_eui64_t eui64,
                                             zigbee_endpoint_id_t endpoint_id,
                                             zcl_cluster_id_t cluster_id);

/**
 * @brief zigpc_attrmgmt_empty_poll_list
 *
 * Empty the polling list
 **/
void zigpc_attrmgmt_empty_poll_list();

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_MANAGEMENT_INT_H
