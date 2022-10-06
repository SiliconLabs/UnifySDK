#include <stdint.h>

/**
* Makes a fake System time available to LibS2.
*/
uint32_t clock_time(void)
{
  static uint16_t iTime;
  
  iTime++;
  return iTime;
}
