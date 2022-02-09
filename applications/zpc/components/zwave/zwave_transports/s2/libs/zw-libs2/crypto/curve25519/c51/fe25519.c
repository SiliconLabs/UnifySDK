#pragma optimize (8, SPEED)

/*
 * File:    avrnacl_8bitc/shared/fe25519.c
 * Author:  Michael Hutter, Peter Schwabe
 * Version: Fri Aug 1 09:07:46 2014 +0200
 * Public Domain
 */

/* © 2014 Sigma Designs, Inc. This is an unpublished work protected by Sigma
 * Designs, Inc. as a trade secret, and is not to be used or disclosed except as
 * provided Z-Wave Controller Development Kit Limited License Agreement. All
 * rights reserved.
 *
 * Notice: All information contained herein is confidential and/or proprietary to
 * Sigma Designs and may be covered by U.S. and Foreign Patents, patents in
 * process, and are protected by trade secret or copyright law. Dissemination or
 * reproduction of the source code contained herein is expressly forbidden to
 * anyone except Licensees of Sigma Designs  who have executed a Sigma Designs’
 * Z-WAVE CONTROLLER DEVELOPMENT KIT LIMITED LICENSE AGREEMENT. The copyright
 * notice above is not evidence of any actual or intended publication of the
 * source code. THE RECEIPT OR POSSESSION OF  THIS SOURCE CODE AND/OR RELATED
 * INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS  TO REPRODUCE, DISCLOSE OR
 * DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE, USE, OR SELL A PRODUCT THAT IT  MAY
 * DESCRIBE.
 *
 * THE SIGMA PROGRAM AND ANY RELATED DOCUMENTATION OR TOOLS IS PROVIDED TO COMPANY
 * "AS IS" AND "WITH ALL FAULTS", WITHOUT WARRANTY OF ANY KIND FROM SIGMA. COMPANY
 * ASSUMES ALL RISKS THAT LICENSED MATERIALS ARE SUITABLE OR ACCURATE FOR
 * COMPANY’S NEEDS AND COMPANY’S USE OF THE SIGMA PROGRAM IS AT COMPANY’S
 * OWN DISCRETION AND RISK. SIGMA DOES NOT GUARANTEE THAT THE USE OF THE SIGMA
 * PROGRAM IN A THIRD PARTY SERVICE ENVIRONMENT OR CLOUD SERVICES ENVIRONMENT WILL
 * BE: (A) PERFORMED ERROR-FREE OR UNINTERRUPTED; (B) THAT SIGMA WILL CORRECT ANY
 * THIRD PARTY SERVICE ENVIRONMENT OR CLOUD SERVICE ENVIRONMENT ERRORS; (C) THE
 * THIRD PARTY SERVICE ENVIRONMENT OR CLOUD SERVICE ENVIRONMENT WILL OPERATE IN
 * COMBINATION WITH COMPANY’S CONTENT OR COMPANY APPLICATIONS THAT UTILIZE THE
 * SIGMA PROGRAM; (D) OR WITH ANY OTHER HARDWARE, SOFTWARE, SYSTEMS, SERVICES OR
 * DATA NOT PROVIDED BY SIGMA. COMPANY ACKNOWLEDGES THAT SIGMA DOES NOT CONTROL
 * THE TRANSFER OF DATA OVER COMMUNICATIONS FACILITIES, INCLUDING THE INTERNET,
 * AND THAT THE SERVICES MAY BE SUBJECT TO LIMITATIONS, DELAYS, AND OTHER PROBLEMS
 * INHERENT IN THE USE OF SUCH COMMUNICATIONS FACILITIES. SIGMA IS NOT RESPONSIBLE
 * FOR ANY DELAYS, DELIVERY FAILURES, OR OTHER DAMAGE RESULTING FROM SUCH ISSUES.
 * SIGMA IS NOT RESPONSIBLE FOR ANY ISSUES RELATED TO THE PERFORMANCE, OPERATION
 * OR SECURITY OF THE THIRD PARTY SERVICE ENVIRONMENT OR CLOUD SERVICES
 * ENVIRONMENT THAT ARISE FROM COMPANY CONTENT, COMPANY APPLICATIONS OR THIRD
 * PARTY CONTENT. SIGMA DOES NOT MAKE ANY REPRESENTATION OR WARRANTY REGARDING THE
 * RELIABILITY, ACCURACY, COMPLETENESS, CORRECTNESS, OR USEFULNESS OF THIRD PARTY
 * CONTENT OR SERVICE OR THE SIGMA PROGRAM, AND DISCLAIMS ALL LIABILITIES ARISING
 * FROM OR RELATED TO THE SIGMA PROGRAM OR THIRD PARTY CONTENT OR SERVICES. TO THE
 * EXTENT NOT PROHIBITED BY LAW, THESE WARRANTIES ARE EXCLUSIVE. SIGMA OFFERS NO
 * WARRANTY OF NON-INFRINGEMENT, TITLE, OR QUIET ENJOYMENT. NEITHER SIGMA NOR ITS
 * SUPPLIERS OR LICENSORS SHALL BE LIABLE FOR ANY INDIRECT, SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES OR LOSS (INCLUDING DAMAGES FOR LOSS OF BUSINESS, LOSS OF
 * PROFITS, OR THE LIKE), ARISING OUT OF THIS AGREEMENT WHETHER BASED ON BREACH OF
 * CONTRACT, INTELLECTUAL PROPERTY INFRINGEMENT, TORT (INCLUDING NEGLIGENCE),
 * STRICT LIABILITY, PRODUCT LIABILITY OR OTHERWISE, EVEN IF SIGMA OR ITS
 * REPRESENTATIVES HAVE BEEN ADVISED OF OR OTHERWISE SHOULD KNOW ABOUT THE
 * POSSIBILITY OF SUCH DAMAGES. THERE ARE NO OTHER EXPRESS OR IMPLIED WARRANTIES
 * OR CONDITIONS INCLUDING FOR SOFTWARE, HARDWARE, SYSTEMS, NETWORKS OR
 * ENVIRONMENTS OR FOR MERCHANTABILITY, NONINFRINGEMENT, SATISFACTORY QUALITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * The Sigma Program  is not fault-tolerant and is not designed, manufactured or
 * intended for use or resale as on-line control equipment in hazardous
 * environments requiring fail-safe performance, such as in the operation of
 * nuclear facilities, aircraft navigation or communication systems, air traffic
 * control, direct life support machines, or weapons systems, in which the failure
 * of the Sigma Program, or Company Applications created using the Sigma Program,
 * could lead directly to death, personal injury, or severe physical or
 * environmental damage ("High Risk Activities").  Sigma and its suppliers
 * specifically disclaim any express or implied warranty of fitness for High Risk
 * Activities.Without limiting Sigma’s obligation of confidentiality as further
 * described in the Z-Wave Controller Development Kit Limited License Agreement,
 * Sigma has no obligation to establish and maintain a data privacy and
 * information security program with regard to Company’s use of any Third Party
 * Service Environment or Cloud Service Environment. For the avoidance of doubt,
 * Sigma shall not be responsible for physical, technical, security,
 * administrative, and/or organizational safeguards that are designed to ensure
 * the security and confidentiality of the Company Content or Company Application
 * in any Third Party Service Environment or Cloud Service Environment that
 * Company chooses to utilize.
 */
