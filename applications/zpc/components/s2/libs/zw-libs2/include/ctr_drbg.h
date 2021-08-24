/* © 2017 Silicon Laboratories Inc.
 */
#ifndef CTR_DRBH_H
#define CTR_DRBH_H

#include<stdint.h>

/**
 * \ingroup crypto
 * @{
 */


#define OUTLEN 16
#define KEYLEN 16
#define SEEDLEN (OUTLEN + KEYLEN)

#ifndef RANDLEN
/**
 * Number of bytes AES_CTR_DRBG_Generate generates.
 */
#define RANDLEN 16
#endif
/*
 * AES-128 processes data 16 bytes at a time
 */
#define AJ_BLOCKSZ 16

/**
 * Context for AES-128 CTR DRBG
 */
typedef struct _CTR_DRBG_CTX {
    uint8_t df;        /**< Use DF or not */
    uint8_t v[OUTLEN]; /**< Internal working state */
    uint8_t k[KEYLEN]; /**< Key working state */
} CTR_DRBG_CTX;


/* Generate 16 bytes of RNG output.
 * \param[inout] ctx The DBRG to reseed
 * \param[in]    seed 32 bytes of reseed entropy
 * \return OK if reseed succeeded, FAIL otherwise.
 */
//void AES_CTR_DRBG_Reseed(CTR_DRBG_CTX* ctx, uint8_t* seed, size_t size);
DllExport
void AES_CTR_DRBG_Reseed(CTR_DRBG_CTX* ctx, uint8_t* seed);

/* Instantiate a DRBG
 * \param[out] ctx             The DBRG to instantiate and seed
 * \param[in]  entropy_input   The 32-byte input entropy to seed the generator
 * \param[in]  personalization An 8-byte fixed length personalization string
 * We don't use security strength parameters.
 * Returns nothing. This cannot fail because the HW RNG cannot fail.
 */
DllExport
void AES_CTR_DRBG_Instantiate(CTR_DRBG_CTX* ctx, uint8_t* entropy_input,const uint8_t *personalization);

/* Generate 64 bytes of RNG output.
 * \param[inout] ctx  The DBRG to generate from
 * \param[out]   rand The buffer of size RANDLEN to write random data to. By default RANDLEN is 16 bytes.
 */
DllExport
void AES_CTR_DRBG_Generate(CTR_DRBG_CTX* ctx, uint8_t* rand);


/**
 * @}
 */

#endif
