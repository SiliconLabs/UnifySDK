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
#ifndef UIC_MQTT_INT_H
#define UIC_MQTT_INT_H

#include "mqtt_client.h"

int uic_mqtt_process_post(const int event, void *procdata);
void uic_mqtt_process_post_delayed(const int event,
                                   const unsigned long milliseconds);
int uic_mqtt_process_count_events(const int event, void *procdata);

void uic_mqtt_process_on_connect(mqtt_client_t instance,
                                 const int file_descriptor);
void uic_mqtt_process_on_disconnect(mqtt_client_t instance,
                                    const int file_descriptor);

#endif  // UIC_MQTT_INT_H
