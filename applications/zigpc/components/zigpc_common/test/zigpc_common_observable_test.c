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

#include "unity.h"

#include "zigpc_common_observable.h"

#include "zigpc_test_observer_mock.h"

#define TEST_OBSERVABLE_EVENTS_COUNT 5

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp() {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

static struct zigpc_observable test_observable;
static zigpc_observer_event_t
  test_observable_events[TEST_OBSERVABLE_EVENTS_COUNT]
  = {0, 1, 2, 3, 4};

void setUp()
{
  zigpc_observable_clear(&test_observable);
}

void tearDown() {}

/**
 * @brief Test to check invalid args passed when initializing observable
 *
 */
void test_zigpc_observable_init_should_handle_invalid_args(void)
{
  // ARRANGE
  sl_status_t result_null;
  sl_status_t result_invalid_instance;
  sl_status_t result_invalid_event_list;
  sl_status_t result_invalid_event_count;

  // ACT
  result_null               = zigpc_observable_init(NULL, NULL, 0);
  result_invalid_instance   = zigpc_observable_init(NULL,
                                                  test_observable_events,
                                                  TEST_OBSERVABLE_EVENTS_COUNT);
  result_invalid_event_list = zigpc_observable_init(&test_observable, NULL, 1);
  result_invalid_event_count
    = zigpc_observable_init(&test_observable, test_observable_events, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_null);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_invalid_instance);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_invalid_event_list);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_invalid_event_count);
}

/**
 * @brief Test to check event count passed when initializing observable
 *
 */
void test_zigpc_observable_init_should_handle_overflow_of_events_array(void)
{
  // ARRANGE
  sl_status_t result;

  // ACT
  result = zigpc_observable_init(&test_observable,
                                 test_observable_events,
                                 ZIGPC_OBSERVER_EVENTS_MAX + 1);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW, result);
}

/**
 * @brief Test to check for duplicate events passed when initializing observable
 *
 */
void test_zigpc_observable_init_should_handle_duplicate_events(void)
{
  // ARRANGE
  sl_status_t result;
  zigpc_observer_event_t test_events[7] = {0, 5, 3, 2, 6, 3, 1};

  // ACT
  result = zigpc_observable_init(&test_observable, test_events, 7);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_CONFIGURATION, result);
}

/**
 * @brief Test to check invalid args passed when registering observer callbacks
 *
 */
void test_zigpc_observable_registering_should_handle_invalid_args(void)
{
  // ARRANGE
  sl_status_t result_invalid;
  sl_status_t result_invalid_instance;
  sl_status_t result_invalid_observer;

  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  // ACT
  result_invalid = zigpc_observable_register(NULL, 0, NULL);
  result_invalid_instance
    = zigpc_observable_register(NULL, 0, zigpc_test_observer);
  result_invalid_observer
    = zigpc_observable_register(&test_observable, 0, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_invalid);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_invalid_instance);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_invalid_observer);
}

/**
 * @brief Test to check invalid event type passed when registering observers
 *
 */
void test_zigpc_observable_registering_should_handle_invalid_event_type(void)
{
  // ARRANGE
  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  // ACT
  sl_status_t result
    = zigpc_observable_register(&test_observable, 0xFF, zigpc_test_observer);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_TYPE, result);
}

/**
 * @brief Test to check status when duplicate entries are registered
 *
 */
void test_zigpc_observable_registering_should_handle_duplicate_entries(void)
{
  // ARRANGE
  sl_status_t result;

  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);
  zigpc_observable_register(&test_observable, 0, zigpc_test_observer);

  // ACT
  result = zigpc_observable_register(&test_observable, 0, zigpc_test_observer);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, result);
}

/**
 * @brief Test Helpers to generate observer functions
 *
 */
#define GEN_OBSERVE_FUNC(postfix) \
  void gen_observer_##postfix(void *data) {}

GEN_OBSERVE_FUNC(0)
GEN_OBSERVE_FUNC(1)
GEN_OBSERVE_FUNC(2)
GEN_OBSERVE_FUNC(3)
GEN_OBSERVE_FUNC(4)

/**
 * @brief Test helpers to reference generated observer functions
 *
 */
#define OBSERVE_FUNC(postfix) gen_observer_##postfix

/**
 * @brief Test to check if registration produces an error when full
 *
 */
