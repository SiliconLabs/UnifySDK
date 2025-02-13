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

#ifndef EED_MATTER_DEVICE_TYPE_H
#define EED_MATTER_DEVICE_TYPE_H

#ifdef __cplusplus
#include <string>
#include <map>
#include <vector>

extern "C" {
#endif

#include "sl_status.h"

// To get the cluster list using device type
sl_status_t getClustersForDeviceType(const char* deviceType, char* clusterList, size_t clusterListSize);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// Check if the config is a device type
bool isDeviceType(const std::string& config);

#endif // __cplusplus

#endif // EED_MATTER_DEVICE_TYPE_H