#ifdef ZW050x
/* Put XDATA in XDATA_LOWER because upper XDATA is full on some targets */
#pragma userclass (xdata = LOWER)
#endif


#ifdef __AVR__
  #include <avr/pgmspace.h>
#endif

#include <stdint.h>

#include "bigint.h"
#include "fe25519.h"
//#include "wc_util.h"

#include <ZW_basis_api.h>

//extern uint8_t debug_flag;

/******************************************************************/
/*                Static constants and functions                  */
/******************************************************************/

/* m = 2^255-19 = 7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed */
static const uint8_t ECCParam_p[32] = {0xED, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F};



#ifdef NOT_USED
#if 0
/* If you want to test if two 8bit values are equal, please just do that... */
/* Actually this should be inlined... */
static uint8_t equal(uint8_t a, uint8_t b) /* 8-bit inputs */
{
  return (0 == (a ^ b));
}
#else
static uint16_t equal(uint16_t a,uint16_t b) /* 8-bit inputs */
{
  uint32_t x = a ^ b; /* 0: yes; 1..255: no */
  x -= 1; /* 65535: yes; 0..254: no */
  x >>= 16; /* 1: yes; 0: no */
  return x;
}
#endif
#endif

/******************************************************************/
/*                Public constants and functions                  */
/******************************************************************/