void test_zigpc_observable_registering_should_inform_max_capacity(void)
{
  // ARRANGE
  sl_status_t result;

  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  zigpc_observable_register(&test_observable,
                            test_observable_events[0],
                            OBSERVE_FUNC(0));
  zigpc_observable_register(&test_observable,
                            test_observable_events[0],
                            OBSERVE_FUNC(1));
  zigpc_observable_register(&test_observable,
                            test_observable_events[0],
                            OBSERVE_FUNC(2));
  zigpc_observable_register(&test_observable,
                            test_observable_events[0],
                            OBSERVE_FUNC(3));
  zigpc_observable_register(&test_observable,
                            test_observable_events[0],
                            OBSERVE_FUNC(4));

  // ACT
  result = zigpc_observable_register(&test_observable,
                                     test_observable_events[0],
                                     zigpc_test_observer);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NO_MORE_RESOURCE, result);
}

/**
 * @brief Test to check invalid args passed when unregistering observers
 *
 */
void test_zigpc_observable_unregistering_should_handle_invalid_args(void)
{
  // ARRANGE
  sl_status_t result_invalid;
  sl_status_t result_invalid_instance;
  sl_status_t result_invalid_observer;

  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  // ACT
  result_invalid = zigpc_observable_unregister(NULL, 0, NULL);
  result_invalid_instance
    = zigpc_observable_unregister(NULL, 0, zigpc_test_observer);
  result_invalid_observer
    = zigpc_observable_unregister(&test_observable, 0, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_invalid);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_invalid_instance);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result_invalid_observer);
}

/**
 * @brief Test to check invalid event type passed when unregistering observers
 *
 */
void test_zigpc_observable_unregistering_should_handle_invalid_event_type(void)
{
  // ARRANGE
  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  // ACT
  sl_status_t result
    = zigpc_observable_unregister(&test_observable, 0xFF, zigpc_test_observer);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_TYPE, result);
}

/**
 * @brief Test to check if unregistering an already unregistered observer succeeds
 *
 */
void test_zigpc_observable_unregistering_observer_twice_should_succeed(void)
{
  // ARRANGE
  sl_status_t result;

  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  zigpc_observable_register(&test_observable,
                            test_observable_events[1],
                            zigpc_test_observer);
  zigpc_observable_unregister(&test_observable,
                              test_observable_events[1],
                              zigpc_test_observer);

  // ACT
  result = zigpc_observable_unregister(&test_observable,
                                       test_observable_events[1],
                                       zigpc_test_observer);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, result);
}

/**
 * @brief Test to cehck if notify API checks for invalid parameters
 *
 */
void test_zigpc_observable_should_check_invalid_params_on_notify(void)
{
  // ARRANGE
  sl_status_t result;

  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  // ACT
  result = zigpc_observable_notify(NULL, 0, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, result);
}

/**
 * @brief Test to check if an observer is called on a registered event
 *
 */
void test_zigpc_observable_should_notify_observer_on_registered_event(void)
{
  // ARRANGE
  struct {
    int x;
  } test_struct = {
    .x = 0xABCDEF,
  };

  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  zigpc_observable_register(&test_observable,
                            test_observable_events[0],
                            zigpc_test_observer);

  zigpc_test_observer_Expect(&test_struct);

  // ACT
  zigpc_observable_notify(&test_observable,
                          test_observable_events[0],
                          (void *)&test_struct);

  // ASSERT
  // Handled by CMock
}

/**
 * @brief Test to check if observers are not called on any other registered events
 *
 */
void test_zigpc_observable_should_not_notify_observer_on_other_event(void)
{
  // ARRANGE
  struct {
    int x;
  } test_struct = {
    .x = 0xABCDEF,
  };

  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  zigpc_observable_register(&test_observable,
                            test_observable_events[0],
                            zigpc_test_observer);

  // ACT
  sl_status_t result = zigpc_observable_notify(&test_observable,
                                               test_observable_events[1],
                                               (void *)&test_struct);

  // ASSERT
  // Handled by CMock (zigpc_test_observer should not be called)
  TEST_ASSERT_EQUAL(SL_STATUS_OK, result);
}

/**
 * @brief Test to check if observers are not called on any non-registered events
 *
 */
void test_zigpc_observable_should_not_notify_observer_on_unknown_event(void)
{
  // ARRANGE
  struct {
    int x;
  } test_struct = {
    .x = 0xABCDEF,
  };

  zigpc_observable_init(&test_observable,
                        test_observable_events,
                        TEST_OBSERVABLE_EVENTS_COUNT);

  zigpc_observable_register(&test_observable,
                            test_observable_events[0],
                            zigpc_test_observer);

  // ACT
  sl_status_t result
    = zigpc_observable_notify(&test_observable, 0xFF, (void *)&test_struct);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_TYPE, result);

  // ASSERT
  // Handled by CMock (zigpc_test_observer should not be called)
}
