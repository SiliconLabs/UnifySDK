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

/*
HOW-TO and class relationship of diagnostic manager :
The goal of this component is to receive and send diagnosis information via mqtt

This component is composed of three parts, the manager, the metrics and a mqtt helper.

The manager :
  -Contains all the metric in a map
  -Receives request as a string of metric array from mqtt_helper
  -Ask the relevant metrics to update their values and notify back the manager
  -Publishes the resulting information under the correct unid and topic

The Metric :
  - Are derived from the base metric classes
  - Get relevant information and then notify the manager that the info is ready
  
The MQTT Helper
  -Contains the process which handles request
  -Initialises the whole component on a network init event
  -Is used to add all metrics in the map in the initialisation
  -Subscribes to the correct request topic


*/
#ifndef ZIGPC_DIAGNOSTICS_MQTT_HELPER
#define ZIGPC_DIAGNOSTICS_MQTT_HELPER

#include "zigpc_diagnostics.h"
#include <string>

#include <vector>

void uic_diagnostics_mqtt_callback(const char *topic,
                                   const char *payload,
                                   size_t payload_size);

sl_status_t uic_diagnostics_setup(std::string unid);

void uic_metric_init();

std::vector<std::string>
  uic_diagnostic_request_to_strings(std::string *payload_string);

#endif
