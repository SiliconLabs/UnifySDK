/* © 2014 Silicon Laboratories Inc.
 */

/* Author: mdumbare */

#include <stdio.h>
#include <string.h>
#include <ZW_s2_key_scrambler.h>
int main()
{
    uint8_t key[16];
    uint8_t out[16];
//    int i;

    memset(key, 0x34, 16);
    memset(out, 0, 16);
    ZW_s2_key_scrambler_set_key(key);
    ZW_s2_key_scrambler_get_key(out);
#if 0
    for (i = 0; i < 16; i++) {
        printf("%x", out[i]);
    }
#endif

    if (memcmp(key, out, 16) == 0) {
        printf("test passed\n");
        return 1;
    }
    
    return 0;
}
