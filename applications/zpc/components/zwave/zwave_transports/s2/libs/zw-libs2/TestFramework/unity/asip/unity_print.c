/* © 2017 Silicon Laboratories Inc.
 */

#include "../unity_print.h"
#include <stdint.h>

#define ASIP_THREEPORT_ThreadAnd  (0x8020)

/*extern uint8_t  * gUnityMessageBuffer;
extern uint16_t   gUnityMessageBufferSize;*/


uint8_t p [1024];

uint8_t * gUnityMessageBuffer = p;
uint16_t   gUnityMessageBufferSize = sizeof(p);
/*void longjmp() {
  while(1);
}
int setjmp()
{
  return 0;
}
*/
static uint16_t   mMessageBufferIndex;

void unity_print_init(void)
{
  gUnityMessageBuffer[gUnityMessageBufferSize-1] = 0;
}

int ZW_putchar(int c)
{
  if (c != 0 && (mMessageBufferIndex < gUnityMessageBufferSize))
    gUnityMessageBuffer[mMessageBufferIndex++] = c;
  return 0;
}

void unity_print_close(void)
{
  *((int *)ASIP_THREEPORT_ThreadAnd) = 0x00000000;
  while(1);
}

