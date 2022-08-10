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
// Includes from this component
#include "datastore.h"
#include "datastore_fixt.h"

// Test includes
#include "unity.h"

// Unify components
#include "sl_status.h"
#include "sl_log.h"
#include "uic_version.h"

// Generic includes
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define DATASTORE_VERSION_V1 1
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTeardown(int num_failures)
{
  return num_failures;
}

// Test storing int in the datastore
void test_datastore_int_sunny_day()
{
  const int mydata = 1337;
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_init(":memory:"),
                            "Initialization of datastore");
  TEST_ASSERT_FALSE_MESSAGE(datastore_contains_int("mykey"),
                            "mykey shouldn't be in the datastore");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_store_int("mykey", mydata),
                            "Expect OK when storing data");

  TEST_ASSERT_TRUE_MESSAGE(datastore_contains_int("mykey"),
                           "mykey should be in the datastore");

  int64_t mydata_fetched = 0;
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_fetch_int("mykey", &mydata_fetched),
                            "Expect OK when fetching mykey");
  TEST_ASSERT_EQUAL_MESSAGE(mydata,
                            mydata_fetched,
                            "Expect fetched data to match mydata");
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_teardown(),
                            "Expect teardown to match SL_STATUS_OK");
}

void test_datastore_array_sunny_day()
{
  const uint8_t mydata[] = {1, 3, 3, 7};
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_init(":memory:"),
                            "Initialization of datastore");
  TEST_ASSERT_FALSE_MESSAGE(datastore_contains_arr("mykey"),
                            "mykey shouldn't be in the datastore");

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    datastore_store_arr("mykey", mydata, sizeof(mydata)),
    "Expect OK when storing data");

  TEST_ASSERT_TRUE_MESSAGE(datastore_contains_arr("mykey"),
                           "mykey should be in the datastore");

  uint8_t mydata_fetched[sizeof(mydata)] = {0};
  unsigned int fetched_size              = sizeof(mydata_fetched);
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    datastore_fetch_arr("mykey", mydata_fetched, &fetched_size),
    "Expect OK when fetching mykey");
  TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(mydata,
                                       mydata_fetched,
                                       sizeof(mydata_fetched),
                                       "Expect fetched data to match mydata");
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_teardown(),
                            "Expect teardown to match SL_STATUS_OK");
}

void test_datastore_init_existing_db()
{
  const char db_name[] = "mytestdb.db";
  /// Remove db file if it exists
  if (access(db_name, F_OK) != -1) {
    TEST_ASSERT_EQUAL_MESSAGE(0,
                              remove(db_name),
                              "Expect Remove DB file successful");
  }
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_init(db_name),
                            "Initialization of datastore");
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_teardown(),
                            "Expect teardown to match SL_STATUS_OK");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_init(db_name),
                            "Initialization of datastore");
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_teardown(),
                            "Expect teardown to match SL_STATUS_OK");
  /// Cleanup
  TEST_ASSERT_EQUAL_MESSAGE(0,
                            remove(db_name),
                            "Expect Remove DB file successful");
}

void test_datastore_add_existing_key()
{
  const int mydata = 1337;
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_init(":memory:"),
                            "Initialization of datastore");
  TEST_ASSERT_FALSE_MESSAGE(datastore_contains_int("mykey"),
                            "mykey shouldn't be in the datastore");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_store_int("mykey", mydata),
                            "Expect OK when storing data");

  TEST_ASSERT_TRUE_MESSAGE(datastore_contains_int("mykey"),
                           "mykey should be in the datastore");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_store_int("mykey", mydata),
                            "Expect OK when storing data");

  int64_t mydata_fetched = 0;
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_fetch_int("mykey", &mydata_fetched),
                            "Expect OK when fetching mykey");
  TEST_ASSERT_EQUAL_MESSAGE(mydata,
                            mydata_fetched,
                            "Expect fetched data to match mydata");
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_teardown(),
                            "Expect teardown to match SL_STATUS_OK");
}

