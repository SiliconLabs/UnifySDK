/* © 2014 Silicon Laboratories Inc.
 */
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>
#include "ccm.h"

void test_verify_handful_payload_lengths(void)
{
    uint8_t key[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
    uint8_t nonce[15-7] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
    uint32_t aad_len = 16;
    uint8_t aad[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f} ;
    uint8_t *plaintext;
    uint16_t plaintext_len;
    uint8_t *ciphertext;
    uint32_t ciphertext_len;
    uint8_t *text_to_encrypt;
    uint8_t *text_to_encrypt_bkup;
    uint16_t text_to_encrypt_len = 16;
    int i, j;
    int ret;
    int lengths[6] = {0, 10, 22, 54, 1279, 1280}; // Makes no sense to test for length 0 since the compare function will compare zero characters.
    //int lengths[6] = {1, 10, 22, 54, 1279, 1280};
    uint8_t q;
    uint8_t n;
    uint8_t t;
    set_q_n_t(7, 8, 6);
    get_q_n_t(&q, &n, &t);

    for(i = 0; i < 6; i++) {
        plaintext_len = text_to_encrypt_len = lengths[i];
        ciphertext_len = text_to_encrypt_len + t;
        ciphertext = malloc(ciphertext_len);
        if (ciphertext == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            //return;
            //goto out3;
        }
        text_to_encrypt = malloc(text_to_encrypt_len);
        if (text_to_encrypt == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            goto out2;
        }
        text_to_encrypt_bkup = malloc(text_to_encrypt_len);
        if (text_to_encrypt_bkup == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            goto out1;
        }

        plaintext = malloc(plaintext_len);
        if (ciphertext == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            goto out4;
        }
        for (j = 0; j < lengths[i]; j++) {
            text_to_encrypt[j] = j;
            text_to_encrypt_bkup[j] = j;
         }
        memcpy(ciphertext, text_to_encrypt, text_to_encrypt_len); //in-place encryption, so pass ciphertext which is of size "text_to_encrypt_len + t"
        ret = CCM_encrypt_and_auth(key, nonce, aad, aad_len, ciphertext, text_to_encrypt_len);
        if (ret == 0) {
            printf("verify_handful_payload_lengths TEST FAILED in encryption\n");
            goto out;
        } else {
             ciphertext_len = ret;
        }

        ret = CCM_decrypt_and_auth(key, nonce, aad, aad_len, ciphertext, ciphertext_len);
        if ((ret == 0) && (lengths[i] != 0)) {
            printf("verify_handlful_payload_lengths TEST FAILED in decryption\n");
            ret = 1;
            goto out;
        } else {
            plaintext_len = ret;
        } 

        //UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(text_to_encrypt_bkup, ciphertext, plaintext_len, __LINE__, "");

        if(memcmp(ciphertext, text_to_encrypt_bkup, plaintext_len)) {
            printf("verify_handful_payload_lengths TEST FAILED at %dth iteration\n", i);
            ret = 1;
            goto out;
        }
        printf("verify_handful_payload_lengths TEST PASSSED for plaintext length: %d\n", lengths[i]);

        free(plaintext);
        free(text_to_encrypt);
        free(text_to_encrypt_bkup);
        free(ciphertext);
    }
    printf("verify_handful_payload_lengths TEST PASSSED\n");
    return;
out:
    free(plaintext);
out4:
    free(text_to_encrypt_bkup);
out1:
    free(text_to_encrypt);
out2:
    free(ciphertext);

    return;
}

void ignore_test_verify_all_payload_lengths(void)
{
    uint8_t key[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
    uint8_t nonce[15-7] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
    uint32_t aad_len = 16;
    uint8_t aad[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f} ;
    uint8_t *plaintext;
    uint16_t plaintext_len;
    uint8_t *ciphertext;
    uint32_t ciphertext_len;
    uint8_t *text_to_encrypt;
    uint8_t *text_to_encrypt_bkup;
    uint16_t text_to_encrypt_len = 16;
    int i, j;
    int ret;

    uint8_t q;
    uint8_t n;
    uint8_t t;
    set_q_n_t(7, 8, 6);
    get_q_n_t(&q, &n, &t);
    /*
    q = 7; 
    t = 6;
    n = 8;
    */

    for(i = 1; i < 0xffff; i++) {
        plaintext_len = text_to_encrypt_len = i;
        ciphertext_len = text_to_encrypt_len + t;
        ciphertext = malloc(ciphertext_len);
        if (ciphertext == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            goto out3;
        }
        text_to_encrypt = malloc(text_to_encrypt_len);
        if (text_to_encrypt == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            goto out2;
        }
        text_to_encrypt_bkup = malloc(text_to_encrypt_len);
        if (text_to_encrypt_bkup == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            goto out2;
        }

        plaintext = malloc(plaintext_len);
        if (ciphertext == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            goto out1;
        }
        for (j = 0; j < i; j++) {
            text_to_encrypt[j] = j;
            text_to_encrypt_bkup[j] = j;
         }
        memcpy(ciphertext, text_to_encrypt, text_to_encrypt_len); //in-place encryption, so pass ciphertext which is of size "text_to_encrypt_len + t"
        ret = CCM_encrypt_and_auth(key, nonce, aad, aad_len, ciphertext, text_to_encrypt_len);
        if (ret == 0) {
            printf("verify_all_payload_lengths TEST FAILED in encryption\n");
            goto out;
        } else {
             ciphertext_len = ret;
        }

        ret = CCM_decrypt_and_auth(key, nonce, aad, aad_len, ciphertext, ciphertext_len);
        if (ret == 0) {
            printf("verify_all_payload_lengths TEST FAILED in decryption\n");
            ret = 1;
            goto out;
        } else {
            plaintext_len = ret;
        } 

        UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(text_to_encrypt_bkup, ciphertext, plaintext_len, __LINE__, "");

        /*
        if(memcmp(ciphertext, text_to_encrypt_bkup, plaintext_len)) {
            printf("verify_all_payload_lengths TEST FAILED at %dth iteration\n", i);
            ret = 1;
            goto out;
        }
        printf("verify_all_payload_lengths TEST PASSSED for plaintext length: %d\n", i);
        */
        free(plaintext);
        free(text_to_encrypt);
        free(text_to_encrypt_bkup);
        free(ciphertext);
    }
    printf("verify_all_payload_lengths TEST PASSSED\n");
    return;
out:
    free(plaintext);
out1:
    free(text_to_encrypt);
out2:
    free(ciphertext);
out3:
    return;
}

void test_example1(void)
{
    uint8_t key[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
    /* 7 is valie of q below */
    uint8_t nonce[15-7] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
    uint32_t aad_len = 16;
    uint8_t aad[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f} ;
    uint8_t text_to_encrypt[16]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f};
    const uint8_t text_to_encrypt_bkup[16]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f};
#define TEXT_TO_ENCRYPT_LEN_DEF (16)
    uint16_t text_to_encrypt_len = TEXT_TO_ENCRYPT_LEN_DEF;
    uint16_t plaintext_len = 16;
#define T_DEF (6)
#define CIPHERTEXT_LEN_DEF (TEXT_TO_ENCRYPT_LEN_DEF + T_DEF)
    //uint8_t *ciphertext;
    uint8_t ciphertext[CIPHERTEXT_LEN_DEF];
    uint32_t ciphertext_len = CIPHERTEXT_LEN_DEF;
    int ret;
    uint8_t nist_cipher_and_auth_tag[16 + 6] = {0xd2, 0xa1, 0xf0, 0xe0, 0x51, 0xea, 0x5f, 0x62, 0x08, 0x1a, 0x77, 0x92, 0x07, 0x3d, 0x59, 0x3d, 0x1f, 0xc6, 0x4f, 0xbf, 0xac, 0xcd};
    uint8_t q;
    uint8_t n;
    uint8_t t;

    set_q_n_t(7, 8, T_DEF);
    get_q_n_t(&q, &n, &t);

    memcpy(ciphertext, text_to_encrypt, text_to_encrypt_len); //in-place encryption, so pass ciphertext which is of size "text_to_encrypt_len + t"
    printf("Refer to http://csrc.nist.gov/publications/nistpubs/800-38C/SP800-38C_updated-July20_2007.pdf page 17 Example 2\n");
    ret = CCM_encrypt_and_auth(key, nonce, aad, aad_len, ciphertext, text_to_encrypt_len);

    UNITY_TEST_ASSERT((0 != ret), __LINE__, "Could not encrypt :(");

    ciphertext_len = ret;

    UNITY_TEST_ASSERT_EQUAL_UINT32(sizeof(nist_cipher_and_auth_tag), ciphertext_len, __LINE__, "Ciphertext length does not match with NIST document.");

    UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(nist_cipher_and_auth_tag, ciphertext, ciphertext_len, __LINE__, "Ciphertext does not match with NIST document.");

    ret = CCM_decrypt_and_auth(key, nonce, aad, aad_len, ciphertext, ciphertext_len);

    UNITY_TEST_ASSERT((0 != ret), __LINE__, "Could not decrypt :(");

    plaintext_len = ret;

    UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(text_to_encrypt_bkup, ciphertext, plaintext_len, __LINE__, "");
}

void test_big_aad_example(void)
{
    uint8_t key[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
    uint8_t nonce[13] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c};
    uint8_t *aad;
    uint8_t text_to_encrypt[32]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f};
    uint8_t text_to_encrypt_bkup[32]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f};
    uint16_t text_to_encrypt_len = 32;
    uint16_t plaintext_len = 32;
    int i, j = 0;
    int aad_len = 524288 / 8;
    uint8_t *ciphertext;
    uint32_t ciphertext_len;
    int ret;
    uint8_t nist_cipher_and_auth_tag[32 + 14] = {0x69,0x91,0x5d,0xad,0x1e,0x84,0xc6,0x37,0x6a,0x68,0xc2,0x96,0x7e,0x4d,0xab,0x61,0x5a,0xe0,
                0xfd,0x1f,0xae,0xc4,0x4c,0xc4,0x84,0x82,0x85,0x29,0x46,0x3c,0xcf,0x72,0xb4,0xac,0x6b,0xec,0x93,0xe8,0x59,0x8e,0x7f,0x0d,0xad,0xbc,0xea,0x5b};

    uint8_t q;
    uint8_t n;
    uint8_t t;
    set_q_n_t(2, 13, 14);
    get_q_n_t(&q, &n, &t);
    /*
    q = 2; 
    t = 14;
    n = 13;
    */

    /*the for loop below writes 1 byte more than aad_len, so just to be secure alloc 2 more bytes*/
    aad = malloc(aad_len + 2);

    UNITY_TEST_ASSERT_NOT_NULL(aad, __LINE__, "malloc failed for aad.");

