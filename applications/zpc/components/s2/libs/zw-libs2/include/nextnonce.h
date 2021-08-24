/* © 2017 Silicon Laboratories Inc.
 */

#ifndef NEXTNONCE_H
#define NEXTNONCE_H
#include "ctr_drbg.h"

/**
 * \ingroup crypto
 *
 * @{
 */

/* Instantiate nextnonce

 * \param[out] The DBRG to instantiate and seed
 * \param[in] The 16-byte input entropy from sender to seed the generator
 * \param[in] The 16-byte input entropy from receiver to seed the generator
 * \param[in] An 32-byte fixed length personalization string
 * We dont use security strength parameters. No reseeding counter.
 */
DllExport
void next_nonce_instantiate(CTR_DRBG_CTX* ctx, const uint8_t* ei_sender, const uint8_t* ei_receiver , uint8_t *k_nonce);

/* Generate 16 bytes of RNG output.
 * \param[inout] The DBRG to generate from
 * \param[out] The 16 byte buffer to write random data to.
 * \return OK if random data was generated, FAIL otherwise.
 */
DllExport
int next_nonce_generate(CTR_DRBG_CTX* ctx, uint8_t* rand);

/**
 * @}
 */
#endif
