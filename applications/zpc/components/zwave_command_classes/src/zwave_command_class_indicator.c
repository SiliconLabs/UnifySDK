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

#include "ctimer.h"

#include "assert.h"
#include "ZW_classcmd.h"
#include "zwave_command_class_indicator.h"
#include "zwave_command_handler.h"
#include "platform_exec.h"
#include "zwave_controller_command_class_indices.h"
#include "zwave_command_classes_utils.h"
#include "zwave_controller_connection_info.h"
#include "zwave_tx.h"
#include "sl_log.h"
#include "script_defines.h"

#define LOG_TAG "zwave_command_class_indicator"

static uint8_t g_on_off_period_length = 0;
static uint8_t g_on_off_num_cycles    = 0;
static uint8_t g_on_time              = 0;
static char *g_node_identify_script   = IDENTIFY_SCRIPT_PATH;
static struct ctimer g_indicator_timer;

/**
 * Call external script/application to control the indicator LED.
 *
 * \param on_time_ms  ON duration (in milliseconds) for a single blink cycle.
 *                    If on_time_ms is zero the indicator should be turned off.
 *
 * \param off_time_ms OFF duration (in milliseconds) for a single blink cycle.
 *
 * \param num_cycles  Number of blink cycles. If num_cycles is zero the indicator
 *                    LED should blink "forever" or until the next time this
 *                    function is called.
 */
static void call_external_blinker(unsigned int on_time_ms,
                                  unsigned int off_time_ms,
                                  unsigned int num_cycles)
{
  char *execve_args[5]     = {0};
  char *const env[]        = {"PATH=/bin", 0};
  char on_time_ms_arg[20]  = {0};
  char off_time_ms_arg[20] = {0};
  char num_cycles_arg[20]  = {0};

  snprintf(on_time_ms_arg, 20, "%u", on_time_ms);
  snprintf(off_time_ms_arg, 20, "%u", off_time_ms);
  snprintf(num_cycles_arg, 20, "%u", num_cycles);

  execve_args[0] = g_node_identify_script;
  execve_args[1] = on_time_ms_arg;
  execve_args[2] = off_time_ms_arg;
  execve_args[3] = num_cycles_arg;
  execve_args[4] = 0;

  // Our way to indicate here is in the logs
  // so set it to info level for a better visibility
  sl_log_info(LOG_TAG,
              "Starting Identify blinker: "
              "on_time: %dms - off_time: %dms - num_cycles: %d",
              on_time_ms,
              off_time_ms,
              num_cycles);

  platform_exec(g_node_identify_script, execve_args, env);
}

/**
 * Called on timeout of the timer controlling the length of the full blink sequence.
 *
 * Will reset all Node Identify properties to default values such
 * that the INDICATOR_GET command will reflect that the indicator
 * is no longer active.
 *
 * Please note: The timer is a "best effort" approximation to reflect the
 * status of the externally controlled LED. Since the external blinker is
 * running free after it has been started we do not know exactly if it has
 * blinked the requested number of times when this timeout occurs since the
 * external blink timers could have drifted a bit.
 *
 * We do not try to stop the LED from here - it's controlled by an external
 * script that we expect to terminate at the "right" time.
 * We only reset the internal state variables to reflect the the LED is
 * expected to no longer be active.
 *
 * The timer is not started if the client has requested an unlimited number
 * of blink cycles (\ref g_on_off_num_cycles == \c 0xFF). In that case there
 * will be no timeout.
 */
static void indicator_sequence_timout_callback(void *ptr)
{
  g_on_off_period_length = 0;
  g_on_off_num_cycles    = 0;
  g_on_time              = 0;
}

/**
 * Calls \ref call_external_blinker after calculating the parameters to pass.
 *
 * Uses the global variables \ref g_on_off_period_length, \ref g_on_off_num_cycles
 * and \ref g_on_time (set by Indicator Command Class commands) to calculate
 * the values to pass to \ref call_external_blinker. In addition it updates the
 * \ref g_indicator_timer timer.
 *
 * Should be called whenever a SET command is received.
 */