#ifdef NOT_USED
    if (aad == NULL) {
        printf("malloc failed for aad\n");
        return;
    }
#endif
    ciphertext_len = text_to_encrypt_len + t;
    ciphertext = malloc(ciphertext_len);

    if (ciphertext == NULL) {
        printf("malloc failed for ciphertext\n");
        ret = 1;
        goto free_aad;
    }

    for (j = 0; j <= aad_len; j++) {
        i = j;
        if (i > 0xff)
            i = i % 0x100; 
        aad[j] = i;
    }

    memcpy(ciphertext, text_to_encrypt, text_to_encrypt_len); //in-place encryption, so pass ciphertext which is of size "text_to_encrypt_len + t"
    ret = CCM_encrypt_and_auth(key, nonce, aad, aad_len, ciphertext, text_to_encrypt_len);
    if (ret == 0) {
        printf("EXAMPLE4 TEST FAILED in encryption\n");
        goto out;
    } else {
        ciphertext_len = ret;
    }
        
    if(memcmp(ciphertext, nist_cipher_and_auth_tag, ciphertext_len) != 0) {
        printf("EXAMPLE4 ENCRYPTION TEST FAILED\n");
        ret = 1;
        goto out;
    }
    ret = CCM_decrypt_and_auth(key, nonce, aad, aad_len, ciphertext, ciphertext_len);
    if (ret == 0) {
        printf("test_big_aad_example TEST FAILED in decryption\n");
        ret = 1;
        goto out;
    }

    UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(text_to_encrypt_bkup, ciphertext, plaintext_len, __LINE__, "");

    /*
    if(memcmp(ciphertext, text_to_encrypt_bkup, plaintext_len)) {
        printf("test_big_aad_example ENCRYPTION test FAILED\n");
        ret = 1;
        goto out;
    }
    printf("test_big_aad_example test PASSSED\n");
    */
    ret = 0;
