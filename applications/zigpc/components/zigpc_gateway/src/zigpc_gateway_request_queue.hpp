/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file zigpc_gateway_request_queue.hpp
 * @defgroup zigpc_gateway_call ZigPC Gateway Request Queue
 * @ingroup zigpc_gateway
 * @brief Request Queue API to send messages to EmberAf/ZigbeeHost layer.
 *
 * The public API available to send messages to the ZigbeeHost API via ZigPC
 * Gateway is redirected to the below asynchronous Request Queue
 * (zigpc::gateway::RequestQueue).
 *
 * RequestQueue Storage
 * ---
 * The RequestQueue uses the virtual base class to store API call
 * (zigpc::gateway::Entry). This class provides one virtual
 * function `invoke()` that must be overridden by derived classes. Each
 * ZigbeeHost API call should be wrapped in a separate derived class.
 *
 * For example, the derived class `DiscoverDeviceRequest` uses the `invoke` function
 * to call the zigbeeHostZdoActiveEndpointsRequest APIs using the information
 * persisted in the object in its constructor.
 *
 *
 * RequestQueue Behaviour
 * ---
 * The RequestQueue is initialized as a singleton and is accessible using the
 * static getInstance() function.
 *
 * The ZigPC Gateway public API handlers in `src/zigpc_gateway.cpp` calls the
 * appropriate derived class constructor and creates a shared_ptr instance of
 * that object. This object is passed to the enqueue function in the
 * RequestQueue singleton.
 *
 * The dispatch/dequeuing of the RequestQueue is triggered by the internal
 * Contiki process in response to a ZigbeeHostTick timer used to periodically
 * call the emberAf stack.
 *
 * NOTE: In the dispatch procedure, the member variable RequestQueue::defer_cycles
 * is used to prevent invoking any API calls for the following reasons:
 * - To proportionately call the ZigbeeHostTick() routine for every 1 API call
 *   dispatched (i.e. currently the RequestQueue is setup to call ZigbeeHostTick()
 *   x2 per RequestQueue::Entry dispatched).
 * - To prevent sending any RequestQueue::Entry after getting network
 *   saturation errors; In this case, the call data is not popped from the
 *   stack and a retry of the call will be performed.
 *
 * See constants in zigpc::gateway::RequestQueue for these defer cycle
 * defaults.
 *
 * @{
 */

#ifndef ZIGPC_GATEWAY_CALL_HPP
#define ZIGPC_GATEWAY_CALL_HPP

#include <memory>
#include <queue>
#include <string>

// ZigPC includes
#include <zigpc_common_zigbee.h>

/* ZigbeeHost includes */
#include <zigbee_host.h>

namespace zigpc::gateway
{
/**
 * @brief The singleton ZigbeeHost Request Queue responsible for storing and
 * dispatching calls.
 *
 */
class RequestQueue
{
  public:
  /**
   * @brief Number of cycles to defer under normal circumstances. This is used
   * to increase/decrease the number of times zigbeeHostTick() is called per 1
   * ZigbeeHost API call.
   *
   */
  static const size_t DEFER_CYCLES_DEFAULT = 1U;

  /**
   * @brief Number of cycles to defer when the queue is allowed to send
   * requests either on init or post reset.
   *
   * NOTE: This value is chosen arbitrarily based on experimentation.
   *
   */
  static const size_t DEFER_CYCLES_START_BACKOFF = 20U;

  /**
   * @brief Number of cycles to defer when a return status is indicative of a
   * lost connection to the NCP.
   *
   * NOTE: This value is chosen arbitrarily based on experimentation.
   *
   */
  static const size_t DEFER_CYCLES_LOST_CONNECTION = 50U;

  /**
   * @brief Number of defer cycles when too many messages are in-flight on the
   * network.
   *
   */
  static const size_t DEFER_CYCLES_MESSAGE_LIMIT_REACHED = 10U;

  /**
 * @brief Base class used in the RequestQueue to invoke ZigbeeHost API calls.
 *
 */
  class Entry
  {
    const char *label;         /** < Call label */
    size_t send_attempts = 0U; /**< Times tried to successfully send the call */
    public:
    explicit Entry(const char *label);
    virtual ~Entry() = default;

    /**
   * @brief Get the Name of the call.
   *
   * @return const char*
   */
    const char *getLabel(void) const;

    /**
   * @brief Get the number of attempts made to send the message.
   *
   * @return size_t
   */
    size_t getSendAttempts(void) const;

    /**
   * @brief Increment send attempts.
   *
   */
    void incrementSendAttempt(void);

    /**
   * @brief Invoke/execute the ZigbeeHost call using the infomation stored in
   * the object.
   *
   * @return sl_status_t
   */
    virtual sl_status_t invoke(void) = 0;
  };

  RequestQueue();
  RequestQueue(RequestQueue &other) = delete;
  void operator=(const RequestQueue &) = delete;

  /**
   * @brief Retrieve singleton instance.
   *
   * @return RequestQueue&
   */
  static RequestQueue &getInstance();

  /**
   * @brief Prevent sending any ZigbeeHost requests due to the NCP being reset.
   *
   */
  void stopDispatching();

  /**
   * @brief Configure queue to allow sending ZigbeeHost requests.
   *
   * NOTE: This must be called after initializing the queue.
   *
   */
  void startDispatching(void);

  /**
   * @brief Store call data to the queue that is used to dispatch calls
   * to EmberAf/ZigbeeHost API.
   *
   * @param call  Shared pointer to ZigbeeHost derived call data.
   */
  void enqueue(std::shared_ptr<Entry> call);

  /**
   * @brief Dispatch a EmberAf/ZigbeeHost call from the queue.
   *
   * This dispatch routine is responsible for sending calls to the
   * EmberAf/ZigbeeHost API.
   *
   * NOTE: If dispatching a EmberAf/ZigbeeHost call returns SL_STATUS_ZIGBEE_MAX_MESSAGE_LIMIT_REACHED ,
   * the call data is assumed to have failed since there are too many messages
   * in-flight on the Zigbee network. To rectify this, the call is not dequeued and
   * and call-defer-cycle counter is increased to defer making any calls for that
   * many iterations.
   *
   * @return sl_status_t SL_STATUS_OK on EmberAf/ZigbeeHost call success,
   * SL_STATUS_TRANSMIT_BUSY if there are too many messages on the Zigbee
   * network, or SL_STATUS_FAIL if the EmberAf/ZigbeeHost call failed.
   */
  sl_status_t dispatch(void);

  private:
  std::queue<std::shared_ptr<Entry>> queue; /**< ZigbeeHost call data */
  size_t defer_cycles = 0U; /**< Number of cycles to defer before dispatching
                                 ZigbeeHost request(default=0U) */
  bool dispatching_allowed = false; /**< Indicate if Requests can be
                                         dispatched to ZigbeeHost layer
                                         (default=FALSE) */
};

/**
 * @brief Call to initialize the network.
 *
 */
class NetworkInitRequest : public RequestQueue::Entry
{
  public:
  explicit NetworkInitRequest(void);
  ~NetworkInitRequest() override = default;

  sl_status_t invoke(void) override;
};

/**
 * @brief Call to interview device on the network.
 *
 */
class DiscoverDeviceRequest : public RequestQueue::Entry
{
  private:
  zigbee_eui64_t eui64; /**< Device ID */

  public:
  explicit DiscoverDeviceRequest(const zigbee_eui64_t eui64);
  ~DiscoverDeviceRequest() override = default;

  sl_status_t invoke(void) override;
};

/**
 * @brief Call to discover endpoint on the network.
 *
 */
class DiscoverEndpointRequest : public RequestQueue::Entry
{
  private:
  zigbee_eui64_t eui64;             /**< Device ID */
  zigbee_endpoint_id_t endpoint_id; /**< Endpoint ID */

  public:
  explicit DiscoverEndpointRequest(const zigbee_eui64_t eui64,
                                   zigbee_endpoint_id_t endpoint_id);
  ~DiscoverEndpointRequest() override = default;

  sl_status_t invoke(void) override;
};

/**
   * @brief Call to remove device on the network.
   *
   */
class DeviceRemoveRequest : public RequestQueue::Entry
{
  private:
  zigbee_eui64_t eui64; /**< Device ID */

  public:
  explicit DeviceRemoveRequest(const zigbee_eui64_t eui64);
  ~DeviceRemoveRequest() override = default;

  sl_status_t invoke(void) override;
};

/**
 * @brief Call to send one-way bind request from device to the gateway.
 *
 */
class BindingRequestRequest : public RequestQueue::Entry
{
  private:
  zigbee_eui64_t source_eui64;             /**< Device ID */
  zigbee_endpoint_id_t source_endpoint_id; /**< Endpoint ID */
  zcl_cluster_id_t cluster_id;      /**< Cluster ID */
  zigbee_eui64_t dest_eui64;             /**< Device ID */
  zigbee_endpoint_id_t dest_endpoint_id; /**< Endpoint ID */
  bool is_binding_req;                  /**True if bind, False if unbind */

  public:
  explicit BindingRequestRequest(const zigbee_eui64_t source_eui64,
                                 zigbee_endpoint_id_t source_endpoint_id,
                                 zcl_cluster_id_t cluster_id,
                                 const zigbee_eui64_t dest_eui64,
                                 zigbee_endpoint_id_t dest_endpoint_id,
                                 bool is_binding_req);
  ~BindingRequestRequest() override = default;

  sl_status_t invoke(void) override;
};

/**
 * @brief Call to send ZCL Configure Reporting message to device on the network.
 *
 */
class ZCLConfigureReportingRequest : public RequestQueue::Entry
{
  private:
  zigbee_eui64_t eui64;             /**< Device ID */
  zigbee_endpoint_id_t endpoint_id; /**< Endpoint ID */
  zcl_cluster_id_t cluster_id;      /**< Cluster ID */
  zcl_frame_t frame;                /**< ZCL Configure Reporting Data */

  public:
  explicit ZCLConfigureReportingRequest(const zigbee_eui64_t eui64,
                                        zigbee_endpoint_id_t endpoint_id,
                                        zcl_cluster_id_t cluster_id,
                                        const uint8_t *report_record,
                                        unsigned int record_size);
  ~ZCLConfigureReportingRequest() override = default;

  sl_status_t invoke(void) override;
};

/**
 * @brief Call to send generic ZCL frame to device on the network.
 *
 */
class ZCLFrameUnicastRequest : public RequestQueue::Entry
{
  private:
  zigbee_eui64_t eui64;             /**< Device ID */
  zigbee_endpoint_id_t endpoint_id; /**< Endpoint ID */
  zcl_cluster_id_t cluster_id;      /**< Cluster ID */
  const zcl_frame_t frame;          /**< ZCL Frame Data */

  public:
  explicit ZCLFrameUnicastRequest(const zigbee_eui64_t eui64,
                                  zigbee_endpoint_id_t endpoint_id,
                                  zcl_cluster_id_t cluster_id,
                                  const zcl_frame_t &frame);
  ~ZCLFrameUnicastRequest() override = default;

  sl_status_t invoke(void) override;
};

/**
 * @brief Call to send generic ZCL frame to a Zigbee Group on the network.
 *
 */
class ZCLFrameMulticastRequest : public RequestQueue::Entry
{
  private:
  zigbee_group_id_t group_id;  /**< Group ID */
  zcl_cluster_id_t cluster_id; /**< Cluster ID */
  const zcl_frame_t frame;     /**< ZCL Frame Data */

  public:
  explicit ZCLFrameMulticastRequest(zigbee_group_id_t group_id,
                                    zcl_cluster_id_t cluster_id,
                                    const zcl_frame_t &frame);
  ~ZCLFrameMulticastRequest() override = default;

  sl_status_t invoke(void) override;
};

/**
 * @brief Call to track OTA image by the OTA server plugin.
 *
 */
class AddOTAImageRequest : public RequestQueue::Entry
{
  private:
  std::string filename; /**< OTA Image path to track */

  public:
  explicit AddOTAImageRequest(std::string &filename);
  ~AddOTAImageRequest() override = default;

  sl_status_t invoke(void) override;
};

}  // namespace zigpc::gateway

#endif  // ZIGPC_GATEWAY_CALL_HPP

/** @} end zigpc_gateway_call */
