;/****************************************************************************
; *
; * Copyright (c) 2001-2015
; * Sigma Designs, Inc.
; * All Rights Reserved
; *
; *---------------------------------------------------------------------------
; *
; * Description: Intel 8051 assembly language optimized version of "bigint"
; *              functions for calculating mul, add, and sub of 256 bits integers.
; *
; *              This work is based upon the below work:
; *
;/*
; * File:    avrnacl_8bitc/shared/bigint.c
; * Author:  Michael Hutter, Peter Schwabe
; * Version: Tue Aug 12 08:23:16 2014 +0200
; * Public Domain
; */
; *
; * Author:   Erik Friis Harck
; *
; * Last Changed By:  $Author: iza $
; * Revision:         $Revision: 22797 $
; * Last Changed:     $Date: 2012-05-10 15:55:06 +0200 (to, 10 maj 2012) $
; *
; ****************************************************************************/

; bigint.src generated from: BIGINT.C
; COMPILER INVOKED BY:
;        C:\KEIL\C51\bin\C51.exe BIGINT.C WARNINGLEVEL(2) DEBUG OBJECTADVANCED OBJECTEXTEND LARGE CODE LISTINCLUDE NOINTPROMOTE NOCOND INTVECTOR(0X1800) OPTIMIZE(11,SIZE) INCDIR(E:\code\mantra\trunk\Z-Wave\include,E:\code\mantra\trunk\Z-Wave\IO_defines,.\build,.\build\WallController_slave_enhanced_232_ZW050x_EU\Rels,C:\KEIL\C51\inc,..\ApplicationUtilities,..\ApplicationCommandHandlers,..\..\zlib\cmdclass,..\..\zlib\devtypes,..\..\zlib\handler,E:\code\mantra\trunk\Z-Wave\\IO_defines) SYMBOLS DF(ZW_DEBUG,ZW_DEBUG_APP,EU,ZW_SLAVE,ZW_SLAVE_32,ZW_SLAVE_ENHANCED_232,ZW_SLAVE_ROUTING,ZW050x,ZW0501,NEW_NODEINFO,ZW_SELF_HEAL,BANKING,ENDPOINT_SUPPORT,MULTI_CHANNEL_TRANSPORT,NON_BATT,bigint,slave_enhanced_232) PR(.\build\WallController_slave_enhanced_232_ZW050x_EU\list\bigint.lst) OJ(.\build\WallController_slave_enhanced_232_ZW050x_EU\Rels\bigint.obj) SRC(bigint.src)

$CA

NAME  BIGINT

?PR?_bigint_add?BIGINT                   SEGMENT CODE 
?ID?_bigint_add?BIGINT                   SEGMENT IDATA OVERLAYABLE
?XD?_bigint_add?BIGINT                   SEGMENT XDATA OVERLAYABLE
?PR?_bigint_sub?BIGINT                   SEGMENT CODE 
?XD?_bigint_sub?BIGINT                   SEGMENT XDATA OVERLAYABLE
?PR?_bigint_mul?BIGINT                   SEGMENT CODE 
?XD?_bigint_mul?BIGINT                   SEGMENT XDATA OVERLAYABLE
?DT?_bigint_mul?BIGINT                   SEGMENT DATA OVERLAYABLE
?PR?_bigint_cmov?BIGINT                  SEGMENT CODE 
?XD?_bigint_cmov?BIGINT                  SEGMENT XDATA OVERLAYABLE
  PUBLIC  ?_bigint_cmov?BYTE
  PUBLIC  _bigint_cmov
  PUBLIC  _bigint_mul32
  PUBLIC  ?_bigint_mul?BYTE
  PUBLIC  _bigint_mul
  PUBLIC  ?_bigint_sub?BYTE
  PUBLIC  _bigint_sub
  PUBLIC  ?_bigint_add?BYTE
  PUBLIC  _bigint_add

  RSEG  ?ID?_bigint_add?BIGINT
; Overlaying local variables bigint_add_a, bigint_sub_a, and bigint_mul_a arrays (this is not nice)
; But it will do, as long as these functions do not call each other.
bigint_add_a:
bigint_sub_a:
bigint_mul_a:
bigint_cmov_x:     DS   32

  RSEG  ?DT?_bigint_mul?BIGINT
; Overlaying local variables bigint_add_b, bigint_sub_b, and bigint_mul_b arrays (this is not nice)
; But it will do, as long as these functions do not call each other.
bigint_add_b:
bigint_sub_b:
bigint_mul_b:      DS   32

  RSEG  ?XD?_bigint_add?BIGINT
