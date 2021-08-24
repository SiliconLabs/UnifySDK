/* # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

// Component includes
#include "dev_cli_command_runner.h"

// Test framework
#include <unity.h>

// Standard libraries
#include <stdbool.h>
#include <string.h>

// Pseudo standard library
#include <dirent.h>

// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  printf("Initialising test");
}

// Check that it is possible to print all commands
void test_print_commands_help()
{
  bool state = print_help();
  TEST_ASSERT_TRUE(state);
}

// Test that the correct number of command class objects is intialized
void test_correct_number_of_commands()
{
  int number_of_commands = return_number_of_commands();

  // One could count the number of files but the folder strcuture changes on
  // Jenkins and it is an unstable approach in the long run. So the logic is
  // that if there is more than one command the initialization of command
  // classes at compile time is succesfull.
  printf("Number of commands must be above 0: %d>0\n", number_of_commands);

  TEST_ASSERT_TRUE(number_of_commands > 0);
}

// Try and test running a dummy command class
void test_executing_command()
{
  bool state = run_command("test");
  TEST_ASSERT_TRUE(state);
}

// Teardown the test suite (called once after all test_xxx functions are
// called)
int suiteTearDown(int num_failures)
{
  printf("Ending test suite");
  return num_failures;
}
