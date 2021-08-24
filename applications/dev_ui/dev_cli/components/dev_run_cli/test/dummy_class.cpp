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

// Standard library
#include <unistd.h>

// Import command handler class
#include "dev_cli_command_handler.hpp"

// Pre decleration of the main function for command
int test_main();

/*
Initiating the command command is done by doing as follows:
The first argument is the command you need to input as argument for CLI =>
Template.command The second argument is a small text describing the
funcitonality and intent of command => Template.arguments_help The third
argument is if you need additional input arguments then you explain these there
=> Template.arguments_help The fourth and last argument is your function you
want to be main. You pass it to the class and it will setup a pointer to it =>
Template.command_ptr
*/

CommandTemplate Test("test", "test help", "usage: []", test_main);

int test_main()
{
  printf("Executing test main - yay\n");

  return 0;
}