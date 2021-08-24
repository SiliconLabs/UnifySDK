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
 *    WHATSOEVER RESULTING FROM LOSS OF USE, __data OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctr_drbg.h>
#include <unity.h>
uint8_t dbgCTRDRBG = 0;

typedef struct {
    uint8_t df;            /* Use DF or not */
    const char* entropy;   /* Entropy input */
    const char* nonce;     /* Nonce */
    const char* personal;  /* Personalization string */
    const char* reseed;    /* Reseed input */
    const char* rand;      /* Output */
} TEST_CASE;

//static CTR_DRBG_CTX drbgctx;

/*
 * Known answer tests taken from
 * http://csrc.nist.gov/groups/STM/cavp/
 * http://csrc.nist.gov/groups/STM/cavp/documents/drbg/drbgtestvectors.zip
 * AES-128 use df, no predication resisitance, no reseed
 */
static TEST_CASE const testVector[] = {
    {
        0,
        "cee23de86a69c7ef57f6e1e12bd16e35e51624226fa19597bf93ec476a44b0f2",
        "",
        "a2ef16f226ea324f23abd59d5e3c660561c25e73638fe21c87566e86a9e04c3e",
        "",
        "2a76d71b329f449c98dc08fff1d205a2fbd9e4ade120c7611c225c984eac8531288dd3049f3dc3bb3671501ab8fbf9ad49c86cce307653bd8caf29cb0cf07764",
    },
    {
        0,
        "b09eb4a82a39066ec945bb7c6aef6a0682a62c3e674bd900297d4271a5f25b49",
        "",
        "a3b768adcfe76d61c972d900da8dffeeb2a42e740247aa719ed1c924d2d10bd4",
        "",
        "5a1c26803f3ffd4daf32042fdcc32c3812bb5ef13bc208cef82ea047d2890a6f5dcecf32bcc32a2585775ac5e1ffaa8de00664c54fe00a7674b985619e953c3a",
    },
    {
        0,
        "0bbc898f8daf3f12fc009c846fe2cea22f683a432eea297257312d5a44bac131",
        "",
        "8d34be9d8110b84b02a60508deae773c1a808f4e2a0ec81747ae2ec923fe99a9",
        "",
        "da49e24a6cb1d9e51b98ea6103627d9ad035770b7bdc760606e2b5f35afd13b7a61a4a18fac25258985fa1fb2b88a7cc17278b0539d7cf74f940f008ee2cf4cd",
    },
    {
        0,
        "3ea1f3fb153636c33982464f0e5b0daeba59c3f1ee91a612c4f6a9dcfcd0a978",
        "",
        "ea8671fc9c02584d69af91de2adacec1408d91d512718945ed1e7dc0b620b323",
        "",
        "2429e7d817cfd4f8500948d2ec2dec02b7d035b4bb986144bb918a31bfd2269e6907c34ac8beab69508869a4f04bc3c23ccfbae5d59eab857ece000d554b273a",
    },
    {
        0,
        "b6b2033e382decd21e4eabd8f1177761d06a12bae1cfed0059b7e16bd9bab8d7",
        "",
        "c99a49132543fce49b804e9f417d22e49c460bc4e60a6d36701fea561b93203d",
        "",
        "63626608b446c7d02212209d0a3888e40534864d8f5cd28aaff09505ee5e894751e5cb8467a5d85d87a675b7852724deb0d12038035400c3405fafb1a47f88c8",
    },
    {
        0,
        "acc465d1bf94ccdeec06c74c812db3a993c408b5c2ef7ebe9bdeb6a1a51976a7",
        "",
        "77278d47a169c559518d46ffb23aa594efdaebb067c48d4a392f60b94cc15b36",
        "",
        "e0e6e417de8fc5d212bdda4c026a13d6eae402874d62c30577ee4c3445ace85479cbc3b55bbbe1573c13f9b0242d7e3f6e7e91d932b3d5a6dca8df43c188ae5e",
    },
    {
        0,
        "14b33415d2321fbb10a768ced712c8999ff2f19b63264a81adc2fdc16370b185",
        "",
        "029b48805285349c292a99ee68fcefda1f61dd60a09f4616cb2efe4f8b140056",
        "",
        "8a69feb26be025a3849bb0b60959717daa59c889c383593d1f8debf58051eb463d07c659cfbe47e94224ba18833b71d95905e0cde785364a87706e31297fb323",
    },
    {
        0,
        "5a83af23cafb30e3a18e28651b3c1bd01813c44216e7e44b790d6664e2c3839a",
        "",
        "d0f108ae7c65e16cfa13e5582204eb8cd6ebb08f1f6a5d476c275247c1a83eb5",
        "",
        "e64397f0eea0d33d9715bcef2ee7eec22a86e8d066a25e44706dc688c499bd7ef08172c8cf36e3bddf79f2bec136a01aad844930e48a16fe1800d69fb0f4e163",
    },
    {
        0,
        "1dc24dd9d6a405a007bd082cfbdbd863185e072b67d663b14d7e8f16900cfce6",
        "",
        "0eca85ddcb6f38ff3683968ef98b52408428dcae2ec3b0fa4c68906c1b6481cd",
        "",
        "156938566fc25d493c1c60d8925819a6e59a2479d75f3efff16d46aada68403140407955c1fd9d2a890bcf67ac9b3b82d1d6cf788fd863da3d41ac6e34f217a0",
    },
    {
        0,
        "ecb7e61a4792a2115213d141d20710e8a3212d7847dd53dfa5d4d7777d10d97e",
        "",
        "bf09235d30cd69ada285948fe1be2e0c4e145ac8485d12ca7b8239136da1c638",
        "",
        "868db5832b2e9c3d2c9794b174b328ed2cc86e41017863eabc4a7c096a487bfe4d67ccf93a5e2c67d88dbd8f1419b2a9f1293e7a70e8e8fe93e2156496b0fa54",
    },
    {
        0,
        "e55727c590ec3ac108f4e5fec39a2d34bf89aee4e215dcc970db8ae8f6a0e4a8",
        "",
        "ade3d28a8e43aab8fd31dec8bdbe5c41c0b3f7f69a2d0aada5608ab0e57c8bb0",
        "",
        "198742299feecf6083e3a0bef75ac2b93de2defa6525883a55f88247dc6902279f792402faffe4a81213e40facb873cd499e4b0f7f0ff592bc06699db773b899",
    },
    {
        0,
        "c380dafd84f2782b7539ca1c3ad9715fe6b1805310a578afcffc9210ac127ded",
        "",
        "b298533d9c74a0d9b9aa0d82edc434002b4d1372318c2865b7700a5b7ebeabf0",
        "",
        "9aec7a5ba3e091e6a6c99e04395af2ab2eeaa1ef089baa51dc23ea31603b899ea298317603354f38fd9c36c2a53a05c1e468c6ae32fe4c3b0056ec0d5eff22b6",
    },
    {
        0,
        "f158210535a404f6cb4f9550b3f26e3f777a9faa164774749f48ef4a8ce09449",
        "",
        "d56fa873cc762f64b3ab31b6291e247efca71fca90453cfff1f2b0b3e265c475",
        "",
        "034b41b2a9a6764e5ed1edb00aea3185fe43eb81b4253e7cade97956fccd9fc5782328fada8ed5208f1d46b1f872e333b9e2f036a51746ccaf39e1a85af8eb23",
    },
    {
        0,
        "973a99b7e03b393ef689af8cb90d7436cae10e4814814aed342dd38e2a7346e3",
        "",
        "300d3d2adbac6d7ddadde07e08b0d72b5b39ff36031e81d8303958242e3cf03f",
        "",
        "62012842991fe3220f1d961045f028a3b6a729f5a451b8c2ec90e1c1e2b1e4042e97267e1bfa1782a10c3c29509bc8f2adffd3d695861e1594da91702830faf7",
    },
    {
        0,
        "22d1d8c8bde76a239d032804717face16d77b51170d0f53ccbcca4eaff4fb315",
        "",
        "5d3a7d40fdf95b98454bca03c6fcbf6abf3807de75171b55bab2db5a3f5f12f2",
        "",
        "1007e11f48e3c4813fddd67310db56d67a49fe93e45e61b37ba81485df6a62ee57ca41fa1d987f467c2939790a20421c2b4f70b28fb0b90bbeab1ac0ae884f1a",
    },
};