?_bigint_add?BYTE:
          r?040:   DS   2
  ORG  2
          a?041:   DS   2
  ORG  4
          b?042:   DS   2
  ORG  6
        len?043:   DS   1
  ORG  7
;---- Variable 'tmp?045' assigned to Register 'R5,R6' ----
        tmp?045:   DS   2

  RSEG  ?XD?_bigint_cmov?BIGINT
?_bigint_cmov?BYTE:
;---- Parameter 'r?683' assigned to Register 'R6,R7' ----
          r?683:   DS   2
  ORG  2
;---- Parameter 'x?684' assigned to Register 'R4,R5' ----
          x?684:   DS   2
;---- Parameter 'b?685' assigned to Register 'R3' ----
          b?685:   DS   1
  ORG  5
        len?686:   DS   1
  ORG  6
          i?687:   DS   1
  ORG  7
       mask?688:   DS   1

  RSEG  ?XD?_bigint_sub?BIGINT
?_bigint_sub?BYTE:
          r?146:   DS   2
  ORG  2
          a?147:   DS   2
  ORG  4
          b?148:   DS   2
  ORG  6
        len?149:   DS   1
  ORG  7
;---- Variable 'tmp?151' assigned to Register 'R5,R6' ----
        tmp?151:   DS   2

  RSEG  ?XD?_bigint_mul?BIGINT
?_bigint_mul?BYTE:
;---- Parameter 'r?252' assigned to Register 'R6,R7' ----
          r?252:   DS   2
  ORG  2
;---- Parameter 'a?253' assigned to Register 'R4,R5' ----
          a?253:   DS   2
  ORG  4
;---- Parameter 'b?254' assigned to Register 'R2,R3' ----
          b?254:   DS   2
  ORG  6
        len?255:   DS   1
  ORG  7
          j?257:   DS   1
  ORG  8
          t?258:   DS   2

; #pragma optimize (8, SPEED)
; 
; /*
;  * File:    avrnacl_8bitc/shared/bigint.c
;  * Author:  Michael Hutter, Peter Schwabe
;  * Version: Tue Aug 12 08:23:16 2014 +0200
;  * Public Domain
;  */
; 
; #ifdef __C51__
; #include <ZW_stdint.h>
; #include <ZW_uart_api.h>
; #else
; #include <stdint.h>
; #endif
; #include "avrnacl.h"
; #include "bigint.h"
; #include "wc_util.h"
; 
; uint8_t bigint_add(uint8_t xdata *r, const uint8_t xdata *a, const uint8_t xdata *b, uint8_t len)

  RSEG  ?PR?_bigint_add?BIGINT
_bigint_add:
  USING 0
  MOV   DPTR,#r?040
  MOV   A,R6
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R7
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R4
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R5
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R2
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R3
  MOVX  @DPTR,A
; R3 is really not used for low part of *b parameter
  MOV   DPTR,#len?043
  MOVX  A,@DPTR
  MOV   R3,A            ; Use R3 for len?043 instead
; {
  MOV   DPTR,#a?041
  MOVX  A,@DPTR
  MOV   R2,A            ; R2 = HIGH a
  INC   DPTR
  MOVX  A,@DPTR
  MOV   DPL,A
  MOV   A,R2
  MOV   DPH,A
  MOV   R0,#bigint_add_a
  MOV   A,R3            ; len?043
  MOV   R2,A            ; R2 = len
?bigint_add_a_copy:
  MOVX  A,@DPTR         ; A = *a++
  INC   DPTR
  MOV   @R0,A           ; *bigint_add_a++ = A
  INC   R0
  DJNZ  R2,?bigint_add_a_copy
  MOV   DPTR,#b?042
  MOVX  A,@DPTR
  MOV   R2,A            ; R2 = HIGH b
  INC   DPTR
  MOVX  A,@DPTR
  MOV   DPL,A
  MOV   A,R2
  MOV   DPH,A
  MOV   R0,#bigint_add_b
  MOV   A,R3            ; len?043
  MOV   R2,A            ; R2 = len
?bigint_add_b_copy:
  MOVX  A,@DPTR         ; A = *b++
  INC   DPTR
  MOV   @R0,A           ; *bigint_add_b++ = A
  INC   R0
  DJNZ  R2,?bigint_add_b_copy
;   uint8_t i;
;   crypto_uint16 tmp = 0;
  MOV   DPTR,#r?040
  MOVX  A,@DPTR
  MOV   R2,A            ; R2 = HIGH r
  INC   DPTR
  MOVX  A,@DPTR
  MOV   DPL,A
  MOV   A,R2            ; A = HIGH r
  MOV   DPH,A
;---- Variable 'tmp?045' assigned to Register 'R5,R6' ----
  CLR   C               ; Start do..while loop with no carry.
  MOV   R0,#bigint_add_a
  MOV   R1,#bigint_add_b
;   for (i=0; i<len; i++)
?C0001:
;   {
;     tmp = ((uint16_t)a[i]) + ((uint16_t)b[i]) + tmp;
  MOV   A,@R1           ; A = b[i]
  INC   R1
  MOV   R7,A            ; R7 = b[i]
  MOV   A,@R0           ; A = a[i]
  INC   R0
  ADDC  A,R7            ; A = a[i] + b[i]
;     r[i] = tmp & 0xff;
  MOVX  @DPTR,A         ; r[i] = LOW (tmp)
;     tmp >>= 8;
  INC   DPTR
;   }
  DJNZ  R3,?C0001
;   return (uint8_t)tmp;
  CLR   A
  RLC   A
  MOV   R7,A            ; return carry from MSB addition
; }
  RET   
