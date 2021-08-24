/* © 2018 Silicon Laboratories Inc.
 */
/*******************************  ZW_typedefs.h  *******************************
 *           #######
 *           ##  ##
 *           #  ##    ####   #####    #####  ##  ##   #####
 *             ##    ##  ##  ##  ##  ##      ##  ##  ##
 *            ##  #  ######  ##  ##   ####   ##  ##   ####
 *           ##  ##  ##      ##  ##      ##   #####      ##
 *          #######   ####   ##  ##  #####       ##  #####
 *                                           #####
 *          Products that speak Z-Wave work together better
 *
 *              Copyright (c) 2008
 *              Zensys A/S
 *              Denmark
 *
 *              All Rights Reserved
 *
 *    This source file is subject to the terms and conditions of the
 *    Zensys Software License Agreement which restricts the manner
 *    in which it may be used.
 *
 *---------------------------------------------------------------------------
 *
 * Description: Module description
 *
 * Author:   Ivar Jeppesen
 *
 * Last Changed By:  $Author: efh $
 * Revision:         $Revision: 29359 $
 * Last Changed:     $Date: 2014-07-11 11:13:33 +0200 (fr, 11 jul 2014) $
 *
 ****************************************************************************/
#ifndef _ZW_TYPEDEFS_H_
#define _ZW_TYPEDEFS_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/****************************************************************************/
/*                                 MACROS                                   */
/****************************************************************************/
/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/
#define code
#define xdata

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef bool   BOOL;

#define CODE           /* Used for defining callback function which allways */
                           /* resides in code space. */

#define TRUE  true
#define FALSE false

/* Define for making easy and consistent callback definitions */
#define VOID_CALLBACKFUNC(completedFunc)  void (CODE *completedFunc)


#define UNUSED(x) x = x; /* Hack to silence warning C280 Unreferenced local variable */


#endif /* _ZW_TYPEDEFS_H_ */
