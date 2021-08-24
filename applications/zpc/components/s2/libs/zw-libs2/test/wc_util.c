/* © 2014 Silicon Laboratories Inc.
 */
/*
 * wc_util.c
 *
 *  Created on: 16/07/2015
 *      Author: COlsen
 */

#ifdef __C51__

#include <ZW_uart_api.h>
#define print_nl() ZW_DEBUG_SEND_NL()
#define print_num(data)  ZW_DEBUG_SEND_NUM(data)
#define print_str(STR) ZW_DEBUG_SEND_STR(STR)
#else
#include <stdio.h>

#define print_nl() puts("\n")
#define print_num(data)  printf("%x", data)
#define print_str(STR) puts(STR)
#endif

#include <stdint.h>

/*
#define ZW_DEBUG_MYPRODUCT_SEND_BYTE(data) ZW_DEBUG_SEND_BYTE(data)
#define ZW_DEBUG_MYPRODUCT_SEND_STR(STR) ZW_DEBUG_SEND_STR(STR)
#define ZW_DEBUG_MYPRODUCT_SEND_NUM(data)  ZW_DEBUG_SEND_NUM(data)
#define ZW_DEBUG_MYPRODUCT_SEND_WORD_NUM(data) ZW_DEBUG_SEND_WORD_NUM(data)
#define ZW_DEBUG_MYPRODUCT_SEND_NL()  ZW_DEBUG_SEND_NL()
 */
void print_number(uint8_t number)
{
  print_num(number);
}

void print_txt(char * str)
{
  print_str(str);
}

void print16(uint8_t * pData)
{
  uint8_t i;
  print_nl();
  for (i = 0;i < 16;++i)
  {
    if (i > 0)
    {
      print_str(",");
    }
    else
    {
      print_str(" ");
    }
    print_num(pData[i]);
    if (i % 8 == 7)
    {
      print_nl();
    }
  }
}

void print32(uint8_t * pData)
{
  uint8_t i;
  print_nl();
  for (i = 0;i < 32;++i)
  {
    if (i > 0)
    {
      print_str(",");
    }
    else
    {
      print_str(" ");
    }
    print_num(pData[i]);
    if (i % 8 == 7)
    {
      print_nl();
    }
  }
}


void print64(uint8_t * pData)
{
  uint8_t i;
  print_nl();
  for (i = 0;i < 64;++i)
  {
    if (i > 0)
    {
      print_str(",");
    }
    else
    {
      print_str(" ");
    }
    print_num(pData[i]);
    if (i % 8 == 7)
    {
      print_nl();
    }
  }
}
