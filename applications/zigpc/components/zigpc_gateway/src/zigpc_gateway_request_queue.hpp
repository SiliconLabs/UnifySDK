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
 * @brief Request Queue API to send messages to EmberAf/Z3Gateway layer.
 *
 * The public API available to send messages to the Z3Gateway API via ZigPC
 * Gateway is redirected to the below asynchronous Request Queue
 * (zigpc::gateway::RequestQueue).
 *
 * RequestQueue Storage
 * ---
 * The RequestQueue uses the virtual base class to store API call
 * (zigpc::gateway::Entry). This class provides one virtual
 * function `invoke()` that must be overridden by derived classes. Each
 * Z3Gateway API call should be wrapped in a separate derived class.
 *
 * For example, the derived class `PermitJoinRequest` uses the `invoke` function
 * to call the z3gatewayTrustCenterJoinOpen/Close APIs using the information
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
 * Contiki process in response to a Z3GatewayTick timer used to periodically
 * call the emberAf stack.
 *
 * NOTE: In the dispatch procedure, the member variable RequestQueue::defer_cycles
 * is used to prevent invoking any API calls for the following reasons:
 * - To proportionately call the Z3GatewayTick() routine for every 1 API call
 *   dispatched (i.e. currently the RequestQueue is setup to call Z3GatewayTick()
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

// ZigPC includes
#include <zigpc_common_zigbee.h>

/* Z3gateway includes */
#include <z3gateway.h>

namespace zigpc::gateway
{
/**
 * @brief The singleton Z3Gateway Request Queue responsible for storing and
 * dispatching calls.
 *
 */
class RequestQueue
{
  public:
  /**
   * @brief Number of cycles to defer under normal circumstances. This is used
   * to increase/decrease the number of times z3gatewayTick() is called per 1
   * Z3Gateway API call.
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
 * @brief Base class used in the RequestQueue to invoke Z3Gateway API calls.
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
   * @brief Invoke/execute the Z3Gateway call using the infomation stored in
   * the object.
   *
   * @return EmberStatus
   */
    virtual EmberStatus invoke(void) = 0;
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
   * @brief Prevent sending any Z3Gateway requests due to the NCP being reset.
   *
   */
  void stopDispatching();

  /**
   * @brief Configure queue to allow sending Z3Gateway requests.
   *
   * NOTE: This must be called after initializing the queue.
   *
   */
  void startDispatching(void);

  /**
   * @brief Store call data to the queue that is used to dispatch calls
   * to EmberAf/Z3Gateway API.
   *
   * @param call  Shared pointer to Z3Gateway derived call data.
   */
  void enqueue(std::shared_ptr<Entry> call);

  /**
   * @brief Dispatch a EmberAf/Z3Gateway call from the queue.
   *
   * This dispatch routine is responsible for sending calls to the
   * EmberAf/Z3Gateway API.
   *
   * NOTE: If dispatching a EmberAf/Z3Gateway call returns EMBER_MAX_MESSAGE_LIMIT_REACHED,
   * the call data is assumed to have failed since there are too many messages
   * in-flight on the Zigbee network. To rectify this, the call is not dequeued and
   * and call-defer-cycle counter is increased to defer making any calls for that
   * many iterations.
   *
   * @return sl_status_t SL_STATUS_OK on EmberAf/Z3Gateway call success,
   * SL_STATUS_TRANSMIT_BUSY if there are too many messages on the Zigbee
   * network, or SL_STATUS_FAIL if the EmberAf/Z3Gateway call failed.
   */
  sl_status_t dispatch(void);

  private:
  std::queue<std::shared_ptr<Entry>> queue; /**< Z3Gateway call data */
  size_t defer_cycles = 0U; /**< Number of cycles to defer before dispatching
                                 Z3Gateway request(default=0U) */
  bool dispatching_allowed = false; /**< Indicate if Requests can be
                                         dispatched to Z3Gateway layer
                                         (default=FALSE) */
};

/**
 * @brief Call to initialize the network.
 *
 */
class NetworkInitRequest : public RequestQueue::Entry
{
  public:
  NetworkInitRequest(void);
  virtual ~NetworkInitRequest() = default;

  EmberStatus invoke(void) override;
};

/**
 * @brief Call to allow/deny devices from joining the network.
 *
 */
class PermitJoinRequest : public RequestQueue::Entry
{
  private:
  bool enable_joins; /**< Enable/Disable Joins */

  public:
  PermitJoinRequest(bool enable_joins);
  virtual ~PermitJoinRequest() = default;

  EmberStatus invoke(void) override;
};

/**
 * @brief Call to add install code to the gateway.
 *
 */
class AddInstallCodeRequest : public RequestQueue::Entry
{
  private:
  zigbee_eui64_t eui64;               /**< Device ID */
  zigbee_install_code_t install_code; /**< Install code buffer */
  uint8_t install_code_length;        /**< Install code length */

  public:
  AddInstallCodeRequest(const zigbee_eui64_t eui64,
                        const zigbee_install_code_t install_code,
                        uint8_t install_code_length);
  virtual ~AddInstallCodeRequest() = default;

  EmberStatus invoke(void) override;
};

/**
 * @brief Call to interview device on the network.
 *
 */
class DeviceInterviewRequest : public RequestQueue::Entry
{
  private:
  zigbee_eui64_t eui64; /**< Device ID */

  public:
  DeviceInterviewRequest(const zigbee_eui64_t eui64);
  virtual ~DeviceInterviewRequest() = default;

  EmberStatus invoke(void) override;
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
  DeviceRemoveRequest(const zigbee_eui64_t eui64);
  virtual ~DeviceRemoveRequest() = default;

  EmberStatus invoke(void) override;
};

/**
 * @brief Call to send one-way bind request from device to the gateway.
 *
 */
class BindingRequestRequest : public RequestQueue::Entry
{
  private:
  zigbee_eui64_t eui64;             /**< Device ID */
  zigbee_endpoint_id_t endpoint_id; /**< Endpoint ID */
  zcl_cluster_id_t cluster_id;      /**< Cluster ID */

  public:
  BindingRequestRequest(const zigbee_eui64_t eui64,
                        zigbee_endpoint_id_t endpoint_id,
                        zcl_cluster_id_t cluster_id);
  virtual ~BindingRequestRequest() = default;

  EmberStatus invoke(void) override;
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
  ZCLConfigureReportingRequest(const zigbee_eui64_t eui64,
                               zigbee_endpoint_id_t endpoint_id,
                               zcl_cluster_id_t cluster_id,
                               const uint8_t *report_record,
                               unsigned int record_size);
  virtual ~ZCLConfigureReportingRequest() = default;

  EmberStatus invoke(void) override;
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
  ZCLFrameUnicastRequest(const zigbee_eui64_t eui64,
                         zigbee_endpoint_id_t endpoint_id,
                         zcl_cluster_id_t cluster_id,
                         const zcl_frame_t &frame);
  virtual ~ZCLFrameUnicastRequest() = default;

  EmberStatus invoke(void) override;
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
  ZCLFrameMulticastRequest(zigbee_group_id_t group_id,
                           zcl_cluster_id_t cluster_id,
                           const zcl_frame_t &frame);
  virtual ~ZCLFrameMulticastRequest() = default;

  EmberStatus invoke(void) override;
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
  AddOTAImageRequest(std::string &filename);
  virtual ~AddOTAImageRequest() = default;

  EmberStatus invoke(void) override;
};

}  // namespace zigpc::gateway

#endif  // ZIGPC_GATEWAY_CALL_HPP

/** @} end zigpc_gateway_call */
