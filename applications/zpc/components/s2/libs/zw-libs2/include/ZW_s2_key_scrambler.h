/* © 2017 Silicon Laboratories Inc.
 */
/**
 * @file ZW_s2_key_scrambler.h
 * @author Christian Salmony Olsen
 * @date 31/08/2015
 * @brief This module scrambles and de-scrambles the S2 network key.
 */

#ifndef PRODUCTPLUS_SWITCHONOFF_NVM_TEST_ZW_S2_KEY_SCRAMBLER_H_
#define PRODUCTPLUS_SWITCHONOFF_NVM_TEST_ZW_S2_KEY_SCRAMBLER_H_

#include <stdint.h>
#include <ZW_typedefs.h>

/**
 * @brief Served with a pointer to an array of 16 bytes, this function will
 * de-scramble the s2 network key read from NVM and store it in the array.
 * @param[inout] p_scramble_key Pointer to key used for scrambling/descrambling.
 * @param[inout] p_key          Pointer to scrambled and descrambled key
 */
void ZW_s2_key_scrambler_descramble(uint8_t * p_scramble_key, uint8_t * p_key);

/**
 * @brief Served with a pointer to an array of 16 bytes, this function will
 * scramble the array and store it in the NVM.
 * @param[inout] p_scramble_key Pointer to key used for scrambling/descrambling.
 * @param[inout] p_key Pointer to in-place scrambled key
 */
void ZW_s2_key_scrambler_scramble(uint8_t * p_scramble_key, uint8_t * p_key);

/* Scramble and descramble are identical operations */
#define ZW_s2_key_scrambler_scramble(x, y) ZW_s2_key_scrambler_descramble(x, y)

#endif /* PRODUCTPLUS_SWITCHONOFF_NVM_TEST_ZW_S2_KEY_SCRAMBLER_H_ */