void fe25519_add(fe25519_t xdata *r, const fe25519_t xdata *a, const fe25519_t xdata *b)
{
  uint8_t carry=0;
  uint16_t tmp;
  uint8_t i;

  carry = bigint_add(r->value,a->value,b->value, 32);
  tmp = ((carry << 1) | (r->value[31] >> 7)) * 19;
  for(i=0;i<31;i++)
  {
    tmp = (uint16_t) r->value[i] + tmp;
    r->value[i] = tmp & 0xff;
    tmp >>= 8;
  }
  r->value[i] = (r->value[i] & 0x7f) + tmp;
// Does not give very much...  ((uint8_t *)r)[i] = (((uint8_t *)r)[i] & 0x7f) + tmp;
}

void fe25519_sub(fe25519_t xdata *r, const fe25519_t xdata *a, const fe25519_t xdata *b)
{
  uint8_t borrow;
  uint16_t tmp;
  uint8_t i;

  borrow = bigint_sub(r->value,a->value,b->value, 32);

  tmp = borrow * 38;
  for (i=0; i<32; i++)
  {
    tmp = (uint16_t) r->value[i] - tmp;
    r->value[i] = tmp & 0xff;
    tmp >>= 15;
  }
//  tmp *= 38;
// Does not give anything...  tmp = (uint8_t)tmp * 38;
  tmp = (uint8_t)tmp * 38;
  for (i=0; i<3; i++)
  {
    tmp = (uint16_t) r->value[i] - tmp;
    r->value[i] = tmp & 0xff;
    tmp >>= 15;
  }
}

void fe25519_mul(fe25519_t xdata *r, const fe25519_t xdata *a, const fe25519_t xdata *b)
{
  uint8_t t[64];
  uint16_t UV=0;
  uint8_t i;

  ZW_WatchDogKick();
  /* multiplication */
  bigint_mul32(t, a->value, b->value);

//print64(t);

//  if (1 == debug_flag)
  {
//    print64(t);
  }

  /* reduction */
  for (i=0; i<32; i++)
  {
    //UV = (t[i+32] << 5) + (t[i+32] << 2) + (t[i+32] << 1) + (UV >> 8) + t[i];
	  UV = (((uint16_t)t[i+32]) << 5) + (((uint16_t)t[i+32]) << 2) + (((uint16_t)t[i+32]) << 1) + (UV >> 8) + t[i];
  	r->value[i] = (UV & 0xFF);
  }
  UV >>= 8;
//  UV *= 38;
  UV = (uint8_t)UV * 38;
  for (i=0; i<32; i++)
  {
	  UV = UV + r->value[i];
  	r->value[i] = (UV & 0xFF);
    UV >>= 8;
  }
//  UV *= 38;
  UV = (uint8_t)UV * 38;
  for (i=0; i<3; i++) /* XXX: 3 should be enough here, is 2 enough? */
  {
	  UV = UV + r->value[i];
  	r->value[i] = (UV & 0xFF);
    UV >>= 8;
  }

  //if (1 == debug_flag)
  {
//    print64(r->value);
  }
}

void fe25519_square(fe25519_t xdata *r, const fe25519_t xdata *x)
{
  fe25519_mul(r,x,x);
}

void fe25519_setzero(fe25519_t xdata *r)
{
  uint8_t i;
  for(i=0;i<32;i++)
    r->value[i]=0;
}

void fe25519_setone(fe25519_t xdata *r)
{
  uint8_t i;
  r->value[0] = 1;
  for(i=1;i<32;i++)
    r->value[i]=0;
}

#ifdef NOT_USED
unsigned char fe25519_getparity(const fe25519_t xdata *x)
{
  fe25519_t t;
  t = *x;
  fe25519_freeze(&t);
  return t.value[0] & 1;
}

int fe25519_iszero(const fe25519_t xdata *x)
{
  uint8_t i;
  uint8_t r = 0;
  fe25519_t t;
  t = *x;
  fe25519_freeze(&t);
  for(i=1;i<32;i++)
    r |= t.value[i];
  return equal(r,0);
}


int fe25519_iseq_vartime(const fe25519_t xdata *x, const fe25519_t xdata *y)
{
  fe25519_t t1;
  fe25519_t t2;
  uint8_t i;
  t1 = *x;
  t2 = *y;
  fe25519_freeze(&t1);
  fe25519_freeze(&t2);
  for(i=0;i<32;i++)
    if(t1.value[i] != t2.value[i]) return 0;
  return 1;
}


void fe25519_neg(fe25519_t xdata *r, const fe25519_t xdata *x)
{
  fe25519_t t;
  fe25519_setzero(&t);
  fe25519_sub(r, &t, x);
}
#endif

