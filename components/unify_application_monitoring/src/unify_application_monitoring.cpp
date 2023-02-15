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
#include "unify_application_monitoring.h"

// Unify Components
#include "uic_mqtt.h"
#include "dotdot_mqtt.h"
#include "uic_version.h"

// Generic includes
#include <string>
#include <string.h>

// Constants
constexpr const char EMPTY_SUPPORTED_COMMANDS[] = R"({"value":[]})";

// Private variables
namespace
{
bool unify_application_monitoring_enabled = false;
const char *mqtt_client_id                = nullptr;
std::string base_topic;

// ApplicationMonitoring Attributes
bool application_connected      = false;
std::string application_name    = std::string("Unknown Application");
std::string application_version = std::string(UIC_VERSION);
}  // namespace

///////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////
static void publish_application_monitoring_application_connected_attribute()
{
  if (false == unify_application_monitoring_enabled) {
    return;
  }
  mqtt_client_id = uic_mqtt_get_client_id();
  if (mqtt_client_id == nullptr) {
    return;
  }
  base_topic = "ucl/by-mqtt-client/" + std::string(mqtt_client_id);
  uic_mqtt_dotdot_application_monitoring_application_connected_publish(
    base_topic.c_str(),
    application_connected,
    UCL_MQTT_PUBLISH_TYPE_ALL);
}

static void publish_application_monitoring_application_name_attribute()
{
  if (false == unify_application_monitoring_enabled) {
    return;
  }
  mqtt_client_id = uic_mqtt_get_client_id();
  if (mqtt_client_id == nullptr) {
    return;
  }
  base_topic = "ucl/by-mqtt-client/" + std::string(mqtt_client_id);
  uic_mqtt_dotdot_application_monitoring_application_name_publish(
    base_topic.c_str(),
    application_name.c_str(),
    UCL_MQTT_PUBLISH_TYPE_ALL);
}

static void publish_application_monitoring_application_version_attribute()
{
  if (false == unify_application_monitoring_enabled) {
    return;
  }
  mqtt_client_id = uic_mqtt_get_client_id();
  if (mqtt_client_id == nullptr) {
    return;
  }
  base_topic = "ucl/by-mqtt-client/" + std::string(mqtt_client_id);
  uic_mqtt_dotdot_application_monitoring_application_version_publish(
    base_topic.c_str(),
    application_version.c_str(),
    UCL_MQTT_PUBLISH_TYPE_ALL);
}
static void publish_application_monitoring_cluster_revision_attribute()
{
  if (false == unify_application_monitoring_enabled) {
    return;
  }
  mqtt_client_id = uic_mqtt_get_client_id();
  if (mqtt_client_id == nullptr) {
    return;
  }
  base_topic = "ucl/by-mqtt-client/" + std::string(mqtt_client_id);
  uic_mqtt_dotdot_application_monitoring_publish_cluster_revision(
    base_topic.c_str(),
    APPLICATION_MONITORING_CLUSTER_REVISION);
}

static void publish_application_monitoring_supported_commands()
{
  if (false == unify_application_monitoring_enabled) {
    return;
  }
  mqtt_client_id = uic_mqtt_get_client_id();
  if (mqtt_client_id == nullptr) {
    return;
  }
  base_topic        = "ucl/by-mqtt-client/" + std::string(mqtt_client_id);
  std::string topic = base_topic + "/ApplicationMonitoring/SupportedCommands";
  uic_mqtt_publish(topic.c_str(),
                   EMPTY_SUPPORTED_COMMANDS,
                   strlen(EMPTY_SUPPORTED_COMMANDS),
                   true);
}

static void publish_application_monitoring_application_attributes()
{
  publish_application_monitoring_application_connected_attribute();
  publish_application_monitoring_application_name_attribute();
  publish_application_monitoring_application_version_attribute();
  publish_application_monitoring_cluster_revision_attribute();
}

///////////////////////////////////////////////////////////////////////////////
// MQTT Callback functions
///////////////////////////////////////////////////////////////////////////////
static void on_application_disconnection()
{
  application_connected = false;
  publish_application_monitoring_application_connected_attribute();
}
static void on_application_connected()
{
  application_connected = uic_mqtt_is_connected_to_broker();
  publish_application_monitoring_application_connected_attribute();
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t unify_application_monitoring_init()
{
  unify_application_monitoring_enabled = true;
  application_connected                = uic_mqtt_is_connected_to_broker();
  uic_mqtt_set_before_disconnect_callback(&on_application_disconnection);
  uic_mqtt_set_after_connect_callback(&on_application_connected);
  publish_application_monitoring_application_attributes();
  publish_application_monitoring_supported_commands();
  return SL_STATUS_OK;
}

void unify_application_monitoring_teardown()
{
  unify_application_monitoring_enabled = false;
  mqtt_client_id                       = uic_mqtt_get_client_id();
  if (mqtt_client_id == nullptr) {
    return;
  }
  base_topic = "ucl/by-mqtt-client/" + std::string(mqtt_client_id);
  uic_mqtt_unretain(base_topic.c_str());
}

void unify_application_monitoring_set_application_name(
  const char *_application_name)
{
  application_name = std::string(_application_name);
  publish_application_monitoring_application_name_attribute();
}

void unify_application_monitoring_set_application_version(
  const char *_application_version)
{
  application_version = std::string(_application_version);
  publish_application_monitoring_application_version_attribute();
}