out:
    free(ciphertext);
free_aad:
    free(aad);
    return;
}

void test_handful_aad_lengths(void)
{
    uint8_t key[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
    uint8_t nonce[13] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c};
    uint8_t *aad;
    uint8_t text_to_encrypt[32]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f};
    uint8_t text_to_encrypt_bkup[32]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f};
    uint16_t text_to_encrypt_len = 32;
    uint16_t plaintext_len = 32;
    int i, j;
    int aad_len;
    uint8_t *ciphertext;
    uint32_t ciphertext_len;
    int ret;

    int lengths[] = {0, 1, 10, 11, 12, 21, 22, 300};

    uint8_t q;
    uint8_t n;
    uint8_t t;
    set_q_n_t(2, 13, 8);
    get_q_n_t(&q, &n, &t);
    /*
    q = 2; 
    t = 8;
    n = 13;
    */

    for(i = 0; i < sizeof(lengths)/sizeof(*lengths); i++) {
        aad_len = lengths[i];
        aad = malloc(aad_len);
        for (j = 0; j < lengths[i]; j++) {
            aad[j] = j;
        }
        if (aad == NULL) {
            printf("malloc failed for aad\n");
            return;
        }
        ciphertext_len = text_to_encrypt_len + t;
        ciphertext = malloc(ciphertext_len);
        if (ciphertext == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            goto free_aad;
        }

        memcpy(ciphertext, text_to_encrypt, text_to_encrypt_len); //in-place encryption, so pass ciphertext which is of size "text_to_encrypt_len + t"
        ret = CCM_encrypt_and_auth(key, nonce, aad, aad_len, ciphertext, text_to_encrypt_len);
        if (ret == 0) {
            printf("test_all_aad_lengths TEST FAILED in encryption\n");
            goto out;
        } else {
            ciphertext_len = ret;
        }

        plaintext_len = CCM_decrypt_and_auth(key, nonce, aad, aad_len, ciphertext, ciphertext_len);
        if (plaintext_len == 0) {
            printf("test_all_aad_lengths TEST FAILED in decryption\n");
            ret = 1;
            goto out;
        }
        
        UNITY_TEST_ASSERT_EQUAL_UINT8_ARRAY(text_to_encrypt_bkup, ciphertext, plaintext_len, __LINE__, "");

        /*
        if(memcmp(ciphertext, text_to_encrypt_bkup, plaintext_len) != 0) {
            printf("test_all_aad_lengths ENCRYPTION test FAILED\n");
            ret = 1;
            goto out;
        }
        memcpy(text_to_encrypt, text_to_encrypt_bkup, text_to_encrypt_len);
        printf("test_all_aad_lengths test PASSSED for aad length: %d\n", lengths[i]);
        */
        free(aad);
        free(ciphertext);
    }
    return;