/// Ensure we don't crash if datastore_init hasn't been called
void test_datastore_missing_init()
{
  int64_t mydata               = 1337;
  unsigned int mydata_arr_size = 2;
  uint8_t mydata_arr[]         = {1, 2};
  TEST_ASSERT_FALSE_MESSAGE(datastore_contains_int("mykey"),
                            "mykey shouldn't be in the datastore");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            datastore_store_int("mykey", mydata),
                            "Expect SL_STATUS_FAIL when storing data");
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                            datastore_fetch_int("mykey", &mydata),
                            "Expect SL_STATUS_NOT_FOUND when fetching data");
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    datastore_store_arr("mykey", mydata_arr, mydata_arr_size),
    "Expect SL_STATUS_FAIL when storing data");

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    datastore_fetch_arr("mykey", mydata_arr, &mydata_arr_size),
    "Expect SL_STATUS_NOT_FOUND when fetching data");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_teardown(),
                            "Expect teardown to match SL_STATUS_OK");
}

void test_datastore_fixt_setup()
{
  datastore_fixt_setup_and_handle_version("datastore.db", DATASTORE_VERSION_V1);
  uint8_t fetched_uic_version[UIC_VERSION_MAX_LEN] = {0};
  unsigned int fetched_uic_version_size            = UIC_VERSION_MAX_LEN;

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_fetch_arr("uic_version",
                                                fetched_uic_version,
                                                &fetched_uic_version_size),
                            "Expect OK when fetching uic_version");
  TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(
    fetched_uic_version,
    UIC_VERSION,
    fetched_uic_version_size,
    "Expect fetched UIC version to match fetched_uic_version");
  int64_t version_fetched = 0;
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_fetch_int("version", &version_fetched),
                            "Expect OK when fetching version");
  TEST_ASSERT_EQUAL_MESSAGE(
    DATASTORE_VERSION_V1,
    version_fetched,
    "Expect fetched version to match DATASTORE_VERSION_V1");

  //Write incompatible version number in datastore
  datastore_store_int("version", 100);
  //Check if it fails or not
  TEST_ASSERT_EQUAL(
    datastore_fixt_setup_and_handle_version("datastore.db",
                                            DATASTORE_VERSION_V1),
    SL_STATUS_FAIL);
  //Remove the datastore.db or in next run the test case will fail
  TEST_ASSERT_EQUAL_MESSAGE(0,
                            remove("datastore.db"),
                            "Expect Remove DB file successful");
}

void test_datastore_underflow_protection()
{
  const uint8_t mydata[] = {1, 3, 3, 7};
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_init(":memory:"),
                            "Initialization of datastore");
  TEST_ASSERT_FALSE_MESSAGE(datastore_contains_arr("mykey"),
                            "mykey shouldn't be in the datastore");

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    datastore_store_arr("mykey", mydata, sizeof(mydata)),
    "Expect OK when storing data");

  TEST_ASSERT_TRUE_MESSAGE(datastore_contains_arr("mykey"),
                           "mykey should be in the datastore");

  uint8_t mydata_fetched[sizeof(mydata) - 1] = {0};
  unsigned int fetched_size                  = sizeof(mydata_fetched);
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_WOULD_OVERFLOW,
    datastore_fetch_arr("mykey", mydata_fetched, &fetched_size),
    "Expect OK when fetching mykey");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_teardown(),
                            "Expect teardown to match SL_STATUS_OK");
}

void test_datastore_transaction()
{
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, datastore_start_transaction());
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_init(":memory:"),
                            "Initialization of datastore");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, datastore_commit_transaction());
  TEST_ASSERT_EQUAL(SL_STATUS_OK, datastore_start_transaction());
  TEST_ASSERT_EQUAL(SL_STATUS_IN_PROGRESS, datastore_start_transaction());
  TEST_ASSERT_EQUAL(SL_STATUS_OK, datastore_commit_transaction());
}
