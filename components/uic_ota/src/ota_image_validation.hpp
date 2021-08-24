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
 * @file ota_image_validation.hpp
 * @brief OTA image validation helper functions.
 *
 * The OTA image helper functions helps validating the image binary files 
 * transferred over MQTT from image providers and provide them to 
 * protocol controllers.
 *
 * @{
 */

#ifndef OTA_IMAGE_VALIDATION_HPP
#define OTA_IMAGE_VALIDATION_HPP

// UIC
#include "sl_status.h"

// Standard library
#include <cstddef>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Checking the size of an image is not too big.
 * 
 * @param message_length Length of mqtt message
 * @param image_cache_size Size of image cache
 * 
 * @returns status of file size check.
 */
sl_status_t is_image_valid_size(const size_t message_length,
                                const unsigned long image_cache_size);

#ifdef __cplusplus
}
#endif

#endif  //OTA_IMAGE_VALIDATION_HPP
/** @} end ota_image_validation */
