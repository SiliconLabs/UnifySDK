/***************************************************************************//**
 * @file
 * @brief  Sample API functions.
 *
 *  See also @ref board.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
/** @addtogroup board
 *
 * See host.h for source code.
 *@{
 */

#define halInternalInitBoard()                         \
  do {                                                 \
    /*halInternalInitLed(); not currently simulated */ \
  } while (0)

#define halInternalPowerDownBoard()                    \
  do {                                                 \
    /*halInternalInitLed(); not currently simulated */ \
  } while (0)

#define halInternalPowerUpBoard() \
  do {                            \
  } while (0)

enum HalBoardLedPins {
  BOARDLED0 = 0,
  BOARDLED1 = 1,
  BOARDLED2 = 2,
  BOARDLED3 = 3,
  BOARDLED4 = 4,
  BOARDLED5 = 5,
  BOARD_ACTIVITY_LED  = BOARDLED0,
  BOARD_HEARTBEAT_LED = BOARDLED1
};

#define BUTTON0         0
#define BUTTON1         1
#define TEMP_SENSOR_ADC_CHANNEL 0
#define TEMP_SENSOR_SCALE_FACTOR 3

/** @}  END addtogroup */