void fe25519_cmov(fe25519_t xdata *r, const fe25519_t xdata *x, uint8_t b)
{
  //print_txt("cmov before");
  //print32(x->value);
  bigint_cmov(r->value,x->value,b,32);
  //print_txt("cmov after");
  //print32(r->value);
}

void fe25519_unpack(fe25519_t xdata *r, const uint8_t xdata *x)
{
  uint8_t i;
  for(i=0;i<32;i++) r->value[i] = x[i];
  r->value[31] &= 127;
}

void fe25519_pack(uint8_t xdata *r, const fe25519_t xdata *x)
{
  uint8_t i;
  fe25519_t y;
  y = *x;
  fe25519_freeze(&y);
  for(i=0;i<32;i++)
    r[i] = y.value[i];
}

/* reduction modulo 2^255-19 */
void fe25519_freeze(fe25519_t xdata *r)
{
  uint8_t c;
  fe25519_t rt;
  c = bigint_sub(rt.value, r->value, ECCParam_p, 32);
  fe25519_cmov(r,&rt,1-c);
  c = bigint_sub(rt.value, r->value, ECCParam_p, 32);
  fe25519_cmov(r,&rt,1-c);
}

void fe25519_invert(fe25519_t xdata *r, const fe25519_t xdata *x)
{
	fe25519_t z2;
	fe25519_t z9;
	fe25519_t z11;
	fe25519_t z2_5_0;
	fe25519_t z2_10_0;
	fe25519_t z2_20_0;
	fe25519_t z2_50_0;
	fe25519_t z2_100_0;
	fe25519_t t0;
	fe25519_t t1;
	uint8_t i;

	/* 2 */ fe25519_square(&z2,x);
	/* 4 */ fe25519_square(&t1,&z2);
	/* 8 */ fe25519_square(&t0,&t1);

	/* 9 */ fe25519_mul(&z9,&t0,x);
	/* 11 */ fe25519_mul(&z11,&z9,&z2);
	/* 22 */ fe25519_square(&t0,&z11);
	/* 2^5 - 2^0 = 31 */ fe25519_mul(&z2_5_0,&t0,&z9);

	/* 2^6 - 2^1 */ fe25519_square(&t0,&z2_5_0);
	/* 2^7 - 2^2 */ fe25519_square(&t1,&t0);

	/* 2^8 - 2^3 */ fe25519_square(&t0,&t1);
	/* 2^9 - 2^4 */ fe25519_square(&t1,&t0);
	/* 2^10 - 2^5 */ fe25519_square(&t0,&t1);
	/* 2^10 - 2^0 */ fe25519_mul(&z2_10_0,&t0,&z2_5_0);


	/* 2^11 - 2^1 */ fe25519_square(&t0,&z2_10_0);
	/* 2^12 - 2^2 */ fe25519_square(&t1,&t0);
	/* 2^20 - 2^10 */ for (i = 2;i < 10;i += 2) { fe25519_square(&t0,&t1); fe25519_square(&t1,&t0); }
	/* 2^20 - 2^0 */ fe25519_mul(&z2_20_0,&t1,&z2_10_0);


	/* 2^21 - 2^1 */ fe25519_square(&t0,&z2_20_0);
	/* 2^22 - 2^2 */ fe25519_square(&t1,&t0);
	/* 2^40 - 2^20 */ for (i = 2;i < 20;i += 2) { fe25519_square(&t0,&t1); fe25519_square(&t1,&t0); }
	/* 2^40 - 2^0 */ fe25519_mul(&t0,&t1,&z2_20_0);


	/* 2^41 - 2^1 */ fe25519_square(&t1,&t0);
	/* 2^42 - 2^2 */ fe25519_square(&t0,&t1);
	/* 2^50 - 2^10 */ for (i = 2;i < 10;i += 2) { fe25519_square(&t1,&t0); fe25519_square(&t0,&t1); }
	/* 2^50 - 2^0 */ fe25519_mul(&z2_50_0,&t0,&z2_10_0);


	/* 2^51 - 2^1 */ fe25519_square(&t0,&z2_50_0);
	/* 2^52 - 2^2 */ fe25519_square(&t1,&t0);
	/* 2^100 - 2^50 */ for (i = 2;i < 50;i += 2) { fe25519_square(&t0,&t1); fe25519_square(&t1,&t0); }
	/* 2^100 - 2^0 */ fe25519_mul(&z2_100_0,&t1,&z2_50_0);


	/* 2^101 - 2^1 */ fe25519_square(&t1,&z2_100_0);
	/* 2^102 - 2^2 */ fe25519_square(&t0,&t1);
	/* 2^200 - 2^100 */ for (i = 2;i < 100;i += 2) { fe25519_square(&t1,&t0); fe25519_square(&t0,&t1); }
	/* 2^200 - 2^0 */ fe25519_mul(&t1,&t0,&z2_100_0);


	/* 2^201 - 2^1 */ fe25519_square(&t0,&t1);
	/* 2^202 - 2^2 */ fe25519_square(&t1,&t0);
	/* 2^250 - 2^50 */ for (i = 2;i < 50;i += 2) { fe25519_square(&t0,&t1); fe25519_square(&t1,&t0); }
	/* 2^250 - 2^0 */ fe25519_mul(&t0,&t1,&z2_50_0);


	/* 2^251 - 2^1 */ fe25519_square(&t1,&t0);
	/* 2^252 - 2^2 */ fe25519_square(&t0,&t1);
	/* 2^253 - 2^3 */ fe25519_square(&t1,&t0);

	/* 2^254 - 2^4 */ fe25519_square(&t0,&t1);
	/* 2^255 - 2^5 */ fe25519_square(&t1,&t0);
	/* 2^255 - 21 */ fe25519_mul(r,&t1,&z11);
}

