// CLEANUP: Moved from Serialapi.h - since it does not use the serial API

/* Access hardware AES encrypter. ext_input,ext_output,ext_key are all 16 byte arrays
 * Returns true if routine went well. */
bool SerialAPI_AES128_Encrypt(const uint8_t *ext_input, uint8_t *ext_output, const uint8_t *ext_key);