#if 0
uint32_t AJ_PlatformEntropy(uint8_t* __data, uint32_t size)
{
    FILE* f = fopen("/dev/urandom", "r");
    if (NULL == f) {
        return 0;
    }
    size = fread(__data, sizeof (uint8_t), size, f);
    fclose(f);
    return size;
}
#endif

int AJ_RawToHex(const uint8_t* raw, size_t rawLen, char* hex, size_t hexLen, uint8_t lower)
{
    static const char nibble_upper[] = "0123456789ABCDEF";
    static const char nibble_lower[] = "0123456789abcdef";
    const char* nibble = lower ? nibble_lower : nibble_upper;
    char* h = hex + 2 * rawLen;
    const uint8_t* a = raw + rawLen;

    if ((2 * rawLen + 1) > hexLen) {
        return 11;
    }
    h[0] = '\0';
    /*
     * Running backwards encode each byte in inStr as a pair of ascii hex digits.
     * Going backwards allows the raw and hex buffers to be the same buffer.
     */
    while (rawLen--) {
        uint8_t n = *(--a);
        h -= 2;
        h[0] = nibble[n >> 4];
        h[1] = nibble[n & 0xF];
    }
    return 0;
}

#if 0
void AJ_RandBytes(uint8_t* rand, uint32_t size)
{
    int status = 13;
    uint8_t seed[SEEDLEN];

    if (rand && size) {
        status = AES_CTR_DRBG_Generate(&drbgctx, rand, size);
        if (status != 0) {
            // Reseed required
            AJ_PlatformEntropy(seed, sizeof (seed));
            AES_CTR_DRBG_Reseed(&drbgctx, seed, sizeof (seed));
            status = AES_CTR_DRBG_Generate(&drbgctx, rand, size);
        }
    } else {
        // This is the first call to initialize
        size = AJ_PlatformEntropy(seed, sizeof (seed)); // Fill seed with SEEDLEN no of random __data from /dev/urandom
        drbgctx.df = (SEEDLEN == size) ? 0 : 1;
        AES_CTR_DRBG_Instantiate(&drbgctx, seed, sizeof (seed));
    }
}
#endif

