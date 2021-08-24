/* © 2014 Silicon Laboratories Inc.
 */
/* Author: mdumbare */
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ctr_drbg.h>
#include <aes_cmac.h>

/**
 * Outputs 32 bytes of data
 */
static void ckdf_nonce0_expand(uint8_t *prk, uint8_t *mei)
{
    /*Constant(EI) = 0x88 repeated 15 times*/
    const uint8_t constant_ei[15] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88};
    uint8_t t0[32]; /* 15 plus 1 plus 1 */
    uint8_t t1[16];
    uint8_t t2[16];

    /*T0 = Constant ei | 0x00*/
    /* TODO FIXME SPEC: spec says contant_nk. We think it should be constant_ei */
    memcpy(t0, constant_ei, 15);
    t0[15] = 0;

    /*T1 = CMAC(K NONCE , T0 | Constant NK | 0x01)*/
    memcpy(t0+16, constant_ei, 15);
    t0[16+15] = 1;
    aes_cmac_calculate(prk, t0, 32, t1); /* TODO t1 will be 16 byte after this? */

    /*T2 = CMAC(K NONCE , T1 | Constant NK | 0x02)*/
    memcpy(t0, t1, 16);  // backup t1 inside t0
    memcpy(t0+16, constant_ei, 15);
    t0[16+15] = 2;
    aes_cmac_calculate(prk, t0, 32, t2);

    /* MEI = T1 | T2 */
    memcpy(mei, t1, 16);
    memcpy(mei+16, t2, 16);
}

#ifdef VERBOSE
static void print_hex(uint8_t* buf, int len) {
  int i;
  for (i = 0; i < len; i++)
         {
           printf("%02X", buf[i]);
         }
       printf("\n");
}
#endif

void next_nonce_instantiate(CTR_DRBG_CTX* ctx,const uint8_t* ei_sender,const uint8_t* ei_receiver , uint8_t *k_nonce)
{
    /* Constant(NONCE) = 0x26 repeated 16 times.*/
    const uint8_t constant_nonce[16] = {0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26};
    uint8_t temp[32];
    uint8_t X[16];
    uint8_t mei[32];

    /* sender's EI | receiver's EI */
    memcpy(temp, ei_sender, 16);

    memcpy(temp+16, ei_receiver, 16);
    aes_cmac_calculate(constant_nonce, temp, 32, X);
    ckdf_nonce0_expand(X, mei);

    /*puts(__FUNCTION__);
    print_hex(mei,32);
    print_hex(k_nonce,32);
*/
    AES_CTR_DRBG_Instantiate(ctx, mei, k_nonce);
}

int next_nonce_generate(CTR_DRBG_CTX* ctx, uint8_t* rand)
{
    AES_CTR_DRBG_Generate(ctx, rand);
  /*  puts(__FUNCTION__);
    print_hex(rand,16);*/
    return 1;
}
