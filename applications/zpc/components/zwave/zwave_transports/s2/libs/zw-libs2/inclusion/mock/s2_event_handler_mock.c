/* © 2015 Silicon Laboratories Inc.
 */
/*
 * s2_extern_mock.c
 *
 *  Created on: Aug 31, 2015
 *      Author: trasmussen
 */

#include <stdint.h>
#include "S2.h"
#include "mock_control.h"

#define MOCK_FILE "s2_event_handler_mock.c"

#define MOCK_CALL_COMPARE_INPUT_STRUCT_S2(P_MOCK, ARGUMENT, P_ACTUAL) do {                                       \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT16(P_MOCK, ARGUMENT, P_ACTUAL, zwave_event_t, evt.s2_event.peer.l_node);    \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT16(P_MOCK, ARGUMENT, P_ACTUAL, zwave_event_t, evt.s2_event.peer.r_node);    \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT8(P_MOCK, ARGUMENT, P_ACTUAL, zwave_event_t, evt.s2_event.peer.tx_options);\
  } while (0)


/**
 * This section contains mocking of the functions that are expeted to be implemented externally of
 * the s2 library.
 */
void s2_event_handler(zwave_event_t *p_actual_evt)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, p_actual_evt);

  if (p_mock->compare_rule_arg[0] == COMPARE_STRICT)
  {
    TEST_ASSERT_EQUAL_UINT32(((zwave_event_t *)p_mock->expect_arg[0].pointer)->event_type, p_actual_evt->event_type);

    //MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, p_actual_evt); // TODO: Forcing COMPARE_ANY for temporary development.

    switch (((zwave_event_t *)p_mock->expect_arg[0].pointer)->event_type)
    {
      case S2_NODE_INCLUSION_KEX_REPORT_EVENT:
        MOCK_CALL_COMPARE_STRUCT_MEMBER(p_mock, ARG0, p_actual_evt, zwave_event_t, evt.s2_event.s2_data.kex_report.security_keys, UINT8);
        MOCK_CALL_COMPARE_STRUCT_MEMBER(p_mock, ARG0, p_actual_evt, zwave_event_t, evt.s2_event.s2_data.kex_report.csa, UINT8);
      break;

      case S2_NODE_INCLUSION_PUBLIC_KEY_CHALLENGE_EVENT:
        MOCK_CALL_COMPARE_STRUCT_ARRAY_UINT8(p_mock, ARG0, p_actual_evt, zwave_event_t, evt.s2_event.s2_data.challenge_req.public_key, evt.s2_event.s2_data.challenge_req.length);
        MOCK_CALL_COMPARE_STRUCT_MEMBER(p_mock, ARG0, p_actual_evt, zwave_event_t, evt.s2_event.s2_data.challenge_req.granted_keys, UINT8);
        MOCK_CALL_COMPARE_STRUCT_MEMBER(p_mock, ARG0, p_actual_evt, zwave_event_t, evt.s2_event.s2_data.challenge_req.dsk_length, UINT8);
      break;

      case S2_NODE_INCLUSION_COMPLETE_EVENT:
        MOCK_CALL_COMPARE_STRUCT_MEMBER(p_mock, ARG0, p_actual_evt, zwave_event_t, evt.s2_event.s2_data.inclusion_complete.exchanged_keys, UINT8);
      break;

      case S2_NODE_JOINING_COMPLETE_EVENT:
        MOCK_CALL_COMPARE_STRUCT_MEMBER(p_mock, ARG0, p_actual_evt, zwave_event_t, evt.s2_event.s2_data.inclusion_complete.exchanged_keys, UINT8);
      break;

      case S2_JOINING_COMPLETE_NEVER_STARTED_EVENT:
        MOCK_CALL_COMPARE_STRUCT_MEMBER(p_mock, ARG0, p_actual_evt, zwave_event_t, evt.s2_event.s2_data.inclusion_complete.exchanged_keys, UINT8);
      break;

      case S2_NODE_INCLUSION_FAILED_EVENT:
        MOCK_CALL_COMPARE_STRUCT_MEMBER(p_mock, ARG0, p_actual_evt, zwave_event_t, evt.s2_event.s2_data.inclusion_fail.kex_fail_type, UINT8);
      break;

      case S2_NODE_INCLUSION_INITIATED_EVENT:
        // Event type match already done, no further data to verify on this event.
      break;

      default:
        TEST_FAIL_MESSAGE("Unsupported event type expected on this mock in " TO_STR_MACRO(__FILE__) ":" TO_STR_MACRO(__LINE__));
      break;
    }

  }
}