#ifdef NOT_USED
void fe25519_pow2523(fe25519_t xdata *r, const fe25519_t xdata *x)
{
	fe25519_t z2;
	fe25519_t z9;
	fe25519_t z11;
	fe25519_t z2_5_0;
	fe25519_t z2_10_0;
	fe25519_t z2_20_0;
	fe25519_t z2_50_0;
	fe25519_t z2_100_0;
	fe25519_t t;
	uint8_t i;

	/* 2 */ fe25519_square(&z2,x);
	/* 4 */ fe25519_square(&t,&z2);
	/* 8 */ fe25519_square(&t,&t);
	/* 9 */ fe25519_mul(&z9,&t,x);
	/* 11 */ fe25519_mul(&z11,&z9,&z2);
	/* 22 */ fe25519_square(&t,&z11);
	/* 2^5 - 2^0 = 31 */ fe25519_mul(&z2_5_0,&t,&z9);

	/* 2^6 - 2^1 */ fe25519_square(&t,&z2_5_0);
	/* 2^10 - 2^5 */ for (i = 1;i < 5;i++) { fe25519_square(&t,&t); }
	/* 2^10 - 2^0 */ fe25519_mul(&z2_10_0,&t,&z2_5_0);

	/* 2^11 - 2^1 */ fe25519_square(&t,&z2_10_0);
	/* 2^20 - 2^10 */ for (i = 1;i < 10;i++) { fe25519_square(&t,&t); }
	/* 2^20 - 2^0 */ fe25519_mul(&z2_20_0,&t,&z2_10_0);

	/* 2^21 - 2^1 */ fe25519_square(&t,&z2_20_0);
	/* 2^40 - 2^20 */ for (i = 1;i < 20;i++) { fe25519_square(&t,&t); }
	/* 2^40 - 2^0 */ fe25519_mul(&t,&t,&z2_20_0);

	/* 2^41 - 2^1 */ fe25519_square(&t,&t);
	/* 2^50 - 2^10 */ for (i = 1;i < 10;i++) { fe25519_square(&t,&t); }
	/* 2^50 - 2^0 */ fe25519_mul(&z2_50_0,&t,&z2_10_0);

	/* 2^51 - 2^1 */ fe25519_square(&t,&z2_50_0);
	/* 2^100 - 2^50 */ for (i = 1;i < 50;i++) { fe25519_square(&t,&t); }
	/* 2^100 - 2^0 */ fe25519_mul(&z2_100_0,&t,&z2_50_0);

	/* 2^101 - 2^1 */ fe25519_square(&t,&z2_100_0);
	/* 2^200 - 2^100 */ for (i = 1;i < 100;i++) { fe25519_square(&t,&t); }
	/* 2^200 - 2^0 */ fe25519_mul(&t,&t,&z2_100_0);

	/* 2^201 - 2^1 */ fe25519_square(&t,&t);
	/* 2^250 - 2^50 */ for (i = 1;i < 50;i++) { fe25519_square(&t,&t); }
	/* 2^250 - 2^0 */ fe25519_mul(&t,&t,&z2_50_0);

	/* 2^251 - 2^1 */ fe25519_square(&t,&t);
	/* 2^252 - 2^2 */ fe25519_square(&t,&t);
	/* 2^252 - 3 */ fe25519_mul(r,&t,x);
}
#endif
