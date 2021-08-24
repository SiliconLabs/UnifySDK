/* © 2017 Silicon Laboratories Inc.
 */
/*************************************************************************** 
*
* Description: Some nice descriptive description. 
* 
* Author:   Jakob Buron 
* 
* Last Changed By:  $Author: jdo $ 
* Revision:         $Revision: 1.38 $ 
* Last Changed:     $Date: 2005/07/27 15:12:54 $ 
* 
****************************************************************************/
#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdint.h>
#include "ZW_typedefs.h"
#ifndef EFR32ZG
#include "assert.h"
#endif

#ifdef __C51__
#ifdef DEBUG
#define ZW_DEBUG
#endif
#include <ZW_uart_api.h>


void print16(uint8_t * pData);

#define UNUSED(x) x = x; /* Hack to silence warning C280 Unreferenced local variable */

///* No printf on 8051 target. Comment out. This breaks includes in various modules. Use only in .c files */
//#define SLASH /
//#define printf  SLASH/

#else /* __C51__ */
#ifdef EFR32ZG
#include "Assert.h"
#else
#include "assert.h"
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define print16(x)
#define dbg_print_num(x) // only used in ZW050x target

#ifndef ZW_DEBUG_SEND_BYTE
#define ZW_DEBUG_SEND_BYTE(x)
#define ZW_DEBUG_SEND_NUM(x)
#define ZW_DEBUG_SEND_WORD_NUM(x)
#define ZW_DEBUG_SEND_WNUM
#define ZW_DEBUG_SEND_STR(x)
#define ZW_DEBUG_SEND_NL(x)
#endif

#ifdef DEBUG
#define dbg_printf printf
#endif
#endif /* __C51__ */


#endif /* PLATFORM_H_ */
