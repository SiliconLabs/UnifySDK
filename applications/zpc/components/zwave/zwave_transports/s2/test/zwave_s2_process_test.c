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

#include "cmock.h"
#include "contiki_test_helper.h"
#include "process.h"

#include "zwave_controller_transport_mock.h"
#include "zwave_controller_callbacks_mock.h"

#include "S2_mock.h"
#include "s2_inclusion_mock.h"
#include "sl_log.h"
PROCESS_NAME(zwave_s2_process);

CTR_DRBG_CTX s2_ctr_drbg;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  contiki_test_helper_init();
}

void tearDown()
{
  process_exit(&zwave_s2_process);
}

/// Test case for timers used in S2
///
void test_s2_timers()
{
  s2_inclusion_init_IgnoreAndReturn(TRUE);
  S2_init_ctx_IgnoreAndReturn(0);
  s2_inclusion_set_event_handler_Ignore();
  zwave_controller_transport_register_IgnoreAndReturn(SL_STATUS_OK);
  zwave_controller_register_callbacks_IgnoreAndReturn(SL_STATUS_OK);
  s2_get_key_count_ExpectAndReturn(TRUE);

  process_start(&zwave_s2_process, 0);
  contiki_test_helper_run(0);

  //Test S2 send data timer
  S2_set_timeout(0, 1000);
  // Dont fire yet
  contiki_test_helper_run(10);
  // Fire now
  S2_timeout_notify_Expect(0);
  contiki_test_helper_run(1000);

  //Test S2 inclusion timer (this timer works in 10ms steps)
  s2_inclusion_set_timeout(0, 200);
  // Dont fire yet
  contiki_test_helper_run(10);
  // Fire now
  s2_inclusion_notify_timeout_Expect(0);
  contiki_test_helper_run(2000);

  //Test S2 inclusion timer (this timer works in 10ms steps)
  s2_inclusion_set_timeout(0, 200);
  // Dont fire yet
  contiki_test_helper_run(10);
  // Make sure that the timer stop works
  s2_inclusion_stop_timeout();
  contiki_test_helper_run(10);
  contiki_test_helper_run(2000);
}