static void update_indicator(void)
{
  uint32_t on_time_ms        = 0;
  uint32_t off_time_ms       = 0;
  uint32_t total_duration_ms = 0;
  uint32_t blinker_cycles    = 0;

  /* Cancel timer (should work even if it's not running) */
  ctimer_stop(&g_indicator_timer);

  if (g_on_off_period_length > 0 && g_on_off_num_cycles > 0) {
    int on_off_period_length_ms = g_on_off_period_length * 100;

    /* The value 0x00 is special and means "symmetrical on/off blink" */
    if (g_on_time == 0) {
      on_time_ms = on_off_period_length_ms / 2;
    } else {
      on_time_ms = g_on_time * 100;
    }

    off_time_ms = on_off_period_length_ms - on_time_ms;

    /* The value 0xFF is special and means "run until stopped" */
    if (g_on_off_num_cycles != 0xFF) {
      total_duration_ms = g_on_off_period_length * g_on_off_num_cycles * 100;
      blinker_cycles    = g_on_off_num_cycles;
    }
  }

  /* Tell the external "blinker" to start (or stop) */
  call_external_blinker(on_time_ms, off_time_ms, blinker_cycles);

  if (total_duration_ms != 0) {
    /*
     * We would like to know when the blinker is done. Since there's no
     * "finished" callback from the blinker, we set a timer to match the
     * expected blink duration.
     */
    ctimer_set(&g_indicator_timer,
               total_duration_ms * CLOCK_SECOND / 1000,
               indicator_sequence_timout_callback,
               NULL);
  }
}

/**
 * Maps a single indicator value to the three version 3 indicator properties
 * stored in the global variables \ref g_on_off_period_length,
 * \ref g_on_off_num_cycles and \ref g_on_time.
 *
 * Used for backward compatibility with the version 1 command class.
 * The single indicator value is either the version 1 indicator value
 * or the version 3 Indicator0 value.
 *
 * \param value The value of the indicator. We're assuming Indicator0 to be a
 *        binary indicator (a more advanced extension could be to map to a
 *        multilevel indicator where the value controls e.g. the blink frequency).
 *        \b value is interpreted like this:
 *        - \b  0: Turn the indicator off
 *        - \b >0: Turn the indicator on
 */
static void set_indicator_value0(uint8_t value)
{
  if (value == 0) {
    g_on_off_period_length = 0;
    g_on_off_num_cycles    = 0;
    g_on_time              = 0;
  } else {
    g_on_off_period_length = 0xFF; /* Longest period possible */
    g_on_off_num_cycles
      = 0xFF;         /* Unlimited numer of periods (run until stopped) */
    g_on_time = 0xFF; /* On the full period */
  }

  update_indicator();
}

/**
 * Maps the three version 3 indicator properties stored in the global variables
 * \ref g_on_off_period_length, \ref g_on_off_num_cycles and \ref g_on_time to
 * a single indicator value.
 *
 * \see SetIndicator0Value
 *
 * \return Single value representing the indicator state.
 */
static uint8_t get_indicator_value0(void)
{
  /* We're not mapping the reported value 1:1 with the value (potentially)
   * set previously with SetIndicator0Value.
   * The indicator values could have been set with a V2 or V3 Set command
   * that provides more functionality than simply on and off.
   *
   * Here we report the indicator to be "ON" even if it's currently blinking
   * for a short time.
   */

  uint8_t value = 0;

  if (g_on_off_period_length > 0 && g_on_off_num_cycles > 0) {
    value = 0xFF;
  }

  return value;
}

static sl_status_t zwave_command_class_indicator_handle_set(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data)
{
  const ZW_INDICATOR_SET_FRAME *set_cmd
    = (const ZW_INDICATOR_SET_FRAME *)frame_data;

  if (set_cmd->value < 0x64
      || set_cmd->value
           == 0xFF)  //This field MUST be in the range 0x00..0x63 or 0xFF
  {
    set_indicator_value0(set_cmd->value);
    return SL_STATUS_OK;  //Value was accepted and applied, so we report Supervision SUCCESS.
  } else {
    return SL_STATUS_FAIL;  //Value was reserved/invalid, so Supervision returns FAIL.
  }
}

