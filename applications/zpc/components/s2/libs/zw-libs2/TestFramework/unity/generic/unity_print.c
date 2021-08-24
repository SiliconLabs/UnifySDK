/* © 2017 Silicon Laboratories Inc.
 */

#include "../unity_print.h"
#include <stdio.h>
#include <string.h>


void unity_print_init(void)
{
  // Do nothing. On i386 output is automatically sent to stdout.
  // Disable all print buffering. This ensures output is written to stdout as early as possible
  // and minimizes risk of lost print messages if program seg faults.
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}

void unity_print_close(void)
{
  // Do nothing. On i386 output is automatically sent to stdout.
}

int ZW_putchar(int c)
{
  return putchar(c);
}

