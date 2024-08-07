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
 * @defgroup zwave_rf_region Z-Wave RF Regions definitions
 * @ingroup zwave_definitions
 * @brief RF region definitions
 *
 * @{
 */

#if !defined(ZWAVE_RF_REGION_H)
#define ZWAVE_RF_REGION_H

#include <stdbool.h>

/// @name zwapi_rf_region_t
/// These definitions represent possible rf settings for the Z-Wave module
/// RF region values used with zwapi_set_rf_region() and zwapi_get_rf_region()
///@{
typedef enum {
  ///< Radio is located in Europe. 2 Channel region.
  ZWAVE_RF_REGION_EU = 0x00,
  ///< Radio is located in the US. 2 Channel region.
  ZWAVE_RF_REGION_US = 0x01,
  ///< Radio is located in Australia/New Zealand. 2 Channel region.
  ZWAVE_RF_REGION_ANZ = 0x02,
  ///< Radio is located in Hong Kong. 2 Channel region.
  ZWAVE_RF_REGION_HK = 0x03,
  ///< Radio is located in India. 2 Channel region.
  ZWAVE_RF_REGION_IN = 0x05,
  ///< Radio is located in Israel. 2 Channel region.
  ZWAVE_RF_REGION_IL = 0x06,
  ///< Radio is located in Russia. 2 Channel region.
  ZWAVE_RF_REGION_RU = 0x07,
  ///< Radio is located in China. 2 Channel region.
  ZWAVE_RF_REGION_CN = 0x08,
  ///< US Long range radio
  ZWAVE_RF_REGION_US_LR = 0x09,
  ///< EU Long range radio
  ZWAVE_RF_REGION_EU_LR = 0x0B,
  ///< Radio is located in Japan. 3 Channel region.
  ZWAVE_RF_REGION_JP = 0x20,
  ///< Radio is located in Korea. 3 Channel region.
  ZWAVE_RF_REGION_KR = 0x21,
  ///<  RF region is unknown
  ZWAVE_RF_REGION_UNDEFINED = 0xFE
} zwave_rf_region_t;
///@}

static inline bool IS_RF_REGION_LR(zwave_rf_region_t rf_region) { return (rf_region == ZWAVE_RF_REGION_US_LR) || (rf_region == ZWAVE_RF_REGION_EU_LR); }

#endif  // ZWAVE_RF_REGION_H
/** @} end zwave_rf_region */
