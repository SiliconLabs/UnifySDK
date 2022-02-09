/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "uic_gbl_interface.h"
#include "unity.h"
#include "sl_log.h"
#include <stdbool.h>

#define LOG_TAG "uic_gbl_interface_test"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}
/*
const char* gbl_menu[] =
"Gecko Bootloader v1.5.1\n"
"1. upload gbl\n"
"2. run\n"
"3. ebl info\n"
"BL >\n"
""
;
*/
/// Test of uic_gbl_interface_get_name()
void test_uic_gbl_interfaces()
{
  // FIXME: How do I test this?.....
  // maybe with a pipe to a script emulating the gbl ?
  TEST_IGNORE_MESSAGE("Testcase not implemented");
}
