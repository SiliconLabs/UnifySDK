/* © 2017 Silicon Laboratories Inc.
 */

#include "../unity_print.h"
#include "../unity.h"
#include <stdio.h>
#include <string.h>
#include <ZW700.h>

#define UNITY_PRINT_BAUD_RATE          115200   /**< Baud rate to use for unity printing, 115200. */
#define UNITY_PRINT_TX_ENABLE          1
#define UNITY_PRINT_RX_ENABLE          1
#define UNITY_PRINT_UART_CTRL_HS_TEST  (1 << 6)

#define __XTAL            ( 9600000UL)    /* Oscillator frequency             */
#define __SYSTEM_CLOCK    (5*__XTAL)

extern uint8_t * gUnityMessageBuffer;
extern uint32_t  gUnityMessageBufferSize;

uint8_t * gUnityMessageBuffer    __attribute__((weak)) = NULL;
uint32_t  gUnityMessageBufferSize __attribute__((weak)) = 0;

void unity_print_init(void)
{
  //  ZW_UART0_init(1152,1,1);
  ARM_UART0->BAUDDIV = __SYSTEM_CLOCK / UNITY_PRINT_BAUD_RATE;
  ARM_UART0->CTRL   |= APB_UART_CTRL_TX_ENABLE;
  ARM_UART0->CTRL   |= APB_UART_CTRL_RX_ENABLE;
  ARM_UART0->CTRL   |= UNITY_PRINT_UART_CTRL_HS_TEST;
}

int ZW_putchar(int c)
{
  while(ARM_UART0->STATE & APB_UART_STATE_TX_FULL );
  ARM_UART0->DATA = c; // Transmit char
  return c;
}

/** Function for printing ASIP messages.
 *
 *  @details When unit testing on ASIP a memory buffer is used to pass the log messages from the
 *           ASIP to the ARM Cortex CPU. This function is used for reading the memory buffer from
 *           the ASIP and write the content to the UART.
 */
static void unity_print_asip(void)
{
  uint32_t messageBufferIndex = 0;

  if ((gUnityMessageBuffer == NULL) || (gUnityMessageBufferSize == 0))
  {
    return;
  }

  UnityPrint("UnityAsip - start.");
  ZW_putchar('\n');
  while((messageBufferIndex < gUnityMessageBufferSize) && (gUnityMessageBuffer[messageBufferIndex] != 0))
  {
    ZW_putchar(gUnityMessageBuffer[messageBufferIndex++]);
  }
  ZW_putchar('\n');
  UnityPrint("UnityAsip - end.");
  ZW_putchar('\n');
}

void unity_print_close(void)
{
  unity_print_asip();
  ZW_putchar('\04');
}

int fputc(int c, FILE * stream)
{
  (void)stream;
  return ZW_putchar(c);
}
