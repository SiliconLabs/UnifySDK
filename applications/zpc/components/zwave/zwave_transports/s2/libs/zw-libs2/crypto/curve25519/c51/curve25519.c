#pragma optimize (8, SPEED)

/*
 * File:    avrnacl_8bitc/crypto_scalarmult/curve25519.c
 * Author:  Michael Hutter, Peter Schwabe
 * Version: Wed Aug 6 13:19:40 2014 +0200
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
#include "fe25519.h"

//#include "wc_util.h"

//uint8_t debug_flag = 0;

static void work_cswap(fe25519_t *work, int8_t b)
{
  fe25519_t t;
  fe25519_setzero(&t);
  fe25519_cmov(&t, work+1, b);
  fe25519_cmov(work+1, work+3, b);
  fe25519_cmov(work+3, &t, b);
  fe25519_cmov(&t, work+2, b);
  fe25519_cmov(work+2, work+4, b);
  fe25519_cmov(work+4, &t, b);
}

static const fe25519_t _121666 = {{0x42, 0xDB, 0x01, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

static void ladderstep(fe25519_t *work)
{
  fe25519_t t1,t2,t3,t4,t5,t6,t7;
  fe25519_t *x0 = work; 
  fe25519_t *xp = work+1; 
  fe25519_t *zp = work+2;
  fe25519_t *xq = work+3; 
  fe25519_t *zq = work+4;

//  static uint8_t count = 0;

//  print_txt("print ");
//  print_number(count);
//  count++;


//  print32(work->value);
//  print32((work+1)->value); // xp
//  print32((work+2)->value);
//  print32((work+3)->value);
//  print32((work+4)->value);

  fe25519_add(&t1, xp, zp);

  //print_txt("add xp ");
  //print32(xp->value);


  fe25519_sub(&t2, xp, zp);

//  print_txt("sub t2 ");
//  print32(t2.value);

//print32(t2.value);
  fe25519_square(&t7, &t2);

//  print_txt("square t7");
//  print32(t7.value);

  fe25519_square(&t6, &t1);

//  print_txt("square t6");
//  print32(t6.value);

  fe25519_sub(&t5,&t6,&t7);



  fe25519_add(&t3, xq, zq);
//print32(t3.value);
  fe25519_sub(&t4, xq, zq);
  fe25519_mul(&t2,&t3,&t2);
  fe25519_mul(&t3,&t4,&t1);
  fe25519_add(xq, &t3, &t2);




  fe25519_sub(zq, &t3, &t2);
  fe25519_square(xq, xq);
  fe25519_square(zq, zq);
  fe25519_mul(zq, zq, x0);


//  debug_flag = 1;
  fe25519_mul(xp, &t6, &t7);

//  print_txt("mul xp ");
//  print32(xp->value);

  //fe25519_mul121666(zp, &t5);

  //print32(work[1].value);

  fe25519_mul(zp, &t5, &_121666);


  fe25519_add(zp, zp, &t7);
  fe25519_mul(zp, zp, &t5);
}

static void mladder(fe25519_t *xr, fe25519_t *zr, const uint8_t s[32])
{
  fe25519_t work[5];
  uint8_t bit_, prevbit=0;
  uint8_t swap;
  int8_t j;
  int8_t i;

  work[0] = *xr;
  fe25519_setone(work+1);
  fe25519_setzero(work+2);
  work[3] = *xr;
  fe25519_setone(work+4);

#ifdef NOT_USED
  print_txt("w0");
  print32(work->value); // 09 buffer
  print_txt("w1");
  print32((work + 1)->value); // 01 buffer
  print_txt("w2");
  print32((work + 2)->value); // 00 buffer
  print_txt("w3");
  print32((work + 3)->value); // 09 buffer
  print_txt("w4");
  print32((work + 4)->value); // 01 buffer
#endif

  j = 6;
  for(i=31;i>=0;i--)
  {
//    print_number(i);
//    print32(work[1].value);
    while(j >= 0)
    {
      bit_ = 1&(s[i]>>j);
      swap = bit_ ^ prevbit;
      prevbit = bit_;
      work_cswap(work,swap);
      //print32(work[1].value);
      ladderstep(work);
      //print32(work[1].value);
      j -= 1;
    }
    j = 7;
  }
  *xr = work[1];
  *zr = work[2];
}


void crypto_scalarmult_curve25519(
    uint8_t *r,
    const uint8_t *s,
    const uint8_t *p
    )
{
  fe25519_t t;
  fe25519_t z;
  uint8_t e[32];
  uint8_t i;

//  print32((uint8_t *)s);
//  print32((uint8_t *)p);


  for(i=0;i<32;i++) e[i] = s[i];
  e[0] &= 248;
  e[31] &= 127;
  e[31] |= 64; 

  fe25519_unpack(&t, p);

//  print32(t.value);

  mladder(&t, &z, e);

//  print32(t.value);
//  print32(z.value);

  fe25519_invert(&z, &z);
  fe25519_mul(&t, &t, &z);
  fe25519_pack(r, &t);
}

static const uint8_t base[32] = {9};

void crypto_scalarmult_curve25519_base(
    uint8_t *q,
    const uint8_t *n
    )
{
  crypto_scalarmult_curve25519(q,n,base);
}