static uint8_t A2H(char hex, int* status)
{
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    }
    hex |= 0x20;
    if (hex >= 'a' && hex <= 'f') {
        return 10 + hex - 'a';
    } else if (hex >= 'A' && hex <= 'F') {
        return 10 + hex - 'A';
    } else {
        *status = 3;
        return 0;
    }
}

int AJ_HexToRaw(const char* hex, size_t hexLen, uint8_t* raw, size_t rawLen)
{
    int status = 0;
    char* p = (char*)raw;
    size_t sz = hexLen ? hexLen : strlen(hex);
    size_t i;

    /*
     * Length of encoded hex must be an even number
     */
    if (sz & 1) {
        return 2;
    }
    if (rawLen < (sz / 2)) {
        return 11;
    }
    for (i = 0; (i < sz) && (status == 0); i += 2, hex += 2) {
        *p++ = (A2H(hex[0], &status) << 4) | A2H(hex[1], &status);
    }
    return status;
}

/**
 * Macro for getting the size of an array variable
 */
#define ArraySize(a)  (sizeof(a) / sizeof(a[0]))

void test_ctr_dbrg(void)
{
    CTR_DRBG_CTX ctx;
    size_t i;
    size_t size;
    uint8_t* d;
    uint8_t* __data;
    uint8_t* rand;
    uint8_t* personal;
    uint8_t* p;
    char     message[256];

    for (i = 0; i < ArraySize(testVector); i++) {
        size_t elen = strlen(testVector[i].entropy) / 2;
        size_t nlen = strlen(testVector[i].nonce) / 2;
        size_t plen = strlen(testVector[i].personal) / 2;
        size_t rlen = strlen(testVector[i].reseed) / 2;

        size = elen + nlen;
        __data = malloc(size);
        personal = malloc(size);
        assert(__data);
        assert(personal);
        d = __data;
        p = personal;
        AJ_HexToRaw(testVector[i].entropy, 2 * elen, d, size);
        d += elen;
        AJ_HexToRaw(testVector[i].nonce, 2 * nlen, d, size);

        AJ_HexToRaw(testVector[i].personal, 2 * plen, p, size);

#if 0
        for (i = 0; i < SEEDLEN; i++) {
            /* temp = Leftmost seedlen bits of temp.
               temp = temp || provided___data;
            */
            __data[i] ^= personal[i];
        }
#endif
        //AJ_DumpBytes("SEED", __data, size);
        AES_CTR_DRBG_Instantiate(&ctx, __data, personal);
        free(__data);

        if (rlen) {
            size = rlen;
            __data = malloc(size);
            assert(__data);
            AJ_HexToRaw(testVector[i].reseed, 2 * rlen, __data, size);
            AES_CTR_DRBG_Reseed(&ctx, __data);
            free(__data);
        }

        size = strlen(testVector[i].rand) / 2;
        __data = malloc(size);
        rand = malloc(size);
        TEST_ASSERT(__data);
        TEST_ASSERT(rand);
        AJ_HexToRaw(testVector[i].rand, 2 * size, rand, size);

        AES_CTR_DRBG_Generate(&ctx, __data);
        AES_CTR_DRBG_Generate(&ctx, __data);
#ifdef VERBOSE
        printf("rand: ");
        for(j = 0; j < size; j++)
            printf("%x ", rand[j]);

        printf("\n");
        printf("__data: ");
        for(j = 0; j < size; j++)
            printf("%x ", __data[j]);

        printf("\n");
#endif
        sprintf(message, "Expected failed for test #%lu\n", (long unsigned)i);
        TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(__data, rand, size, message);

        free(__data);
        free(rand);
        free(personal);
    }

    /*
    // Initialize the core context
    AJ_RandBytes(NULL, 0);

    // Get some random __data
    size = 64;
    rand = malloc(size);
    assert(rand);
    hex = malloc(2 * size + 1);
    assert(hex);
    for (i = 0; i < 64; i++) {
        AJ_RandBytes(rand, size);
        AJ_RawToHex(rand, size, hex, 2 * size + 1, 0);
        printf("%s\n\n", hex);
    }
    free(rand);
    free(hex);
    */
}
