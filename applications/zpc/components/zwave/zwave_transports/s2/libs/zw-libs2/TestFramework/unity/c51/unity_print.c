/* © 2017 Silicon Laboratories Inc.
 */

#include "../unity_print.h"
#include <stdio.h>
#include "reg51.h"

sfr SFRPAGE   = 0xFF;
sfr UART0BRL  = 0xAF;
sfr UARTCON   = 0xEC;
sfr UARTSTAT  = 0xED;
sfr UART0BUF  = 0xEF;

sfr WDTCON    = 0xE4;

#define WDTCON_PAGE SFRPAGE=0x02
#define WATCHDOG_DISABLE {WDTCON_PAGE; WDTCON = WDTCON_NOTIFY_BIT; WDTCON = 0;}
#define WDTCON_NOTIFY_BIT              0x04

#define UARTCON_PAGE
#define UARTSTAT_PAGE
#define UART0BUF_PAGE /* ANY */
#define UART0BRL_PAGE      SFRPAGE=0x02
#define UART0BRL_BRL0_BITS 0xFF

#define UARTCON_CTX1_BIT               0x80
#define UARTCON_REN1_BIT               0x40
#define UARTCON_CTX0_BIT               0x08
#define UARTCON_REN0_BIT               0x04
#define UARTCON_BRH0_BITS              0x03

#define UARTSTAT_TA0_BIT               0x01

sfr RSETVEC   = 0xE6;
#define RSETVEC_PORTKEEP_BIT           0x40
#define RSETVEC_PAGE SFRPAGE=0x02

#define SFR_AND(r,v) {r##_PAGE;r=(r&v);}
#define SFR_SET(r,v) {r##_PAGE;r=v;}
#define SFR_OR(r,v) {r##_PAGE;r=(r|v);}

#define OPEN_IOS {SFR_AND(RSETVEC,~RSETVEC_PORTKEEP_BIT);}

#define UART0_SET_BAUD_RATE(BAUD_TIMER) {SFR_AND(UARTCON,  ~UARTCON_BRH0_BITS);\
                                         SFR_OR(UARTCON,((BAUD_TIMER & 0x0300) >> 8));\
                                         SFR_SET(UART0BRL, (BAUD_TIMER & (0x00FF)));\
                                        }
#define UART0_TX_WRITE_BUFFER(TX_DATA) {SFR_SET(UART0BUF,  TX_DATA);}
#define UART0_TX_ENABLE      {SFR_OR(UARTCON, UARTCON_CTX0_BIT);}
#define UART0_TX_ACTIVE_GET  (UARTSTAT & UARTSTAT_TA0_BIT)


void unity_print_init(void)
{
#if 1
  SCON  = 0x50;                   /* SCON: mode 1, 8-bit UART, enable rcvr    */
  TMOD |= 0x20;                   /* TMOD: timer 1, mode 2, 8-bit reload      */
  TH1   = 0xf3;                   /* TH1:  reload value for 2400 baud         */
  TR1   = 1;                      /* TR1:  timer 1 run                        */
  TI    = 1;                      /* TI:   set TI to send first char of UART  */
#else
  int bBaudRate;
  WATCHDOG_DISABLE;
  bBaudRate = 1152;
  bBaudRate = (80000/bBaudRate ) + (((80000 %bBaudRate ) >= (bBaudRate >> 1))  ? 1:0);
  UART0_SET_BAUD_RATE(68);
  UART0_TX_ENABLE;
  
  OPEN_IOS
  UART0BUF = '*';
#endif

}

#if 0
char
putchar(
  char c)  /* IN a byte to write to the UART transmit register.*/
{
  while(UART0_TX_ACTIVE_GET);
  UART0_TX_WRITE_BUFFER(c);
  return c;
}
#endif

void unity_print_close(void)
{
  // Do nothing.
}

int ZW_putchar(int c)
{
  while(UART0_TX_ACTIVE_GET);
  UART0_TX_WRITE_BUFFER(c);
  return c;
}

