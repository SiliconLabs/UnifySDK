// CLEANUP: Moved from Serialapi.c

#ifdef SECURITY_SUPPORT
#include "rijndael-alg-fst.h"
bool SerialAPI_AES128_Encrypt(const uint8_t *ext_input, uint8_t *ext_output, const uint8_t *cipherKey) CC_REENTRANT_ARG{
  int Nr; /* key-length-dependent number of rounds */
  u32 rk[4*(MAXNR + 1)]; /* key schedule */
  /*if(zwapi_support_command_func(FUNC_ID_ZW_AES_ECB)) {
    memcpy(&buffer[0],cipherKey,16);
    memcpy(&buffer[16],ext_input,16);
    zwapi_session_send_frame_with_response(FUNC_ID_ZW_AES_ECB, buffer, 32 , buffer, &byLen );
    memcpy(ext_output,&buffer[IDX_DATA],16);
    return 1;
  } else*/ {
    Nr = rijndaelKeySetupEnc(rk, cipherKey, 128);
    rijndaelEncrypt(rk, Nr, ext_input, ext_output);
    return 1;
  }
}
#endif
