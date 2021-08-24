/* © 2014 Silicon Laboratories Inc.
 */

/* author: mdumbare */
#include <string.h>
#include <stdio.h>
#include "aes_cmac.h"
#include <kderiv.h>

#define CONSTANT_NK_LEN 15

void generic_expand(
    const uint8_t *network_key,
    const uint8_t *constant_nk,
    uint8_t *ccm_key,
    uint8_t *nonce_pstring,
    uint8_t *mpan_key)
{
    uint8_t temp[32];

    /* ccm_key = CMAC(NetworkKey, temp) */
    memcpy(temp, constant_nk, 15);
    temp[15] = 0x01;

    aes_cmac_calculate(network_key, temp, 16, ccm_key);

    /* nonce_pstring first half */
    memcpy(temp, ccm_key, 16);
    memcpy(temp+16, constant_nk, 15);
    temp[31] = 0x02;

    aes_cmac_calculate(network_key, temp, 32, nonce_pstring);

    /* nonce_pstring is 32 bytes, fill out second half */
    memcpy(temp, nonce_pstring, 16);
    memcpy(temp+16, constant_nk, 15);
    temp[31] = 0x03;

    aes_cmac_calculate(network_key, temp, 32, nonce_pstring+16);

    /* MPAN key */
    memcpy(temp, nonce_pstring+16, 16);
    memcpy(temp+16, constant_nk, 15);
    temp[31] = 0x04;

    aes_cmac_calculate(network_key, temp, 32, mpan_key);

    return;
}

void networkkey_expand(
    const uint8_t *network_key,
    uint8_t *ccm_key,
    uint8_t *nonce_pstring,
    uint8_t *mpan_key)
{
    uint8_t constant_nk[15] = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};
    generic_expand(network_key, constant_nk, ccm_key, nonce_pstring, mpan_key);
}

void tempkey_extract(
	const uint8_t  *const ecdh_share_secret,
	const uint8_t  *const auth_tag,
	uint8_t *pseudo_random_keymat_output)
{
    uint8_t temp[96];
    uint8_t constant_prk[16] = {0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33};
    
	/* concatenate ecdh shared secret with auth_tag */
    memcpy(temp, ecdh_share_secret, 32); /*length of ecdh_share_secret is 32 byte */
    memcpy(temp+32, auth_tag, 64);/*length of auth_tag is 64 byte */
	/* cmac concatenation with constant_PRK as key */
    aes_cmac_calculate(constant_prk, temp, 96, pseudo_random_keymat_output);
	/* return first 16 bytes as output */
    return;
}

void tempkey_expand(
    const uint8_t *prk,
    uint8_t *temp_ccm_key,
    uint8_t *temp_nonce_pstring,
    uint8_t *temp_mpan_key)
{
    uint8_t constant_TE[15] = {0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88};
    generic_expand(prk, constant_TE, temp_ccm_key, temp_nonce_pstring, temp_mpan_key);
}
