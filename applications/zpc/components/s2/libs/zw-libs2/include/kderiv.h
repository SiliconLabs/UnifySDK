/* © 2017 Silicon Laboratories Inc.
 */
#ifndef KDERIV_H
#define KDERIV_H
/**
 * \ingroup crypto
 * @{
 */


/*
Input is defined by:
- NetworkKey is obtained during the key exchange phase.
- Constant NK
    = 0x55 repeated 15 times
Output is obtained by:
- T0 = Empty string
- T1 = CCM Key,                            K(C) = CMAC(NetworkKey, T0 | Constant(NK) | 0x01)
- T2 = Nonce Personalization string 1, K(NONCE) = CMAC(NetworkKey, T1 | Constant(NK) | 0x02)
- T3 = Nonce Personalization string 2, K(NONCE) = CMAC(NetworkKey, T2 | Constant(NK) | 0x03)
*/
/**
* Expand network key into ccm key and nonce personalization string (for CTR_DRBG).
* \param[in] network_key The network key
* \param[out] ccm_key 16 byte CCM key.
* \param[out] nonce_pstring The 32-byte personalization string for instantiation of SPAN NextNonce functions.
* \param[out] mpan_key 16 byte MPAN key.
*/
DllExport
void networkkey_expand(
    const uint8_t *network_key,
    uint8_t *ccm_key,
    uint8_t *nonce_pstring,
    uint8_t *mpan_key
    );

/**
   Input is defined by:
      ConstantPRK   = 0x33 repeated 16 times
       ECDH Shared Secret is the output of the ECDH key exchange
       AuthTag is defines as ECDH Controller Public Key | ECDH Device Public Key
   Output is obtained by:
       PRK =  CMAC(ConstantPRK, ECDH Shared Secret | AuthTag)
*/
DllExport
void tempkey_extract(
	const uint8_t  *const ecdh_share_secret,
	const uint8_t  *const auth_tag,
	uint8_t *pseudo_random_keymat_output
	);

/*
 Input is defined by:
     PRK is defined in the previous section of tempkey_extract()
     Constant TE 0x88 repeated 15 times
     ECDH CPK = The ECDH Public Key of the Controller
     ECDH DPK = The ECDH Public Key of the Device being included
 Output is obtained by:
     T0 = empty zero length string
     T1 = CCM Key,                            K(C) = CMAC(PRK, T0 | Constant T0 | counter 0x01)
     T2 = Nonce Personalization string 1, K(NONCE) = CMAC(PRK, T1 | Constant TE | Counter 0x02)
     T3 = Nonce Personalization string 2, K(NONCE) = CMAC(PRK, T2 | Constant TE | Counter 0x03)
     T4 = MPAN Key,                       K(MPAN)  = CMAC(PRK, T3 | Constant TE | Counter 0x04)
*/
DllExport
void tempkey_expand(
    const uint8_t *prk,
    uint8_t *temp_ccm_key,
    uint8_t *temp_nonce_pstring,
    uint8_t *temp_mpan_key
    );


/**
 * @}
 */
#endif