static bool zwave_command_class_indicator_get_object_count(
  int *obj_count, uint8_t cmd_properties, uint16_t frame_length)
{
  int calculated_frame_length = 0;

  const int frame_size_fixed_part
    = offsetof(ZW_INDICATOR_SET_1BYTE_V3_FRAME, variantgroup1);

  if (frame_length < frame_size_fixed_part)
    return false;

  *obj_count
    = cmd_properties & INDICATOR_SET_PROPERTIES1_INDICATOR_OBJECT_COUNT_MASK_V3;
  calculated_frame_length
    = frame_size_fixed_part + *obj_count * sizeof(VG_INDICATOR_SET_V3_VG);

  if (frame_length < calculated_frame_length)
    return false;

  return true;
}

static bool zwave_command_class_indicator_get_indicator_values(
  int obj_count,
  const VG_INDICATOR_SET_V3_VG *vg_array,
  uint8_t *on_off_period_length,
  uint8_t *on_off_num_cycles,
  uint8_t *on_time)
{
  bool indicator_object_error = false;
  for (int i = 0; i < obj_count; i++) {
    if (vg_array[i].indicatorId != INDICATOR_SET_NODE_IDENTIFY_V3) {
      indicator_object_error = true;
      continue;
    }

    const uint8_t value = vg_array[i].value;

    switch (vg_array[i].propertyId) {
      case INDICATOR_SET_ON_OFF_PERIOD_V3:
        *on_off_period_length = value;
        break;

      case INDICATOR_SET_ON_OFF_CYCLES_V3:
        *on_off_num_cycles = value;
        break;

      case INDICATOR_SET_ONE_TIME_ON_OFF_PERIOD_V3:
        *on_time = value;
        break;

      /* Ignore unknown properties, but remember that something went wrong for the Supervision Report */
      default:
        indicator_object_error = true;
        break;
    }
  }
  return !indicator_object_error;
}

static sl_status_t zwave_command_class_indicator_handle_setV3(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  const ZW_INDICATOR_SET_1BYTE_V3_FRAME *set_cmd
    = (const ZW_INDICATOR_SET_1BYTE_V3_FRAME *)frame_data;

  int obj_count = 0;
  if (!zwave_command_class_indicator_get_object_count(&obj_count,
                                                      set_cmd->properties1,
                                                      frame_length)) {
    return SL_STATUS_FAIL;
  }

  if (obj_count == 0) {
    /* Only process indicator0Value here - ignore if obj_count > 0 */
    return zwave_command_class_indicator_handle_set(connection_info,
                                                    frame_data);
  }

  uint8_t on_off_period_length = {0};
  uint8_t on_off_num_cycles    = {0};
  uint8_t on_time              = {0};
  bool result = zwave_command_class_indicator_get_indicator_values(
    obj_count,
    &set_cmd->variantgroup1,
    &on_off_period_length,
    &on_off_num_cycles,
    &on_time);

  /* Did we receive any of the minimum supported properties? */
  if (on_off_period_length != g_on_off_period_length
      && on_off_num_cycles != g_on_off_num_cycles) {
    g_on_off_period_length = on_off_period_length;
    g_on_off_num_cycles    = on_off_num_cycles;

    if (on_time > g_on_off_period_length) {
      /* Ignore (i.e. use default value) "on_time" if bigger than period length */
      g_on_time = 0;
      result    = false;
    } else {
      // If on_time was unspecified, it defaults to 0 anyway
      g_on_time = on_time;
    }

    update_indicator();
  }
  return result ? SL_STATUS_OK : SL_STATUS_FAIL;
}

