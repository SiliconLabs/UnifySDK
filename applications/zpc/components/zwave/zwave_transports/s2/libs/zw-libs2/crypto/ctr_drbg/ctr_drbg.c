/* © 2017 Silicon Laboratories Inc.
 */
/**
 * @file
 */
/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <ctr_drbg.h>
#include <aes.h>

#ifdef VERBOSE
#include <stdio.h>
#endif

#ifndef __C51__
void AJ_AES_ECB_128_ENCRYPT(uint8_t* key, uint8_t* in, uint8_t* out)
{
    AES128_ECB_encrypt(in, key, out);
}
#endif


static void AES_CTR_DRBG_Increment(uint8_t* __data, size_t size)
{
    while (size--) {
        __data[size]++;
        if (__data[size]) {
            break;
        }
    }
}
/*
CTR_DRBG_Update (provided_data, Key, V):
1.provided_data: The data to be used. This must be exactly seedlen
bits in length; this length is guaranteed by the construction of the
provided_data in the instantiate, reseed and generate functions.
2.Key: The current value of Key.
3.V: The current value of V.

Output:
1.K: The new value for Key.
2.V: The new value for V.
*/

static void AES_CTR_DRBG_Update(CTR_DRBG_CTX* ctx, uint8_t __data[SEEDLEN])
{
    size_t i = 0;
    uint8_t tmp[SEEDLEN];
    uint8_t* t = tmp;
#ifdef VERBOSE
    int j = 0;
#endif

    //AJ_AES_Enable(ctx->k);
    for (i = 0; i < SEEDLEN; i += OUTLEN) {
        AES_CTR_DRBG_Increment(ctx->v, OUTLEN); /*V= (V+ 1) mod 2 pow(outlen) */
#ifdef __C51__
        AES128_ECB_encrypt(ctx->v, ctx->k, t);
#else
        AJ_AES_ECB_128_ENCRYPT(ctx->k, ctx->v, t); /* output_block =  Block_Encrypt(Key, V). */
#endif
        t += OUTLEN; /*temp = temp || ouput_block */
    }

    for (i = 0; i < SEEDLEN; i++) {
        /* temp = Leftmost seedlen bits of temp.
           temp = temp || provided_data;
        */
        tmp[i] ^= __data[i];
    }

    memcpy(ctx->k, tmp, KEYLEN);
#ifdef VERBOSE
    printf("\n");
    printf("key: ");
    for(j = 0; j < KEYLEN; j++)
        printf("%x ", ctx->k[j]);
    printf("\n");
#endif
    memcpy(ctx->v, tmp + KEYLEN, OUTLEN);
#ifdef VERBOSE
    printf("v: ");
    for(j = 0; j < OUTLEN; j++)
        printf("%x ", ctx->v[j]);
    printf("\n");
#endif
}


void AES_CTR_DRBG_Reseed(CTR_DRBG_CTX* ctx, uint8_t* seed)
{
#ifdef VERBOSE
    printf("Reseed: ");
#endif
    AES_CTR_DRBG_Update(ctx, seed);
}


void AES_CTR_DRBG_Instantiate(CTR_DRBG_CTX* ctx, uint8_t* entropy, const uint8_t* personal)
{
#if 1
    uint8_t i;
#else
    int i;
#endif

    for (i = 0; i < SEEDLEN; i++) {
        /* temp = Leftmost seedlen bits of temp.
           temp = temp || provided_data;
        */
        entropy[i] ^= personal[i];
    }

#ifdef VERBOSE
    printf("Instantiate: ");
#endif
    memset(ctx->k, 0, KEYLEN);
    memset(ctx->v, 0, OUTLEN);
    ctx->df = 0;
    AES_CTR_DRBG_Reseed(ctx, entropy);
}

void AES_CTR_DRBG_Generate(CTR_DRBG_CTX* ctx, uint8_t* rand)
{
    uint8_t __data[SEEDLEN];
    size_t copy;
    size_t size = RANDLEN;

//    /* Needed?? Is uint8_t enough ??? */
//    uint16_t count = 0;

#ifdef VERBOSE
    printf("Generate: ");
#endif
    // Reseed interval 2^32 (counter wraps to zero)
    // See section 10.2.1.5.1. Step 1 in "CTR_DRBG Generate Proces"
    //AJ_AES_Enable(ctx->k);
    while (size) {
        AES_CTR_DRBG_Increment(ctx->v, OUTLEN);
#ifdef __C51__
        AES128_ECB_encrypt(ctx->v, ctx->k, __data);
#else
        AJ_AES_ECB_128_ENCRYPT(ctx->k, ctx->v, __data);
#endif
        copy = (size < OUTLEN) ? size : OUTLEN;
        memcpy(rand, __data, copy);
        rand += copy;
        size -= copy;
//        count+= copy;
    }

    memset(__data, 0, SEEDLEN);
    AES_CTR_DRBG_Update(ctx, __data);
}
