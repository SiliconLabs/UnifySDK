/* © 2017 Silicon Laboratories Inc.
 */
#ifndef _CURVE_25519_H_
#define _CURVE_25519_H_

/**
 * \ingroup crypto
 * @{
 */

/**
* Calculate the public key corresponding to a random key. The random key is usually a random
* number. Both keys are 32-byte integers.
* \param[out] q The public key korresponding to n.
* \param[in] n The private key.
*/
DllExport
void crypto_scalarmult_curve25519_base(
    uint8_t *q,
    const uint8_t *n
);

/**
* Calculate an ECDH shared secret from a public and a private key.
* \param[out] r The shared secret.
* \param[in] s The private key.
* \param[in] p The public key
*/
DllExport
void crypto_scalarmult_curve25519(
    uint8_t *r,
    const uint8_t *s,
    const uint8_t *p
);

/**
 * @}
 */

#endif /* _CURVE_25519_H_ */