static sl_status_t zwave_command_class_indicator_handle_get(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  ZW_INDICATOR_REPORT_FRAME report = {0};
  report.cmdClass                  = COMMAND_CLASS_INDICATOR;
  report.cmd                       = INDICATOR_REPORT;
  report.value                     = get_indicator_value0();
  return zwave_command_class_send_report(connection_info,
                                         sizeof(report),
                                         (uint8_t *)&report);
}

static sl_status_t zwave_command_class_indicator_handle_getV3(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < sizeof(ZW_INDICATOR_GET_V3_FRAME)) {
    sl_log_debug(LOG_TAG,
                 "incoming frame length: %d is less than size of"
                 "ZW_INDICATOR_GET_V3_FRAME frame: %d",
                 frame_length,
                 sizeof(ZW_INDICATOR_GET_V3_FRAME));
  }

  const ZW_INDICATOR_GET_V3_FRAME *get_cmd
    = (const ZW_INDICATOR_GET_V3_FRAME *)frame_data;

  uint8_t indicator_id = INDICATOR_GET_NODE_IDENTIFY_V3;

  if (frame_length == sizeof(ZW_INDICATOR_GET_V3_FRAME)) {
    indicator_id = get_cmd->indicatorId;
  }

  if (indicator_id == INDICATOR_GET_NODE_IDENTIFY_V3) {
    ZW_INDICATOR_REPORT_3BYTE_V3_FRAME report = {0};

    report.cmdClass        = COMMAND_CLASS_INDICATOR_V3;
    report.cmd             = INDICATOR_REPORT_V3;
    report.indicator0Value = get_indicator_value0();
    report.properties1     = 3; /* Number of property objects */

    report.variantgroup1.indicatorId = INDICATOR_GET_NODE_IDENTIFY_V3;
    report.variantgroup1.propertyId  = INDICATOR_REPORT_ON_OFF_PERIOD_V3;
    report.variantgroup1.value       = g_on_off_period_length;

    report.variantgroup2.indicatorId = INDICATOR_GET_NODE_IDENTIFY_V3;
    report.variantgroup2.propertyId  = INDICATOR_REPORT_ON_OFF_CYCLES_V3;
    report.variantgroup2.value       = g_on_off_num_cycles;

    report.variantgroup3.indicatorId = INDICATOR_GET_NODE_IDENTIFY_V3;
    report.variantgroup3.propertyId
      = INDICATOR_REPORT_ONE_TIME_ON_OFF_PERIOD_V3;
    report.variantgroup3.value = g_on_time;

    return zwave_command_class_send_report(connection_info,
                                           sizeof(report),
                                           (uint8_t *)&report);
  } else {
    /* Unsupported indicator ID. Send back an empty property object */

    ZW_INDICATOR_REPORT_1BYTE_V3_FRAME report = {0};

    report.cmdClass        = COMMAND_CLASS_INDICATOR_V3;
    report.cmd             = INDICATOR_REPORT_V3;
    report.indicator0Value = get_indicator_value0();
    report.properties1     = 1; /* Number of property objects */

    report.variantgroup1.indicatorId = INDICATOR_REPORT_NA_V3;
    report.variantgroup1.propertyId  = 0;
    report.variantgroup1.value       = 0;

    return zwave_command_class_send_report(connection_info,
                                           sizeof(report),
                                           (uint8_t *)&report);
  }
}