out:
    free(ciphertext);
free_aad:
    free(aad);
    return;
}

#ifdef NOT_USED
static int test_all_aad_lengths()
{
    uint8_t key[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
    uint8_t nonce[13] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c};
    uint8_t *aad;
    uint8_t text_to_encrypt[32]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f};
    uint8_t text_to_encrypt_bkup[32]={0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f};
    uint16_t text_to_encrypt_len = 32;
    uint16_t plaintext_len = 32;
    int i, j;
    int aad_len;
    uint8_t *ciphertext;
    uint32_t ciphertext_len;
    int ret;

    uint8_t q;
    uint8_t n;
    uint8_t t;
    set_q_n_t(2, 13, 8);
    get_q_n_t(&q, &n, &t);

    /*
    q = 2; 
    t = 8;
    n = 13;
    */

    for(i = 0; i < 0xffff; i++) {
        aad_len = i;
        aad = malloc(aad_len);
        for (j = 0; j < i; j++) {
            aad[j] = j;
        }
        if (aad == NULL) {
            printf("malloc failed for aad\n");
            return 1;
        }
        ciphertext_len = text_to_encrypt_len + t;
        ciphertext = malloc(ciphertext_len);
        if (ciphertext == NULL) {
            printf("malloc failed for ciphertext\n");
            ret = 1;
            goto free_aad;
        }

        memcpy(ciphertext, text_to_encrypt, text_to_encrypt_len); //in-place encryption, so pass ciphertext which is of size "text_to_encrypt_len + t"
        ret = CCM_encrypt_and_auth(key, nonce, aad, aad_len, ciphertext, text_to_encrypt_len);
        if (ret == 0) {
            printf("test_all_aad_lengths TEST FAILED in encryption\n");
            goto out;
        } else {
            ciphertext_len = ret;
        }

        plaintext_len = CCM_decrypt_and_auth(key, nonce, aad, aad_len, ciphertext, ciphertext_len);
        if (plaintext_len == 0) {
            printf("test_all_aad_lengths TEST FAILED in decryption\n");
            ret = 1;
            goto out;
        }
        
        if(memcmp(ciphertext, text_to_encrypt_bkup, plaintext_len) != 0) {
            printf("test_all_aad_lengths ENCRYPTION test FAILED\n");
            ret = 1;
            goto out;
        }
        memcpy(text_to_encrypt, text_to_encrypt_bkup, text_to_encrypt_len);
        printf("test_all_aad_lengths test PASSSED for aad length: %d\n", i);
        free(ciphertext);
        free(aad);
    }
    return 0;
