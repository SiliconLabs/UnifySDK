
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
 * @defgroup defined_notifications List of defined Notifications Types and Event/States
 * @ingroup notification_command_class
 * @brief List of Notifications for the Notification Command Class
 *
 * This group contains the definition of the assigned Notifications event and types
 * defined in SDS13713.
 *
 * @{
 */

#ifndef NOTIFICATION_COMMAND_CLASS_DEFINED_NOTIFICATIONS
#define NOTIFICATION_COMMAND_CLASS_DEFINED_NOTIFICATIONS

///< State Idle, applies for all Notification types.
#define NOTIFICATION_STATE_IDLE 0x00
///< Unknown state/event, applies for all Notification types.
#define NOTIFICATION_UNKNOWN_STATE_EVENT 0xFE

///< Home Security Notification Type
#define NOTIFICATION_TYPE_HOME_SECURITY 0x07

///< Intrusion state (with location provided)
#define NOTIFICATION_HOME_SECURITY_STATE_INTRUSION_WITH_LOCATION 0x01
///< Intrusion state
#define NOTIFICATION_HOME_SECURITY_STATE_INTRUSION 0x02

///< Product cover removed state
#define NOTIFICATION_HOME_SECURITY_TAMPERING 0x03

///< Motion detected state (with location provided)
#define NOTIFICATION_HOME_SECURITY_STATE_MOTION_DETECTION_WITH_LOCATION 0x07
///< Motion detected state
#define NOTIFICATION_HOME_SECURITY_STATE_MOTION_DETECTION 0x08

///< Magnetic Interference detected
#define NOTIFICATION_HOME_SECURITY_STATE_MAGNETIC_INTERFERENCE 0x0B

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //NOTIFICATION_COMMAND_CLASS_DEFINED_NOTIFICATIONS
/** @} end dotdot_mapper */