static sl_status_t zwave_command_class_indicator_handle_supported_getV3(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  /* Report the supported properties of an indicator */

  const ZW_INDICATOR_SUPPORTED_GET_V3_FRAME *sget_cmd
    = (const ZW_INDICATOR_SUPPORTED_GET_V3_FRAME *)frame_data;

  if (frame_length < sizeof(ZW_INDICATOR_SUPPORTED_GET_V3_FRAME)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if ((sget_cmd->indicatorId == INDICATOR_SUPPORTED_GET_NODE_IDENTIFY_V3)
      || (sget_cmd->indicatorId == INDICATOR_SUPPORTED_GET_NA_V3)) {
    /*
     * If indicator ID == INDICATOR_IND_NA (0x00) the client has requested to
     * discover supported indicator IDs. Since we only support the node identify
     * indicator we advertise this in the exact same manner as if the node
     * identify indicator was requested explicitly.
     */

    static const ZW_INDICATOR_SUPPORTED_REPORT_1BYTE_V3_FRAME report
      = {.cmdClass        = COMMAND_CLASS_INDICATOR_V3,
         .cmd             = INDICATOR_SUPPORTED_REPORT_V3,
         .indicatorId     = INDICATOR_SUPPORTED_GET_NODE_IDENTIFY_V3,
         .nextIndicatorId = INDICATOR_SUPPORTED_GET_NA_V3,
         .properties1     = 1,
         .propertySupportedBitMask1
         = (1 << INDICATOR_REPORT_ON_OFF_PERIOD_V3)
           | (1 << INDICATOR_REPORT_ON_OFF_CYCLES_V3)
           | (1 << INDICATOR_REPORT_ONE_TIME_ON_OFF_PERIOD_V3)};

    return zwave_command_class_send_report(connection_info,
                                           sizeof(report),
                                           (const uint8_t *)&report);

  } else {
    /* The indicator is not supported, send back an empty report. */

    /*
     * All the "Supported Report" structs have one or more bitmasks.
     * Here we must send one without the bitmask, so we use the struct
     * with one bitmask, but tell ZW_SendDataZIP to only send the
     * "fixed header" part (leaving out the bitmask).
     */

    ZW_INDICATOR_SUPPORTED_REPORT_1BYTE_V3_FRAME report
      = {.cmdClass                  = COMMAND_CLASS_INDICATOR_V3,
         .cmd                       = INDICATOR_SUPPORTED_REPORT_V3,
         .indicatorId               = INDICATOR_SUPPORTED_GET_NA_V3,
         .nextIndicatorId           = INDICATOR_SUPPORTED_GET_NA_V3,
         .properties1               = 0,
         .propertySupportedBitMask1 = 0};

    /* Leave out the bitmask when sending */
    size_t send_size = offsetof(ZW_INDICATOR_SUPPORTED_REPORT_1BYTE_V3_FRAME,
                                propertySupportedBitMask1);

    return zwave_command_class_send_report(connection_info,
                                           send_size,
                                           (uint8_t *)&report);
  }
}

sl_status_t zwave_command_class_indicator_support_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case INDICATOR_SET_V3:
      if (frame_length == sizeof(ZW_INDICATOR_SET_FRAME)) {
        return zwave_command_class_indicator_handle_set(connection_info,
                                                        frame_data);
      }
      return zwave_command_class_indicator_handle_setV3(connection_info,
                                                        frame_data,
                                                        frame_length);

    case INDICATOR_GET_V3:
      if (connection_info->local.is_multicast) {
        return SL_STATUS_OK;
      }

      if (frame_length == sizeof(ZW_INDICATOR_GET_FRAME))
        return zwave_command_class_indicator_handle_get(connection_info,
                                                        frame_data,
                                                        frame_length);
      return zwave_command_class_indicator_handle_getV3(connection_info,
                                                        frame_data,
                                                        frame_length);

    case INDICATOR_SUPPORTED_GET_V3:
      if (connection_info->local.is_multicast) {
        return SL_STATUS_OK;
      }

      return zwave_command_class_indicator_handle_supported_getV3(
        connection_info,
        frame_data,
        frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_indicator_init()
{
  zwave_command_handler_t handler = {0};

  handler.support_handler    = zwave_command_class_indicator_support_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_INDICATOR;
  handler.version            = INDICATOR_VERSION_V3;
  handler.command_class_name = "Indicator";
  handler.manual_security_validation = false;

  sl_log_debug(LOG_TAG,
               "loaded indicator script path:%s",
               g_node_identify_script);
  call_external_blinker(0, 0, 0);

  return zwave_command_handler_register_handler(handler);
}