; END OF _bigint_add

; 
; uint8_t bigint_sub(uint8_t xdata *r, const uint8_t xdata *a, const uint8_t xdata *b, uint8_t len)

  RSEG  ?PR?_bigint_sub?BIGINT
_bigint_sub:
  USING 0
  MOV   DPTR,#r?146
  MOV   A,R6
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R7
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R4
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R5
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R2
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R3
  MOVX  @DPTR,A
; R3 is really not used for low part of *b parameter
  MOV   DPTR,#len?149
  MOVX  A,@DPTR
  MOV   R3,A            ; Use R3 for len?149 instead
; {
  MOV   DPTR,#a?147
  MOVX  A,@DPTR
  MOV   R2,A            ; R2 = HIGH a
  INC   DPTR
  MOVX  A,@DPTR
  MOV   DPL,A
  MOV   A,R2
  MOV   DPH,A
  MOV   R0,#bigint_sub_a
  MOV   A,R3            ; len?149
  MOV   R2,A            ; R2 = len
?bigint_sub_a_copy:
  MOVX  A,@DPTR         ; A = *a++
  INC   DPTR
  MOV   @R0,A           ; *bigint_sub_a++ = A
  INC   R0
  DJNZ  R2,?bigint_sub_a_copy
  MOV   DPTR,#b?148
  MOVX  A,@DPTR
  MOV   R2,A            ; R2 = HIGH b
  INC   DPTR
  MOVX  A,@DPTR
  MOV   DPL,A
  MOV   A,R2
  MOV   DPH,A
  MOV   R0,#bigint_sub_b
  MOV   A,R3            ; len?149
  MOV   R2,A            ; R2 = len
?bigint_sub_b_copy:
  MOVX  A,@DPTR         ; A = *b++
  INC   DPTR
  MOV   @R0,A           ; *bigint_sub_b++ = A
  INC   R0
  DJNZ  R2,?bigint_sub_b_copy
;   uint8_t i;
;   crypto_uint16 tmp = 0;
  MOV   DPTR,#r?146
  MOVX  A,@DPTR
  MOV   R2,A            ; R2 = HIGH r
  INC   DPTR
  MOVX  A,@DPTR
  MOV   DPL,A
  MOV   A,R2            ; A = HIGH r
  MOV   DPH,A
  CLR   C               ; Start do..while loop with no carry.
  MOV   R0,#bigint_sub_a
  MOV   R1,#bigint_sub_b
;   for (i=0; i<len; i++) 
?C0005:
;   {
;     tmp = ((uint16_t)a[i]) - ((uint16_t)b[i]) - tmp;
  MOV   A,@R1           ; A = b[i]
  INC   R1
  MOV   R7,A            ; R7 = b[i]
  MOV   A,@R0           ; A = a[i]
  INC   R0
  SUBB  A,R7            ; A = a[i] - b[i]
;     r[i] = tmp & 0xff;
  MOVX  @DPTR,A         ; r[i] = LOW (tmp)
;     tmp >>= 15;
  INC   DPTR
;   }
  DJNZ  R3,?C0005
;   return (uint8_t)tmp;
  CLR   A
  RLC   A
  MOV   R7,A            ; return carry from MSB subtraction
; }
  RET   
; END OF _bigint_sub

