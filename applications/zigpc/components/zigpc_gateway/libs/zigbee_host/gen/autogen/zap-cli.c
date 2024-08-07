/***************************************************************************//**
 * @file  zap-cli.c
 * @brief Generated file for zcl cli using ZAP. Do not update file manually.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_ZCL_CLI_PRESENT

#include <stdlib.h>
#include "sl_cli_config.h"
#include "sl_cli_command.h"
#include "sl_cli.h"
#include "zcl-cli.h"
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_debug_print.h"
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "zap-type.h"
#include "zap-id.h"
#include "app/framework/include/af.h"
#include "zap-config.h"

extern void sli_zigbee_zcl_simple_command(uint8_t frameControl,
                        uint16_t clusterId,
                        uint8_t commandId,
                        sl_cli_command_arg_t *arguments,
                        uint8_t *argumentTypes);
extern sl_cli_command_info_t cli_cmd_zcl_global_group;
extern sl_cli_command_info_t cli_cmd_zcl_mfg_code_command;
extern sl_cli_command_info_t cli_cmd_zcl_time_command;
extern sl_cli_command_info_t cli_cmd_zcl_use_next_sequence_command;
extern sl_cli_command_info_t cli_cmd_zcl_x_default_resp_command;
extern sl_cli_command_info_t cli_cmd_zcl_test_response_on_command;
extern sl_cli_command_info_t cli_cmd_zcl_test_response_off_command;
#if defined(ZCL_USING_IDENTIFY_CLUSTER_SERVER) || defined(ZCL_USING_IDENTIFY_CLUSTER_CLIENT)
extern sl_cli_command_info_t cli_cmd_zcl_identify_on_command;
extern sl_cli_command_info_t cli_cmd_zcl_identify_off_command;
#endif
#if defined(ZCL_USING_BACNET_PROTOCOL_TUNNEL_CLUSTER_CLIENT) || defined(ZCL_USING_BACNET_PROTOCOL_TUNNEL_CLUSTER_SERVER)
extern sl_cli_command_info_t cli_cmd_zcl_bacnet_transfer_whois_command;
#endif
#if defined(ZCL_USING_TUNNELING_CLUSTER_CLIENT) || defined(ZCL_USING_TUNNELING_CLUSTER_SERVER)
extern sl_cli_command_info_t cli_cmd_zcl_tunneling_random_to_server_command;
extern sl_cli_command_info_t cli_cmd_zcl_tunneling_transfer_to_server_command;
extern sl_cli_command_info_t cli_cmd_zcl_tunneling_random_to_client_command;
extern sl_cli_command_info_t cli_cmd_zcl_tunneling_transfer_to_client_command;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Provide function declarations
void sli_zigbee_cli_zcl_groups_add_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_groups_add_if_id_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_chg_supply_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_clear_all_rfids_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_clear_holiday_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_clear_pin_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_clear_rfid_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_clear_weekday_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_thermostat_cws_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_clear_yearday_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_loop_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_cfg_mirror_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_cfg_nft_flags_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_cfg_nft_scheme_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_identify_ez_mode_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_power_profile_energy_phases_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_power_profile_energy_phases_schedule_states_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_emovehue_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_emovetohue_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_emovetohueandsat_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_estephue_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_poll_control_stop_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_groups_get_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_get_holiday_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_get_log_record_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_get_ntfy_msg_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_get_pin_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_get_profile_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_thermostat_grs_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_get_rfid_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_get_sampled_data_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_get_snapshot_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_get_user_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_get_weekday_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_thermostat_gws_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_get_yearday_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_identify_id_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_identify_query_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_local_chg_supply_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_lock_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_mirror_removed_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_level_control_move_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_movecolor_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_movecolortemp_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_movehue_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_movesat_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_movetocolor_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_movetocolortemp_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_movetohue_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_movetohueandsat_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_level_control_mv_to_level_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_level_control_o_mv_to_level_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_movetosat_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_level_control_o_move_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_on_off_off_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_on_off_offeffect_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_on_off_on_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_on_off_onrecall_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_on_off_ontimedoff_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_power_profile_profile_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_power_profile_schedule_constraints_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_power_profile_state_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_pub_ss_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_groups_rmall_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_groups_remove_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_remove_mirror_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_req_fast_poll_mode_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_request_mirror_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_rst_load_limit_ctr_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_basic_rtfd_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_sch_snapshot_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_sch_snapshot_resp_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_set_holiday_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_poll_control_long_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_set_pin_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_set_rfid_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_poll_control_short_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_set_supply_status_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_set_uncntrl_flow_threshold_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_set_user_type_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_set_weekday_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_thermostat_sws_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_set_yearday_schedule_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_thermostat_set_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_start_sampling_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_start_samp_rsp_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_level_control_step_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_stepcolor_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_stepcolortemp_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_stephue_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_stepsat_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_level_control_o_step_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_level_control_stop_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_color_control_stopmovestep_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_level_control_o_stop_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_sup_stat_rsp_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_take_snapshot_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_simple_metering_take_snapshot_resp_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_toggle_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_on_off_toggle_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_identify_trigger_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_unlock_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_door_lock_unlock_with_timeout_command(sl_cli_command_arg_t *arguments);
void sli_zigbee_cli_zcl_groups_view_command(sl_cli_command_arg_t *arguments);

// Command structs. Names are command names prefixed by cli_cmd_zcl_[cluster name]_cluster
static const sl_cli_command_info_t cli_cmd_zcl_groups_cluster_add_group = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_groups_add_command,
            "Command description for AddGroup",
            "group id" SL_CLI_UNIT_SEPARATOR "group name" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_STRING,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_groups_cluster_add_group_if_identifying = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_groups_add_if_id_command,
            "Command description for AddGroupIfIdentifying",
            "group id" SL_CLI_UNIT_SEPARATOR "group name" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_STRING,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_change_supply = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_chg_supply_command,
            "This command is sent from the Head-end or ESI to the Metering Device to instruct it to change the status of the valve or load switch, i.e. the supply.",
            "provider id" SL_CLI_UNIT_SEPARATOR "issuer event id" SL_CLI_UNIT_SEPARATOR "request date time" SL_CLI_UNIT_SEPARATOR "implementation date time" SL_CLI_UNIT_SEPARATOR "proposed supply status" SL_CLI_UNIT_SEPARATOR "supply control bits" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_clear_all_rfids = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_clear_all_rfids_command,
            "Clear all RFID ID codes on the lock for all users.",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_clear_holiday_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_clear_holiday_schedule_command,
            "Clear a holiday schedule for a specific scheduleId",
            "schedule id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_clear_pin = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_clear_pin_command,
            "Clear the PIN for a user with a specific user ID",
            "user id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_clear_rfid = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_clear_rfid_command,
            "Clear the RFID ID for a user with a specific user ID",
            "user id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_clear_weekday_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_clear_weekday_schedule_command,
            "Clear a weekday schedule for doorlock user activation for a specific schedule id and user id.",
            "schedule id" SL_CLI_UNIT_SEPARATOR "user id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_thermostat_cluster_clear_weekly_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_thermostat_cws_command,
            "The Clear Weekly Schedule command is used to clear the weekly schedule.",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_clear_yearday_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_clear_yearday_schedule_command,
            "Clear a yearday schedule for a specific scheduleId and userId",
            "schedule id" SL_CLI_UNIT_SEPARATOR "user id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_color_loop_set = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_loop_command,
            "Command description for ColorLoopSet",
            "update flags" SL_CLI_UNIT_SEPARATOR "action" SL_CLI_UNIT_SEPARATOR "direction" SL_CLI_UNIT_SEPARATOR "time" SL_CLI_UNIT_SEPARATOR "start hue" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_configure_mirror = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_cfg_mirror_command,
            "ConfigureMirror is sent to the mirror once the mirror has been created. The command deals with the operational configuration of the Mirror.",
            "issuer event id" SL_CLI_UNIT_SEPARATOR "reporting interval" SL_CLI_UNIT_SEPARATOR "mirror notification reporting" SL_CLI_UNIT_SEPARATOR "notification scheme" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_configure_notification_flags = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_cfg_nft_flags_command,
            "The ConfigureNotificationFlags command is used to set the commands relating to the bit value for each NotificationFlags attribute that the scheme is proposing to use.",
            "issuer event id" SL_CLI_UNIT_SEPARATOR "notification scheme" SL_CLI_UNIT_SEPARATOR "notification flag attribute id" SL_CLI_UNIT_SEPARATOR "cluster id" SL_CLI_UNIT_SEPARATOR "manufacturer code" SL_CLI_UNIT_SEPARATOR "number of commands" SL_CLI_UNIT_SEPARATOR "command ids" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8OPT,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_configure_notification_scheme = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_cfg_nft_scheme_command,
            "The ConfigureNotificationScheme is sent to the mirror once the mirror has been created. The command deals with the operational configuration of the Mirror and the device that reports to the mirror. No default schemes are allowed to be overwritten.",
            "issuer event id" SL_CLI_UNIT_SEPARATOR "notification scheme" SL_CLI_UNIT_SEPARATOR "notification flag order" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_identify_cluster_ez_mode_invoke = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_identify_ez_mode_command,
            "Invoke EZMode on an Identify Server",
            "action" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_power_profile_cluster_energy_phases_schedule_notification = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_power_profile_energy_phases_schedule_command,
            "The EnergyPhasesScheduleNotification Command is generated by a device supporting the client side of the Power Profile cluster in order to schedule the start of the selected Power Profile and its phases.",
            "power profile id" SL_CLI_UNIT_SEPARATOR "num of scheduled phases" SL_CLI_UNIT_SEPARATOR "scheduled phases" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_HEXOPT,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_power_profile_cluster_energy_phases_schedule_state_request = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_power_profile_energy_phases_schedule_states_command,
            "The EnergyPhasesScheduleStateRequest  Command is generated by a device supporting the client side of the Power Profile cluster to check the states of the scheduling of a power profile, which is supported in the device implementing the server side of Power Profile cluster.",
            "power profile id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_enhanced_move_hue = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_emovehue_command,
            "Command description for EnhancedMoveHue",
            "move mode" SL_CLI_UNIT_SEPARATOR "rate" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_enhanced_move_to_hue = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_emovetohue_command,
            "Command description for EnhancedMoveToHue",
            "enhanced hue" SL_CLI_UNIT_SEPARATOR "direction" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_emovetohueandsat_command,
            "Command description for EnhancedMoveToHueAndSaturation",
            "enhanced hue" SL_CLI_UNIT_SEPARATOR "saturation" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_enhanced_step_hue = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_estephue_command,
            "Command description for EnhancedStepHue",
            "step mode" SL_CLI_UNIT_SEPARATOR "step size" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_poll_control_cluster_fast_poll_stop = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_poll_control_stop_command,
            "The Fast Poll Stop command is used to stop the fast poll mode initiated by the Check-in response.",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_groups_cluster_get_group_membership = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_groups_get_command,
            "Command description for GetGroupMembership",
            "group count" SL_CLI_UNIT_SEPARATOR "group list" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16OPT,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_get_holiday_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_get_holiday_schedule_command,
            "Retrieve a holiday schedule for a specific scheduleId",
            "schedule id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_get_log_record = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_get_log_record_command,
            "Retrieve a log record at a specified index.",
            "log index" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_get_notified_message = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_get_ntfy_msg_command,
            "The GetNotifiedMessage command is used only when a BOMD is being mirrored. This command provides a method for the BOMD to notify the Mirror message queue that it wants to receive commands that the Mirror has queued. The Notification flags set within the command shall inform the mirror of the commands that the BOMD is requesting.",
            "notification scheme" SL_CLI_UNIT_SEPARATOR "notification flag attribute id" SL_CLI_UNIT_SEPARATOR "notification flagsn" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_get_pin = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_get_pin_command,
            "Retrieve PIN information for a user with a specific user ID.",
            "user id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_get_profile = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_get_profile_command,
            "The GetProfile command is generated when a client device wishes to retrieve a list of captured Energy, Gas or water consumption for profiling purposes.",
            "interval channel" SL_CLI_UNIT_SEPARATOR "end time" SL_CLI_UNIT_SEPARATOR "number of periods" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_thermostat_cluster_get_relay_status_log = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_thermostat_grs_command,
            "The Get Relay Status Log command is used to query the thermostat internal relay status log.",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_get_rfid = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_get_rfid_command,
            "Retrieve RFID ID information for a user with a specific user ID.",
            "user id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_get_sampled_data = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_get_sampled_data_command,
            "This command is used to request sampled data from the server. Note that it is the responsibility of the client to ensure that it does not request more samples than can be held in a single command payload.",
            "sample id" SL_CLI_UNIT_SEPARATOR "earliest sample time" SL_CLI_UNIT_SEPARATOR "sample type" SL_CLI_UNIT_SEPARATOR "number of samples" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_get_snapshot = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_get_snapshot_command,
            "This command is used to request snapshot data from the cluster server.",
            "earliest start time" SL_CLI_UNIT_SEPARATOR "latest end time" SL_CLI_UNIT_SEPARATOR "snapshot offset" SL_CLI_UNIT_SEPARATOR "snapshot cause" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_get_user_type = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_get_user_type_command,
            "Retrieve the type for a specific user based on the user ID.",
            "user id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_get_weekday_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_get_weekday_schedule_command,
            "Retrieve a weekday schedule for doorlock user activation for a specific schedule id and user id.",
            "schedule id" SL_CLI_UNIT_SEPARATOR "user id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_thermostat_cluster_get_weekly_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_thermostat_gws_command,
            "Command description for GetWeeklySchedule",
            "days to return" SL_CLI_UNIT_SEPARATOR "mode to return" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_get_yearday_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_get_yearday_schedule_command,
            "Retrieve a yearday schedule for a specific scheduleId and userId",
            "schedule id" SL_CLI_UNIT_SEPARATOR "user id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_identify_cluster_identify = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_identify_id_command,
            "Command description for Identify",
            "identify time" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_identify_cluster_identify_query = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_identify_query_command,
            "Command description for IdentifyQuery",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_local_change_supply = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_local_chg_supply_command,
            "This command is a simplified version of the ChangeSupply command, intended to be sent from an IHD to a meter as the consequence of a user action on the IHD. Its purpose is to provide a local disconnection/reconnection button on the IHD in addition to the one on the meter.",
            "proposed supply status" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_lock_door = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_lock_command,
            "Locks the door",
            "pin" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_STRING,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_mirror_removed = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_mirror_removed_command,
            "The Mirror Removed Command allows the ESI to inform a sleepy Metering Device mirroring support has been removed or halted.",
            "endpoint id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_level_control_cluster_move = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_level_control_move_command,
            "Command description for Move",
            "move mode" SL_CLI_UNIT_SEPARATOR "rate" SL_CLI_UNIT_SEPARATOR "option mask" SL_CLI_UNIT_SEPARATOR "option override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_move_color = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_movecolor_command,
            "Moves the color.",
            "ratex" SL_CLI_UNIT_SEPARATOR "ratey" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_INT16,
                SL_CLI_ARG_INT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_move_color_temperature = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_movecolortemp_command,
            "Command description for MoveColorTemperature",
            "move mode" SL_CLI_UNIT_SEPARATOR "rate" SL_CLI_UNIT_SEPARATOR "color temperature minimum" SL_CLI_UNIT_SEPARATOR "color temperature maximum" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_move_hue = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_movehue_command,
            "Move hue up or down at specified rate.",
            "move mode" SL_CLI_UNIT_SEPARATOR "rate" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_move_saturation = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_movesat_command,
            "Move saturation up or down at specified rate.",
            "move mode" SL_CLI_UNIT_SEPARATOR "rate" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_move_to_color = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_movetocolor_command,
            "Move to specified color.",
            "colorx" SL_CLI_UNIT_SEPARATOR "colory" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_move_to_color_temperature = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_movetocolortemp_command,
            "Move to a specific color temperature.",
            "color temperature" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_move_to_hue = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_movetohue_command,
            "Move to specified hue.",
            "hue" SL_CLI_UNIT_SEPARATOR "direction" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_move_to_hue_and_saturation = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_movetohueandsat_command,
            "Move to hue and saturation.",
            "hue" SL_CLI_UNIT_SEPARATOR "saturation" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_level_control_cluster_move_to_level = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_level_control_mv_to_level_command,
            "Command description for MoveToLevel",
            "level" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "option mask" SL_CLI_UNIT_SEPARATOR "option override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_level_control_cluster_move_to_level_with_on_off = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_level_control_o_mv_to_level_command,
            "Command description for MoveToLevelWithOnOff",
            "level" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_move_to_saturation = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_movetosat_command,
            "Move to specified saturation.",
            "saturation" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_level_control_cluster_move_with_on_off = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_level_control_o_move_command,
            "Command description for MoveWithOnOff",
            "move mode" SL_CLI_UNIT_SEPARATOR "rate" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_on_off_cluster_off = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_on_off_off_command,
            "Command description for Off",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_on_off_cluster_off_with_effect = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_on_off_offeffect_command,
            "Command description for OffWithEffect",
            "effect id" SL_CLI_UNIT_SEPARATOR "effect variant" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_on_off_cluster_on = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_on_off_on_command,
            "Command description for On",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_on_off_cluster_on_with_recall_global_scene = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_on_off_onrecall_command,
            "Command description for OnWithRecallGlobalScene",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_on_off_cluster_on_with_timed_off = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_on_off_ontimedoff_command,
            "Command description for OnWithTimedOff",
            "on off control" SL_CLI_UNIT_SEPARATOR "on time" SL_CLI_UNIT_SEPARATOR "off wait time" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_power_profile_cluster_power_profile_request = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_power_profile_profile_command,
            "The PowerProfileRequest Command is generated by a device supporting the client side of the Power Profile cluster in order to request the Power Profile of a server device.",
            "power profile id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_power_profile_cluster_power_profile_schedule_constraints_request = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_power_profile_schedule_constraints_command,
            "The PowerProfileScheduleConstraintsRequest Command is generated by a device supporting the client side of the Power Profile cluster in order to request the constraints -if set- of Power Profile of a client device, in order to set the proper boundaries for the scheduling when calculating the schedules.",
            "power profile id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_power_profile_cluster_power_profile_state_request = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_power_profile_state_command,
            "The PowerProfileStateRequest Command is generated in order to retrieve the identifiers of current Power Profiles.",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_publish_snapshot = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_pub_ss_command,
            "This command is generated in response to a GetSnapshot command. It is used to return a single snapshot to the client.",
            "snapshot id" SL_CLI_UNIT_SEPARATOR "snapshot time" SL_CLI_UNIT_SEPARATOR "total snapshots found" SL_CLI_UNIT_SEPARATOR "command index" SL_CLI_UNIT_SEPARATOR "total commands" SL_CLI_UNIT_SEPARATOR "snapshot cause" SL_CLI_UNIT_SEPARATOR "snapshot payload type" SL_CLI_UNIT_SEPARATOR "snapshot payload" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8OPT,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_groups_cluster_remove_all_groups = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_groups_rmall_command,
            "Command description for RemoveAllGroups",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_groups_cluster_remove_group = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_groups_remove_command,
            "Command description for RemoveGroup",
            "group id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_remove_mirror = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_remove_mirror_command,
            "This command is used to request the ESI to remove its mirror of Metering Device data.",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_request_fast_poll_mode = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_req_fast_poll_mode_command,
            "The Request Fast Poll Mode command is generated when the metering client wishes to receive near real-time updates of InstantaneousDemand.",
            "fast poll update period" SL_CLI_UNIT_SEPARATOR "duration" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_request_mirror = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_request_mirror_command,
            "This command is used to request the ESI to mirror Metering Device data.",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_reset_load_limit_counter = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_rst_load_limit_ctr_command,
            "The ResetLoadLimitCounter command shall cause the LoadLimitCounter attribute to be reset.",
            "provider id" SL_CLI_UNIT_SEPARATOR "issuer event id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_basic_cluster_reset_to_factory_defaults = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_basic_rtfd_command,
            "Command that resets all attribute values to factory default.",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_schedule_snapshot = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_sch_snapshot_command,
            "This command is used to set up a schedule of when the device shall create snapshot data.",
            "issuer event id" SL_CLI_UNIT_SEPARATOR "command index" SL_CLI_UNIT_SEPARATOR "command count" SL_CLI_UNIT_SEPARATOR "snapshot schedule payload" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_HEXOPT,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_schedule_snapshot_response = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_sch_snapshot_resp_command,
            "This command is generated in response to a ScheduleSnapshot command, and is sent to confirm whether the requested snapshot schedule has been set up.",
            "issuer event id" SL_CLI_UNIT_SEPARATOR "snapshot response payload" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_HEXOPT,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_set_holiday_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_set_holiday_schedule_command,
            "Set the holiday schedule for a specific user",
            "schedule id" SL_CLI_UNIT_SEPARATOR "local start time" SL_CLI_UNIT_SEPARATOR "local end time" SL_CLI_UNIT_SEPARATOR "operating mode during holiday" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_poll_control_cluster_set_long_poll_interval = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_poll_control_long_command,
            "The Set Long Poll Interval command is used to set the read only Long Poll Interval Attribute.",
            "new long poll interval" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_set_pin = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_set_pin_command,
            "Set the PIN for a specified user id.",
            "user id" SL_CLI_UNIT_SEPARATOR "user status" SL_CLI_UNIT_SEPARATOR "user type" SL_CLI_UNIT_SEPARATOR "pin" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_STRING,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_set_rfid = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_set_rfid_command,
            "Set the PIN for a specified user id.",
            "user id" SL_CLI_UNIT_SEPARATOR "user status" SL_CLI_UNIT_SEPARATOR "user type" SL_CLI_UNIT_SEPARATOR "id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_STRING,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_poll_control_cluster_set_short_poll_interval = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_poll_control_short_command,
            "The Set Short Poll Interval command is used to set the read only Short Poll Interval Attribute.",
            "new short poll interval" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_set_supply_status = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_set_supply_status_command,
            "This command is used to specify the required status of the supply following the occurance of certain events on the meter.",
            "issuer event id" SL_CLI_UNIT_SEPARATOR "supply tamper state" SL_CLI_UNIT_SEPARATOR "supply depletion state" SL_CLI_UNIT_SEPARATOR "supply uncontrolled flow state" SL_CLI_UNIT_SEPARATOR "load limit supply state" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_set_uncntrl_flow_threshold_command,
            "This command is used to update the 'Uncontrolled Flow Rate' configuration data used by flow meters.",
            "provider id" SL_CLI_UNIT_SEPARATOR "issuer event id" SL_CLI_UNIT_SEPARATOR "uncontrolled flow threshold" SL_CLI_UNIT_SEPARATOR "unit of measure" SL_CLI_UNIT_SEPARATOR "multiplier" SL_CLI_UNIT_SEPARATOR "divisor" SL_CLI_UNIT_SEPARATOR "stabilisation period" SL_CLI_UNIT_SEPARATOR "measurement period" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_set_user_type = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_set_user_type_command,
            "Set the type value for a user based on user ID.",
            "user id" SL_CLI_UNIT_SEPARATOR "user type" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_set_weekday_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_set_weekday_schedule_command,
            "Set the schedule of days during the week that the associated user based on the user ID will have access to the lock and will be able to operate it.",
            "schedule id" SL_CLI_UNIT_SEPARATOR "user id" SL_CLI_UNIT_SEPARATOR "days mask" SL_CLI_UNIT_SEPARATOR "start hour" SL_CLI_UNIT_SEPARATOR "start minute" SL_CLI_UNIT_SEPARATOR "end hour" SL_CLI_UNIT_SEPARATOR "end minute" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_thermostat_cluster_set_weekly_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_thermostat_sws_command,
            "Command description for SetWeeklySchedule",
            "number of transitions for sequence" SL_CLI_UNIT_SEPARATOR "day of week for sequence" SL_CLI_UNIT_SEPARATOR "mode for sequence" SL_CLI_UNIT_SEPARATOR "payload" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8OPT,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_set_yearday_schedule = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_set_yearday_schedule_command,
            "Set a door lock user id activation schedule according to a specific absolute local start and end time",
            "schedule id" SL_CLI_UNIT_SEPARATOR "user id" SL_CLI_UNIT_SEPARATOR "local start time" SL_CLI_UNIT_SEPARATOR "local end time" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_thermostat_cluster_setpoint_raise_lower = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_thermostat_set_command,
            "Command description for SetpointRaiseLower",
            "mode" SL_CLI_UNIT_SEPARATOR "amount" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_INT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_start_sampling = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_start_sampling_command,
            "The sampling mechanism allows a set of samples of the specified type of data to be taken, commencing at the stipulated start time. This mechanism may run concurrently with the capturing of profile data, and may refer the same parameters, albeit possibly at a different sampling rate.",
            "issuer event id" SL_CLI_UNIT_SEPARATOR "start sampling time" SL_CLI_UNIT_SEPARATOR "sample type" SL_CLI_UNIT_SEPARATOR "sample request interval" SL_CLI_UNIT_SEPARATOR "max number of samples" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_start_sampling_response = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_start_samp_rsp_command,
            "This command is transmitted by a Metering Device in response to a StartSampling command.",
            "sample id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_level_control_cluster_step = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_level_control_step_command,
            "Command description for Step",
            "step mode" SL_CLI_UNIT_SEPARATOR "step size" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "option mask" SL_CLI_UNIT_SEPARATOR "option override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_step_color = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_stepcolor_command,
            "Steps the lighting to a specific color.",
            "stepx" SL_CLI_UNIT_SEPARATOR "stepy" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_INT16,
                SL_CLI_ARG_INT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_step_color_temperature = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_stepcolortemp_command,
            "Command description for StepColorTemperature",
            "step mode" SL_CLI_UNIT_SEPARATOR "step size" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "color temperature minimum" SL_CLI_UNIT_SEPARATOR "color temperature maximum" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_step_hue = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_stephue_command,
            "Step hue up or down by specified size at specified rate.",
            "step mode" SL_CLI_UNIT_SEPARATOR "step size" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_step_saturation = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_stepsat_command,
            "Step saturation up or down by specified size at specified rate.",
            "step mode" SL_CLI_UNIT_SEPARATOR "step size" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_level_control_cluster_step_with_on_off = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_level_control_o_step_command,
            "Command description for StepWithOnOff",
            "step mode" SL_CLI_UNIT_SEPARATOR "step size" SL_CLI_UNIT_SEPARATOR "transition time" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_level_control_cluster_stop = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_level_control_stop_command,
            "Command description for Stop",
            "option mask" SL_CLI_UNIT_SEPARATOR "option override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_color_control_cluster_stop_move_step = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_color_control_stopmovestep_command,
            "Command description for StopMoveStep",
            "options mask" SL_CLI_UNIT_SEPARATOR "options override" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_level_control_cluster_stop_with_on_off = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_level_control_o_stop_command,
            "Command description for StopWithOnOff",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_supply_status_response = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_sup_stat_rsp_command,
            "This command is transmitted by a Metering Device in response to a ChangeSupply command.",
            "provider id" SL_CLI_UNIT_SEPARATOR "issuer event id" SL_CLI_UNIT_SEPARATOR "implementation date time" SL_CLI_UNIT_SEPARATOR "supply status" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_take_snapshot = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_take_snapshot_command,
            "This command is used to instruct the cluster server to take a single snapshot.",
            "snapshot cause" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_simple_metering_cluster_take_snapshot_response = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_simple_metering_take_snapshot_resp_command,
            "This command is generated in response to a TakeSnapshot command, and is sent to confirm whether the requested snapshot has been accepted and successfully taken.",
            "snapshot id" SL_CLI_UNIT_SEPARATOR "snapshot confirmation" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT32,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_toggle = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_toggle_command,
            "Toggles the door lock from its current state to the opposite state locked or unlocked.",
            "pin" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_STRING,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_on_off_cluster_toggle = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_on_off_toggle_command,
            "Command description for Toggle",
            "",
            {
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_identify_cluster_trigger_effect = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_identify_trigger_command,
            "Command description for TriggerEffect",
            "effect id" SL_CLI_UNIT_SEPARATOR "effect variant" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_UINT8,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_unlock_door = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_unlock_command,
            "Unlocks the door",
            "pin" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_STRING,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_door_lock_cluster_unlock_with_timeout = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_door_lock_unlock_with_timeout_command,
            "Unlock the door with a timeout. When the timeout expires, the door will automatically re-lock.",
            "timeout in seconds" SL_CLI_UNIT_SEPARATOR "pin" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_STRING,
                SL_CLI_ARG_END,
            });

static const sl_cli_command_info_t cli_cmd_zcl_groups_cluster_view_group = \
SL_CLI_COMMAND(sli_zigbee_cli_zcl_groups_view_command,
            "Command description for ViewGroup",
            "group id" SL_CLI_UNIT_SEPARATOR ,
            {
                SL_CLI_ARG_UINT16,
                SL_CLI_ARG_END,
            });


// Create group command tables and structs if cli_groups given
// in template. Group name is suffixed with [cluster name]_[cluster_side]_cluster_group_table for tables
// and group commands are cli_cmd_( group name )_group
static const sl_cli_command_entry_t zcl_basic_cluster_command_table[] = {
    { "rtfd", &cli_cmd_zcl_basic_cluster_reset_to_factory_defaults, false },
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_color_control_cluster_command_table[] = {
    { "movetohue", &cli_cmd_zcl_color_control_cluster_move_to_hue, false },
    { "movehue", &cli_cmd_zcl_color_control_cluster_move_hue, false },
    { "stephue", &cli_cmd_zcl_color_control_cluster_step_hue, false },
    { "movetosat", &cli_cmd_zcl_color_control_cluster_move_to_saturation, false },
    { "movesat", &cli_cmd_zcl_color_control_cluster_move_saturation, false },
    { "stepsat", &cli_cmd_zcl_color_control_cluster_step_saturation, false },
    { "movetohueandsat", &cli_cmd_zcl_color_control_cluster_move_to_hue_and_saturation, false },
    { "movetocolor", &cli_cmd_zcl_color_control_cluster_move_to_color, false },
    { "movecolor", &cli_cmd_zcl_color_control_cluster_move_color, false },
    { "stepcolor", &cli_cmd_zcl_color_control_cluster_step_color, false },
    { "movetocolortemp", &cli_cmd_zcl_color_control_cluster_move_to_color_temperature, false },
    { "emovetohue", &cli_cmd_zcl_color_control_cluster_enhanced_move_to_hue, false },
    { "emovehue", &cli_cmd_zcl_color_control_cluster_enhanced_move_hue, false },
    { "estephue", &cli_cmd_zcl_color_control_cluster_enhanced_step_hue, false },
    { "emovetohueandsat", &cli_cmd_zcl_color_control_cluster_enhanced_move_to_hue_and_saturation, false },
    { "loop", &cli_cmd_zcl_color_control_cluster_color_loop_set, false },
    { "stopmovestep", &cli_cmd_zcl_color_control_cluster_stop_move_step, false },
    { "movecolortemp", &cli_cmd_zcl_color_control_cluster_move_color_temperature, false },
    { "stepcolortemp", &cli_cmd_zcl_color_control_cluster_step_color_temperature, false },
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_door_lock_cluster_command_table[] = {
    { "lock", &cli_cmd_zcl_door_lock_cluster_lock_door, false },
    { "unlock", &cli_cmd_zcl_door_lock_cluster_unlock_door, false },
    { "toggle", &cli_cmd_zcl_door_lock_cluster_toggle, false },
    { "unlock-with-timeout", &cli_cmd_zcl_door_lock_cluster_unlock_with_timeout, false },
    { "get-log-record", &cli_cmd_zcl_door_lock_cluster_get_log_record, false },
    { "set-pin", &cli_cmd_zcl_door_lock_cluster_set_pin, false },
    { "get-pin", &cli_cmd_zcl_door_lock_cluster_get_pin, false },
    { "clear-pin", &cli_cmd_zcl_door_lock_cluster_clear_pin, false },
    { "set-weekday-schedule", &cli_cmd_zcl_door_lock_cluster_set_weekday_schedule, false },
    { "get-weekday-schedule", &cli_cmd_zcl_door_lock_cluster_get_weekday_schedule, false },
    { "clear-weekday-schedule", &cli_cmd_zcl_door_lock_cluster_clear_weekday_schedule, false },
    { "set-yearday-schedule", &cli_cmd_zcl_door_lock_cluster_set_yearday_schedule, false },
    { "get-yearday-schedule", &cli_cmd_zcl_door_lock_cluster_get_yearday_schedule, false },
    { "clear-yearday-schedule", &cli_cmd_zcl_door_lock_cluster_clear_yearday_schedule, false },
    { "set-holiday-schedule", &cli_cmd_zcl_door_lock_cluster_set_holiday_schedule, false },
    { "get-holiday-schedule", &cli_cmd_zcl_door_lock_cluster_get_holiday_schedule, false },
    { "clear-holiday-schedule", &cli_cmd_zcl_door_lock_cluster_clear_holiday_schedule, false },
    { "set-user-type", &cli_cmd_zcl_door_lock_cluster_set_user_type, false },
    { "get-user-type", &cli_cmd_zcl_door_lock_cluster_get_user_type, false },
    { "set-rfid", &cli_cmd_zcl_door_lock_cluster_set_rfid, false },
    { "get-rfid", &cli_cmd_zcl_door_lock_cluster_get_rfid, false },
    { "clear-rfid", &cli_cmd_zcl_door_lock_cluster_clear_rfid, false },
    { "clear-all-rfids", &cli_cmd_zcl_door_lock_cluster_clear_all_rfids, false },
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_groups_cluster_command_table[] = {
    { "add", &cli_cmd_zcl_groups_cluster_add_group, false },
    { "view", &cli_cmd_zcl_groups_cluster_view_group, false },
    { "get", &cli_cmd_zcl_groups_cluster_get_group_membership, false },
    { "remove", &cli_cmd_zcl_groups_cluster_remove_group, false },
    { "rmall", &cli_cmd_zcl_groups_cluster_remove_all_groups, false },
    { "add-if-id", &cli_cmd_zcl_groups_cluster_add_group_if_identifying, false },
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_identify_cluster_command_table[] = {
    { "id", &cli_cmd_zcl_identify_cluster_identify, false },
    { "query", &cli_cmd_zcl_identify_cluster_identify_query, false },
    { "ez-mode", &cli_cmd_zcl_identify_cluster_ez_mode_invoke, false },
    { "trigger", &cli_cmd_zcl_identify_cluster_trigger_effect, false },
    { "on", &cli_cmd_zcl_identify_on_command, false},
    { "off", &cli_cmd_zcl_identify_off_command, false},
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_level_control_cluster_command_table[] = {
    { "mv-to-level", &cli_cmd_zcl_level_control_cluster_move_to_level, false },
    { "move", &cli_cmd_zcl_level_control_cluster_move, false },
    { "step", &cli_cmd_zcl_level_control_cluster_step, false },
    { "stop", &cli_cmd_zcl_level_control_cluster_stop, false },
    { "o-mv-to-level", &cli_cmd_zcl_level_control_cluster_move_to_level_with_on_off, false },
    { "o-move", &cli_cmd_zcl_level_control_cluster_move_with_on_off, false },
    { "o-step", &cli_cmd_zcl_level_control_cluster_step_with_on_off, false },
    { "o-stop", &cli_cmd_zcl_level_control_cluster_stop_with_on_off, false },
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_on_off_cluster_command_table[] = {
    { "off", &cli_cmd_zcl_on_off_cluster_off, false },
    { "on", &cli_cmd_zcl_on_off_cluster_on, false },
    { "toggle", &cli_cmd_zcl_on_off_cluster_toggle, false },
    { "offeffect", &cli_cmd_zcl_on_off_cluster_off_with_effect, false },
    { "onrecall", &cli_cmd_zcl_on_off_cluster_on_with_recall_global_scene, false },
    { "ontimedoff", &cli_cmd_zcl_on_off_cluster_on_with_timed_off, false },
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_poll_control_cluster_command_table[] = {
    { "stop", &cli_cmd_zcl_poll_control_cluster_fast_poll_stop, false },
    { "long", &cli_cmd_zcl_poll_control_cluster_set_long_poll_interval, false },
    { "short", &cli_cmd_zcl_poll_control_cluster_set_short_poll_interval, false },
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_power_profile_cluster_command_table[] = {
    { "profile", &cli_cmd_zcl_power_profile_cluster_power_profile_request, false },
    { "state", &cli_cmd_zcl_power_profile_cluster_power_profile_state_request, false },
    { "energy-phases-schedule", &cli_cmd_zcl_power_profile_cluster_energy_phases_schedule_notification, false },
    { "schedule-constraints", &cli_cmd_zcl_power_profile_cluster_power_profile_schedule_constraints_request, false },
    { "energy-phases-schedule-states", &cli_cmd_zcl_power_profile_cluster_energy_phases_schedule_state_request, false },
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_simple_metering_cluster_command_table[] = {
    { "request-mirror", &cli_cmd_zcl_simple_metering_cluster_request_mirror, false },
    { "remove-mirror", &cli_cmd_zcl_simple_metering_cluster_remove_mirror, false },
    { "sch-snapshot-resp", &cli_cmd_zcl_simple_metering_cluster_schedule_snapshot_response, false },
    { "take-snapshot-resp", &cli_cmd_zcl_simple_metering_cluster_take_snapshot_response, false },
    { "pub-ss", &cli_cmd_zcl_simple_metering_cluster_publish_snapshot, false },
    { "cfg-mirror", &cli_cmd_zcl_simple_metering_cluster_configure_mirror, false },
    { "cfg-nft-scheme", &cli_cmd_zcl_simple_metering_cluster_configure_notification_scheme, false },
    { "cfg-nft-flags", &cli_cmd_zcl_simple_metering_cluster_configure_notification_flags, false },
    { "get-ntfy-msg", &cli_cmd_zcl_simple_metering_cluster_get_notified_message, false },
    { "sup-stat-rsp", &cli_cmd_zcl_simple_metering_cluster_supply_status_response, false },
    { "start-samp-rsp", &cli_cmd_zcl_simple_metering_cluster_start_sampling_response, false },
    { "get-profile", &cli_cmd_zcl_simple_metering_cluster_get_profile, false },
    { "mirror-removed", &cli_cmd_zcl_simple_metering_cluster_mirror_removed, false },
    { "req-fast-poll-mode", &cli_cmd_zcl_simple_metering_cluster_request_fast_poll_mode, false },
    { "sch-snapshot", &cli_cmd_zcl_simple_metering_cluster_schedule_snapshot, false },
    { "take-snapshot", &cli_cmd_zcl_simple_metering_cluster_take_snapshot, false },
    { "get-snapshot", &cli_cmd_zcl_simple_metering_cluster_get_snapshot, false },
    { "start-sampling", &cli_cmd_zcl_simple_metering_cluster_start_sampling, false },
    { "get-sampled-data", &cli_cmd_zcl_simple_metering_cluster_get_sampled_data, false },
    { "rst-load-limit-ctr", &cli_cmd_zcl_simple_metering_cluster_reset_load_limit_counter, false },
    { "chg-supply", &cli_cmd_zcl_simple_metering_cluster_change_supply, false },
    { "local-chg-supply", &cli_cmd_zcl_simple_metering_cluster_local_change_supply, false },
    { "set-supply-status", &cli_cmd_zcl_simple_metering_cluster_set_supply_status, false },
    { "set-uncntrl-flow-threshold", &cli_cmd_zcl_simple_metering_cluster_set_uncontrolled_flow_threshold, false },
    { NULL, NULL, false },
};
static const sl_cli_command_entry_t zcl_thermostat_cluster_command_table[] = {
    { "set", &cli_cmd_zcl_thermostat_cluster_setpoint_raise_lower, false },
    { "sws", &cli_cmd_zcl_thermostat_cluster_set_weekly_schedule, false },
    { "gws", &cli_cmd_zcl_thermostat_cluster_get_weekly_schedule, false },
    { "cws", &cli_cmd_zcl_thermostat_cluster_clear_weekly_schedule, false },
    { "grs", &cli_cmd_zcl_thermostat_cluster_get_relay_status_log, false },
    { NULL, NULL, false },
};

// ZCL cluster commands
static const sl_cli_command_info_t cli_cmd_basic_group = \
  SL_CLI_COMMAND_GROUP(zcl_basic_cluster_command_table, "ZCL basic cluster commands");
static const sl_cli_command_info_t cli_cmd_color_control_group = \
  SL_CLI_COMMAND_GROUP(zcl_color_control_cluster_command_table, "ZCL color_control cluster commands");
static const sl_cli_command_info_t cli_cmd_door_lock_group = \
  SL_CLI_COMMAND_GROUP(zcl_door_lock_cluster_command_table, "ZCL door_lock cluster commands");
static const sl_cli_command_info_t cli_cmd_groups_group = \
  SL_CLI_COMMAND_GROUP(zcl_groups_cluster_command_table, "ZCL groups cluster commands");
static const sl_cli_command_info_t cli_cmd_identify_group = \
  SL_CLI_COMMAND_GROUP(zcl_identify_cluster_command_table, "ZCL identify cluster commands");
static const sl_cli_command_info_t cli_cmd_level_control_group = \
  SL_CLI_COMMAND_GROUP(zcl_level_control_cluster_command_table, "ZCL level_control cluster commands");
static const sl_cli_command_info_t cli_cmd_on_off_group = \
  SL_CLI_COMMAND_GROUP(zcl_on_off_cluster_command_table, "ZCL on_off cluster commands");
static const sl_cli_command_info_t cli_cmd_poll_control_group = \
  SL_CLI_COMMAND_GROUP(zcl_poll_control_cluster_command_table, "ZCL poll_control cluster commands");
static const sl_cli_command_info_t cli_cmd_power_profile_group = \
  SL_CLI_COMMAND_GROUP(zcl_power_profile_cluster_command_table, "ZCL power_profile cluster commands");
static const sl_cli_command_info_t cli_cmd_simple_metering_group = \
  SL_CLI_COMMAND_GROUP(zcl_simple_metering_cluster_command_table, "ZCL simple_metering cluster commands");
static const sl_cli_command_info_t cli_cmd_thermostat_group = \
  SL_CLI_COMMAND_GROUP(zcl_thermostat_cluster_command_table, "ZCL thermostat cluster commands");

static const sl_cli_command_entry_t zcl_group_table[] = {
  { "basic", &cli_cmd_basic_group, false },
  { "color-control", &cli_cmd_color_control_group, false },
  { "door-lock", &cli_cmd_door_lock_group, false },
  { "groups", &cli_cmd_groups_group, false },
  { "identify", &cli_cmd_identify_group, false },
  { "level-control", &cli_cmd_level_control_group, false },
  { "on-off", &cli_cmd_on_off_group, false },
  { "poll-control", &cli_cmd_poll_control_group, false },
  { "power-profile", &cli_cmd_power_profile_group, false },
  { "simple-metering", &cli_cmd_simple_metering_group, false },
  { "thermostat", &cli_cmd_thermostat_group, false },
  { "global", &cli_cmd_zcl_global_group, false },
  { "mfg-code", &cli_cmd_zcl_mfg_code_command, false},
  { "time", &cli_cmd_zcl_time_command, false},
  { "use-next-sequence", &cli_cmd_zcl_use_next_sequence_command, false},
  { "x-default-resp", &cli_cmd_zcl_x_default_resp_command, false},
  { "test-response-on", &cli_cmd_zcl_test_response_on_command, false},
  { "test-response-off", &cli_cmd_zcl_test_response_off_command, false},
  { NULL, NULL, false },
};

sl_cli_command_info_t cli_cmd_zcl_group = \
  SL_CLI_COMMAND_GROUP(zcl_group_table, "ZCL commands");

// Create root command table
const sl_cli_command_entry_t sl_cli_zcl_command_table[] = {
  { "zcl", &cli_cmd_zcl_group, false },
  { NULL, NULL, false },
};

sl_cli_command_group_t sl_cli_zcl_command_group =
{
  { NULL },
  false,
  sl_cli_zcl_command_table
};


WEAK(void sli_zigbee_cli_zcl_groups_add_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_STRING
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_GROUPS_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_groups_add_if_id_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_STRING
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_GROUPS_CLUSTER_ID,                                                       \
    5, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_chg_supply_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[6] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    11, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_clear_all_rfids_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    25, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_clear_holiday_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    19, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_clear_pin_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    7, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_clear_rfid_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    24, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_clear_weekday_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    13, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_thermostat_cws_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_THERMOSTAT_CLUSTER_ID,                                                       \
    3, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_clear_yearday_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    16, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_loop_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[7] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    68, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_cfg_mirror_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT24,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    8, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_cfg_nft_flags_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[7] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    10, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_cfg_nft_scheme_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[3] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT32
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    9, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_identify_ez_mode_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_IDENTIFY_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_power_profile_energy_phases_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[3] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_HEXOPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_POWER_PROFILE_CLUSTER_ID,                                                       \
    4, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_power_profile_energy_phases_schedule_states_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_POWER_PROFILE_CLUSTER_ID,                                                       \
    7, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_emovehue_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    65, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_emovetohue_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    64, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_emovetohueandsat_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    67, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_estephue_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    66, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_poll_control_stop_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_POLL_CONTROL_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_groups_get_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_GROUPS_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_get_holiday_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    18, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_get_log_record_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    4, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_get_ntfy_msg_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[3] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT32
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    11, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_get_pin_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    6, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_get_profile_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[3] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_thermostat_grs_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_THERMOSTAT_CLUSTER_ID,                                                       \
    4, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_get_rfid_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    23, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_get_sampled_data_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    8, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_get_snapshot_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT32
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    6, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_get_user_type_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    21, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_get_weekday_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    12, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_thermostat_gws_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_THERMOSTAT_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_get_yearday_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    15, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_identify_id_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_IDENTIFY_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_identify_query_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_IDENTIFY_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_local_chg_supply_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    12, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_lock_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_STRING
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_mirror_removed_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_level_control_move_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_LEVEL_CONTROL_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_movecolor_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_INT16,
    SL_ZCL_CLI_ARG_INT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    8, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_movecolortemp_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[6] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    75, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_movehue_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_movesat_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    4, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_movetocolor_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    7, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_movetocolortemp_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    10, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_movetohue_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_movetohueandsat_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    6, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_level_control_mv_to_level_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_LEVEL_CONTROL_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_level_control_o_mv_to_level_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_LEVEL_CONTROL_CLUSTER_ID,                                                       \
    4, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_movetosat_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    3, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_level_control_o_move_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_LEVEL_CONTROL_CLUSTER_ID,                                                       \
    5, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_on_off_off_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_ON_OFF_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_on_off_offeffect_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_ON_OFF_CLUSTER_ID,                                                       \
    64, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_on_off_on_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_ON_OFF_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_on_off_onrecall_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_ON_OFF_CLUSTER_ID,                                                       \
    65, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_on_off_ontimedoff_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[3] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_ON_OFF_CLUSTER_ID,                                                       \
    66, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_power_profile_profile_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_POWER_PROFILE_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_power_profile_schedule_constraints_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_POWER_PROFILE_CLUSTER_ID,                                                       \
    6, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_power_profile_state_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_POWER_PROFILE_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_pub_ss_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[8] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    6, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_groups_rmall_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_GROUPS_CLUSTER_ID,                                                       \
    4, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_groups_remove_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_GROUPS_CLUSTER_ID,                                                       \
    3, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_remove_mirror_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_req_fast_poll_mode_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    3, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_request_mirror_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_rst_load_limit_ctr_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    10, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_basic_rtfd_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_BASIC_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_sch_snapshot_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_HEXOPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    4, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_sch_snapshot_resp_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_HEXOPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    4, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_set_holiday_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    17, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_poll_control_long_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT32
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_POLL_CONTROL_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_set_pin_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_STRING
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    5, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_set_rfid_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_STRING
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    22, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_poll_control_short_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_POLL_CONTROL_CLUSTER_ID,                                                       \
    3, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_set_supply_status_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    13, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_set_uncntrl_flow_threshold_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[8] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    14, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_set_user_type_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    20, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_set_weekday_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[7] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    11, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_thermostat_sws_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_THERMOSTAT_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_set_yearday_schedule_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    14, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_thermostat_set_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_INT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_THERMOSTAT_CLUSTER_ID,                                                       \
    0, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_start_sampling_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    7, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_start_samp_rsp_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    13, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_level_control_step_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_LEVEL_CONTROL_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_stepcolor_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_INT16,
    SL_ZCL_CLI_ARG_INT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    9, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_stepcolortemp_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[7] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    76, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_stephue_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_stepsat_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[5] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    5, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_level_control_o_step_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[3] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_LEVEL_CONTROL_CLUSTER_ID,                                                       \
    6, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_level_control_stop_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_LEVEL_CONTROL_CLUSTER_ID,                                                       \
    3, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_color_control_stopmovestep_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8OPT,
    SL_ZCL_CLI_ARG_UINT8OPT
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_COLOR_CONTROL_CLUSTER_ID,                                                       \
    71, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_level_control_o_stop_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_LEVEL_CONTROL_CLUSTER_ID,                                                       \
    7, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_sup_stat_rsp_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[4] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    12, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_take_snapshot_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT32
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    5, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_simple_metering_take_snapshot_resp_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT32,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT,  \
    ZCL_SIMPLE_METERING_CLUSTER_ID,                                                       \
    5, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_toggle_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_STRING
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_on_off_toggle_command(sl_cli_command_arg_t *arguments)) {
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_ON_OFF_CLUSTER_ID,                                                       \
    2, \
    arguments, \
    NULL);
}

WEAK(void sli_zigbee_cli_zcl_identify_trigger_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT8,
    SL_ZCL_CLI_ARG_UINT8
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_IDENTIFY_CLUSTER_ID,                                                       \
    64, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_unlock_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_STRING
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_door_lock_unlock_with_timeout_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[2] =  { 
    SL_ZCL_CLI_ARG_UINT16,
    SL_ZCL_CLI_ARG_STRING
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_DOOR_LOCK_CLUSTER_ID,                                                       \
    3, \
    arguments, \
    argumentTypes);
}

WEAK(void sli_zigbee_cli_zcl_groups_view_command(sl_cli_command_arg_t *arguments)) {
  uint8_t argumentTypes[1] =  { 
    SL_ZCL_CLI_ARG_UINT16
  }; 
  sli_zigbee_zcl_simple_command(
    ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER,  \
    ZCL_GROUPS_CLUSTER_ID,                                                       \
    1, \
    arguments, \
    argumentTypes);
}

#ifdef __cplusplus
}
#endif
#endif