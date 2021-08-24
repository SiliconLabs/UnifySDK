#ifndef _S2_INCLUSION_INTERNAL_H_
#define _S2_INCLUSION_INTERNAL_H_

/**
 * These define the key slots used for the various S2 keys.
 * This information is shared between S2.c and s2_inclusion.c.
 */
#define UNAUTHENTICATED_KEY_SLOT    0       //< Slot for unauthenticated key. This value matches the ordering of security_key_t in ZW_security_api.h - do not change
#define AUTHENTICATED_KEY_SLOT      1       //< Slot for authenticated key. This value matches the ordering of security_key_t in ZW_security_api.h - do not change
#define ACCESS_KEY_SLOT             2       //< Slot for access key. This value matches the class_id returned by S2.c when decrypting with this key.
#define LR_AUTHENTICATED_KEY_SLOT   3       //< Slot for LR authenticated key. This value matches the class_id returned by S2.c when decrypting with this key.
#define LR_ACCESS_KEY_SLOT          4       //< Slot for LR access key. This value matches the ordering of security_key_t in ZW_security_api.h - do not change
#ifdef ZW_CONTROLLER
#define TEMP_KEY_SECURE             5       //< Value identifying index for the temporary key in S2 network key when transmitting secure frames.
#define NETWORK_KEY_SECURE          6       //< Value identifying index for the current network key in S2 network key when transmitting secure frames.
#else
#define TEMP_KEY_SECURE             0       //< Value identifying index for the temporary key in S2 network key when transmitting secure frames.
#define NETWORK_KEY_SECURE          1       //< Value identifying index for the current network key in S2 network key when transmitting secure frames.
#endif

#endif /*_S2_INCLUSION_INTERNAL_H_ */