; 
; void bigint_mul32(uint8_t xdata *r, const uint8_t xdata *a, const uint8_t xdata *b)

  RSEG  ?PR?_bigint_mul?BIGINT
_bigint_mul32:
  USING	0
;---- Variable 'r?575' assigned to Register 'R6/R7' ----
;---- Variable 'b?577' assigned to Register 'R2/R3' ----
;---- Variable 'a?576' assigned to Register 'R4/R5' ----
; {
;   bigint_mul(r, a, b, 32);
  MOV   DPTR,#?_bigint_mul?BYTE+06H
  MOV   A,#020H
  MOVX  @DPTR,A
;  Drop through to  _bigint_mul
; END OF _bigint_mul32

; }

; 
; void bigint_mul(unsigned char xdata *r, const unsigned char xdata *a, const unsigned char xdata *b, uint8_t len)

_bigint_mul:
;---- Parameter 'r?252' assigned to Register 'R6,R7' ----
  MOV   DPTR,#r?252
  MOV   A,R6
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R7
  MOVX  @DPTR,A
  INC   DPTR
;---- Parameter 'a?253' assigned to Register 'R4,R5' ----
  MOV   A,R4
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R5
  MOVX  @DPTR,A
  INC   DPTR
;---- Parameter 'b?254' assigned to Register 'R2,R3' ----
  MOV   A,R2
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R3
  MOVX  @DPTR,A
; R3 is really not used for low part of *b parameter
  MOV   DPTR,#len?255
  MOVX  A,@DPTR
  MOV   R3,A            ; Use R3 for len?255 instead
; {
  MOV   DPTR,#a?253
  MOVX  A,@DPTR
  MOV   R1,A            ; R1 = HIGH a
  INC   DPTR
  MOVX  A,@DPTR
  MOV   DPL,A
  MOV   A,R1
  MOV   DPH,A
  MOV   R0,#bigint_mul_a
  MOV   A,R3            ; len?255
  MOV   R1,A            ; R1 = len
?bigint_mul_a_copy:
  MOVX  A,@DPTR         ; A = *a++
  INC   DPTR
  MOV   @R0,A           ; *bigint_mul_a++ = A
  INC   R0
  DJNZ  R1,?bigint_mul_a_copy
  MOV   DPTR,#b?254
  MOVX  A,@DPTR
  MOV   R1,A            ; R1 = HIGH b
  INC   DPTR
  MOVX  A,@DPTR         ; A = LOW b
  MOV   DPL,A
  MOV   A,R1            ;
  MOV   DPH,A
  MOV   R0,#bigint_mul_b
  MOV   A,R3            ; len?255
  MOV   R1,A            ; R1 = len
?bigint_mul_b_copy:
  MOVX  A,@DPTR         ; A = *a++
  INC   DPTR
  MOV   @R0,A           ; *bigint_mul_b++ = A
  INC   R0
  DJNZ  R1,?bigint_mul_b_copy
;   uint8_t i;
;   uint8_t j;
;   uint16_t t;
;   for(i=0;i<2*len;i++)
  MOV   A,R3            ; len?255
  ADD   A,ACC
  MOV   R1,A
;     r[i] = 0;
  MOV   DPTR,#r?252
  MOVX  A,@DPTR
  MOV   R7,A            ; R7 = HIGH (r?252)
  INC   DPTR
  MOVX  A,@DPTR
  MOV   R2,A            ; R2 = LOW (r?252)
  MOV   DPL,A
  MOV   A,R7            ;
  MOV   DPH,A
  CLR   A
?C0009:
  MOVX  @DPTR,A
  INC   DPTR
  DJNZ  R1,?C0009
;   DPTR = r?252;
  MOV   A,R2            ;
  MOV   DPL,A           ; DPL = LOW (r?252)
  MOV   A,R7            ;
  MOV   DPH,A           ; DPH = HIGH (r?252)
; 
;   for (i = 0; i < len; i++)
;---- idata * variable 'bigint_mul_a' assigned to Register 'R1' ----
  MOV   A,R3            ; len?255
  MOV   R6,A
  MOV   R1,#bigint_mul_a
?C0012:
;   {
;     t = 0;
  CLR   A
;   t?258 = 0
;---- Variable 't?258' assigned to Register 'R2' ----
;     both HIGH (t?258) and LOW (t?258), but not at the same time.
  MOV   R2,A            ; t?258 = 0
;     for (j = 0; j < len; j++)
  MOV   A,R3            ; len?255
  MOV   R4,A
;---- data * variable 'bigint_mul_b' assigned to Register 'R0' ----
  MOV   R0,#bigint_mul_b
  PUSH  DPH
  PUSH  DPL
?C0015:
;     {
;       t = r[i + j] + ((uint16_t)(a[i] * b[j])) + (t >> 8);
  MOV   A,@R0           ; A = *bigint_mul_b = b[j]
  MOV   B,A             ; B = b[j]
  MOV   A,@R1           ; A = *bigint_mul_a = a[i]
  MUL   AB
  MOV   R7,A
  MOVX  A,@DPTR         ; A = r[i + j]
  ADD   A,R7
  MOV   R5,A            ; R5 = r[i + j] + ((uint16_t)(a[i] * b[j]))
  CLR   A
  ADDC  A,B
  MOV   R7,A
  MOV   A,R2            ; A = t?258
  ADD   A,R5
  MOVX  @DPTR,A         ; r[i + j] =  r[i + j] + ((uint16_t)(a[i] * b[j])) + (t >> 8)    ; (t & 0xFF)
  CLR   A               ;
  ADDC  A,R7            ; add multiplication carry
  MOV   R2,A            ; R2 = t?258
;       r[i + j] = (t & 0xFF);
  INC   DPTR            ; DPTR = (r + i + j)++
;     }
  INC   R0              ; bigint_mul_b++
  DJNZ  R4,?C0015
;     r[i + len] = (t >> 8);
  MOV   A,R2            ; A = t?258
  MOVX  @DPTR,A         ; r[i + len] = (t >> 8)
  POP   DPL
  POP   DPH
  INC   DPTR            ; DPTR = (r + i + j)++
;   }
  INC   R1              ; bigint_mul_a++
  DJNZ  R6,?C0012
; }
  RET   