out:
    free(ciphertext);
free_aad:
    free(aad);
    return ret;
}

static int test_aad_len_smaller_than_12()
{
    uint8_t key[16]= {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
    uint8_t nonce[16] = {0x8E,0x14,0xD0,0x87,0xEA,0xF6,0x32,0xE1,0xDF,0x8A,0x36,0xC5,0x40,0x25,0xF4,0xB8};
    uint32_t aad_len = 8;
    uint8_t aad[8] = {0x02, 0x01, 0xaa, 0xbb, 0xcc, 0xdd, 0x02, 0x00};
    uint8_t text_to_encrypt[24]={0x48,0x65,0x6c,0x6c,0x6f,0x57,0x6f,0x72,0x6c,0x64, 0x20, 0x53, 0x65, 0x63, 0x6f, 0x6e, 0x64, 0x20, 0x46, 0x72, 0x61, 0x6d, 0x65, 0x00};
    uint16_t text_to_encrypt_len = 24;
    uint16_t plaintext_len = 8;
    uint8_t *ciphertext;
    uint32_t ciphertext_len;
    uint8_t text_to_encrypt_bkup[24];
    int ret;

    uint8_t q;
    uint8_t n;
    uint8_t t;
    set_q_n_t(2, 13, 8);
    get_q_n_t(&q, &n, &t);

    /*
    q = 2;
    t = 8;
    n = 13;
    */
    ciphertext_len = text_to_encrypt_len + t;
    ciphertext = malloc(ciphertext_len);
    if (ciphertext == NULL) {
        printf("malloc failed for ciphertext\n");
        return 1;
    }

    memcpy(ciphertext, text_to_encrypt, text_to_encrypt_len); //in-place encryption, so pass ciphertext which is of size "text_to_encrypt_len + t"
    memcpy(text_to_encrypt_bkup, text_to_encrypt, text_to_encrypt_len); //in-place encryption, so pass ciphertext which is of size "text_to_encrypt_len + t"
    ret = CCM_encrypt_and_auth(key, nonce, aad, aad_len, ciphertext, text_to_encrypt_len);
    if (ret == 0) {
        printf("test_aad_len_smaller_than_12 TEST FAILED in encryption\n");
        goto out;
    } else {
        ciphertext_len = ret;
    }

    ret = CCM_decrypt_and_auth(key, nonce, aad, aad_len, ciphertext, ciphertext_len);
    if (ret == 0) {
        printf("test_aad_len_smaller_than_12 TEST FAILED in decryption\n");
        ret = 1;
        goto out;
    } else {
        plaintext_len = ret;
    }

    if(memcmp(ciphertext, text_to_encrypt_bkup, plaintext_len) != 0) {
        printf("test_aad_len_smaller_than_12 ENCRYPTION TEST FAILED\n");
        ret = 1;
        goto out;
    }
    printf("test_aad_len_smaller_than_12 TEST PASSSED\n");
    ret = 0;
out:
    free(ciphertext);
    return ret;
}

int main(int argc, char *argv[])
{
    if ((argc == 2) && (strncmp(argv[1], "-h", 2) == 0)) {
        printf("./test_ccm -s for verifying all payload lengths and aad lengths and standard tests\n");
        printf("./test_ccm for standard tests\n");
        printf("./test_ccm -h for help\n");
        return 0;
    } else if ((argc == 2) && (strncmp(argv[1], "-s", 2) == 0)) {
        /* Following test takes almost an hour */
        if(test_verify_all_payload_lengths())
            return 1;
    
        if(test_all_aad_lengths())
            return 1;
    }

    if(test_example1())
        return 1;

    if(test_big_aad_example())
        return 1;

   if(test_verify_handful_payload_lengths())
        return 1;

    if(test_handful_aad_lengths())
        return 1;

    if (test_aad_len_smaller_than_12())
        return 1;

    return 0;
}
#endif /* NOT_USED */
