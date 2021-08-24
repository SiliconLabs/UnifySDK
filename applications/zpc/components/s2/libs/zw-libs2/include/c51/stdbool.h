/* © 2017 Silicon Laboratories Inc.
 */
/*
 * stdbool.h for the C51 platform.
 *
 *  Created on: Oct 29, 2015
 *      Author: trasmussen
 */

#ifndef _C51_STDBOOL_H_
#define _C51_STDBOOL_H_

#ifndef __C51__
#error Using C51 specific stdbool.h on non C51 target.
#endif

#define _Bool unsigned char                       //< _Bool as specified in ISO/IEC 9899:TC3p253.
                                                  /* On the C51 target there is a special bit type.
                                                   * But there is a very limitied numer of these, so it will not be used for the _Bool. */

#define bool  _Bool                     //< bool as specified in ISO/IEC 9899:TC3p253.

#ifndef true
#define true  1                         //< true as specified in ISO/IEC 9899:TC3p253.
#endif

#ifndef false
#define false 0                         //< false as specified in ISO/IEC 9899:TC3p253.
#endif

#define __bool_true_false_are_defined 1 //< __bool_true_false_are_defined as specified in ISO/IEC 9899:TC3p253.

#endif // _C51_STDBOOL_H_