; END OF _bigint_mul

; 
; void bigint_cmov(uint8_t xdata *r, const uint8_t xdata *x, uint8_t b, uint8_t len)

  RSEG  ?PR?_bigint_cmov?BIGINT
_bigint_cmov:
  USING 0
;---- Parameter 'r?683' assigned to Register 'R6,R7' ----
  MOV   DPTR,#r?683
  MOV   A,R6
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R7
;---- Parameter 'x?684' assigned to Register 'R4,R5' ----
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R4
  MOVX  @DPTR,A
  INC   DPTR
  MOV   A,R5
  MOVX  @DPTR,A
; R5 is really not used for low part of *x parameter
  MOV   DPTR,#len?686
  MOVX  A,@DPTR
  MOV   R5,A            ; Use R5 for len?686 instead
;---- Parameter 'b?685' assigned to Register 'R3' ----
; {
  MOV   DPTR,#x?684
  MOVX  A,@DPTR
  MOV   R2,A            ; R2 = HIGH x
  INC   DPTR
  MOVX  A,@DPTR
  MOV   DPL,A
  MOV   A,R2
  MOV   DPH,A
  MOV   R0,#bigint_cmov_x
  MOV   A,R5            ; len?686
  MOV   R2,A            ; R2 = len
?bigint_cmov_x_copy:
  MOVX  A,@DPTR         ; A = *x++
  INC   DPTR
  MOV   @R0,A           ; *bigint_cmov_x++ = A
  INC   R0
  DJNZ  R2,?bigint_cmov_x_copy
;   uint8_t i;
;   uint8_t mask = b;
;---- Variable 'mask?688' assigned to Register 'R3' ----
;   mask = -mask;
  MOV   A,R3            ; A = mask = b
  CPL   A
  INC   A
  MOV   R3,A            ; R3 = -mask
;   for(i=0;i<len;i++) 
  MOV   DPTR,#r?683     ; DPTR = r
  MOVX  A,@DPTR
  MOV   R4,A
  INC   DPTR
  MOVX  A,@DPTR
  MOV   DPL,A
  MOV   A,R4
  MOV   DPH,A
  MOV   R0,#bigint_cmov_x
?C0031:
;     r[i] ^= mask & (x[i] ^ r[i]);
  MOVX  A,@DPTR
  MOV   R7,A
  MOV   A,@R0           ;
  INC   R0              ;
  XRL   A,R7
  MOV   R6,A
  MOV   A,R3            ;
  ANL   A,R6
  MOV   R6,A
  MOV   A,R7
  XRL   A,R6
  MOVX  @DPTR,A
  INC   DPTR            ;
  DJNZ  R5,?C0031       ;
; }
  RET   
; END OF _bigint_cmov

  END
