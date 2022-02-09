/* © 2017 Silicon Laboratories Inc.
 */
/*
 * s2_classcmd.h
 *
 *  Created on: Oct 16, 2015
 *      Author: trasmussen
 */
 
#ifndef _S2_CLASSCMD_H_
#define _S2_CLASSCMD_H_

/**
 * \defgroup s2cls Constants used by libs2
 *
 * @{
 */
// NOTE: It is expected that those values should go into ZW_classcmd.h some way.
//       Naming of fields might in that case have minor changes.

/* Values used for Kex Report command */
//#define KEX_REPORT_PROPERTIES1_ECHO_BIT_MASK                                             0x01
//#define KEX_REPORT_PROPERTIES1_RESERVED_MASK                                             0xFE
//#define KEX_REPORT_PROPERTIES1_RESERVED_SHIFT                                            0x01

/* Values used for Kex Set command */
//#define KEX_SET_PROPERTIES1_ECHO_BIT_MASK                                                0x01
//#define KEX_SET_PROPERTIES1_RESERVED_MASK                                                0xFE
//#define KEX_SET_PROPERTIES1_RESERVED_SHIFT                                               0x01

/* Values used for Public Key Report command */
//#define PUBLIC_KEY_REPORT_PROPERTIES1_INCLUDING_NODE_BIT_MASK                            0x01
//#define PUBLIC_KEY_REPORT_PROPERTIES1_RESERVED_MASK                                      0xFE
//#define PUBLIC_KEY_REPORT_PROPERTIES1_RESERVED_SHIFT                                     0x01
/* Values used for Security 2 Transfer End command */
//#define SECURITY_2_TRANSFER_END_PROPERTIES1_KEY_REQUEST_COMPLETE_BIT_MASK                0x01
//#define SECURITY_2_TRANSFER_END_PROPERTIES1_KEY_VERIFIED_BIT_MASK                        0x02
//#define SECURITY_2_TRANSFER_END_PROPERTIES1_RESERVED_MASK                                0xFC
//#define SECURITY_2_TRANSFER_END_PROPERTIES1_RESERVED_SHIFT                               0x02

#define SECURITY_2_SCHEME_1_SUPPORT          0x02     //< Value denoting support for scheme 1 in the sheme support field.
#define SECURITY_2_SCHEME_SUPPORT_MASK       0x02     //< Mask containing all schemes.

#define SECURITY_2_SECURITY_2_CLASS_0        0x01     //< Value denoting support/request/grant for security 2 class 0 key.
#define SECURITY_2_SECURITY_2_CLASS_1        0x02     //< Value denoting support/request/grant for security 2 class 1 key.
#define SECURITY_2_SECURITY_2_CLASS_2        0x04     //< Value denoting support/request/grant for security 2 class 2 key.
#define SECURITY_2_SECURITY_2_CLASS_3        0x08     //< Value denoting support/request/grant for security 2 class 3 (KEY_CLASS_S2_AUTHENTICATED_LR) key.
#define SECURITY_2_SECURITY_2_CLASS_4        0x10     //< Value denoting support/request/grant for security 2 class 4 (KEY_CLASS_S2_ACCESS_LR) key.
#define SECURITY_2_SECURITY_0_NETWORK_KEY    0x80     //< Value denoting support/request/grant for security 0 network key.
#define SECURITY_2_KEY_S2_MASK               0x1F     //< Mask containing all S2 LR and S2 non-LR keys, but not S0
#define SECURITY_2_KEY_MASK                  0x87     //< Mask containing all non-LR keys.
#define SECURITY_2_KEY_MASK_INCL_LR          0x9F     //< Mask containing all keys.

#define KEX_REPORT_CURVE_25519               0x01     //< Value denoting curve25519 used in ECDH.
#define KEX_REPORT_CURVE_MASK                0x01     //< Mask containing all curves.
#define KEX_REPORT_CURVE_RESERVED_MASK       0xFE
#define KEX_REPORT_CURVE_RESERVED_SHIFT      0x01

#define SECURITY_2_ECHO_OFF                  0x00     //< Value denoting that this is not an echo frame.
#define SECURITY_2_ECHO_ON                   0x01     //< Value denoting that this is an echo frame.

#define SECURITY_2_CSA_ON                    0x02     //< Value denoting that csa was requested


#define SECURITY_2_JOINING_NODE              0x00     //< Value denoting that this frame originates from the joining node.
#define SECURITY_2_INCLUDING_NODE            0x01     //< Value denoting that this frame originates from the including node.

#define SECURITY_2_KEY_REQ_CONTINUE          0x00     //< Value denoting that joining node has not received all keys.
#define SECURITY_2_KEY_REQ_COMPLETE          0x01     //< Value denoting that joining node has received all requested keys.
#define SECURITY_2_KEY_VERIFIED              0x02     //< Value denoting that the verify frame could be successfully decrypted using the exchanged network key.

/** Following defines are related to security 2 command class frames. */
#define SECURITY_2_COMMAND_CLASS_POS         0 //< Position of the command class byte.
#define SECURITY_2_COMMAND_POS               1 //< Position of the command byte identifying the security 2 command.

/** Following defines are related to the KEX Set Frame. */
#define SECURITY_2_KEX_SET_ECHO_POS          2 //< Position of the echo field in the KEX set frame.
#define SECURITY_2_KEX_SET_CSA_POS          2 //< Position of the echo field in the KEX set frame.
#define SECURITY_2_KEX_SET_SCHEME_POS        3 //< Position of the echo field in the KEX set frame.
#define SECURITY_2_KEX_SET_CURVE_POS         4 //< Position of the echo field in the KEX set frame.
#define SECURITY_2_KEX_SET_KEYS_POS          5 //< Position of the echo field in the KEX set frame.


/** Following defines are related to the KEX Report Frame. Note: The KEX Report fields are identical to the KEX Set frame. */
#define SECURITY_2_KEX_REP_ECHO_POS          SECURITY_2_KEX_SET_ECHO_POS   //< Position of the echo field in the KEX report frame.
#define SECURITY_2_KEX_REP_SCHEME_POS        SECURITY_2_KEX_SET_SCHEME_POS //< Position of the echo field in the KEX report frame.
#define SECURITY_2_KEX_REP_CURVE_POS         SECURITY_2_KEX_SET_CURVE_POS  //< Position of the echo field in the KEX report frame.
#define SECURITY_2_KEX_REP_KEYS_POS          SECURITY_2_KEX_SET_KEYS_POS   //< Position of the echo field in the KEX report frame.

/** Following defines are related to the Public Key Frame set. */
#define SECURITY_2_PUB_KEY_INC_FLAG_POS      2 //< Position of the including node flag in the public key exchange frame.
#define SECURITY_2_PUB_KEY_KEY_POS           3 //< Position of the key in the public key exchange frame.

/** Following define is related to the Network Key Get Frame. */
#define SECURITY_2_NET_KEY_GET_REQ_KEY_POS   2 //< Position of the key requested field in the network key get frame.

/** Following defines are related to the network key report frame. */
#define SECURITY_2_NET_KEY_REP_GRANT_KEY_POS 2 //< Position of the key granted field in the network key report frame.
#define SECURITY_2_NET_KEY_REP_KEY_POS       3 //< Position of the key in the network key report frame.

/** Following define is related to the Network Transfer End Frame. */
#define SECURITY_2_TRANSFER_END_FLAGS_POS    2 //< Position of the flag field in the transfer end frame.

/** Following define is related to the KEX Fail frame. */
#define SECURITY_2_KEX_FAIL_FAIL_TYPE_POS    2 //< Position of the fail type field in the KEX fail frame.

/** Length definitions */
#define SECURITY_2_KEX_GET_LENGTH            2
#define SECURITY_2_KEX_REPORT_LENGTH         6
#define SECURITY_2_KEX_SET_LENGTH            6
#define SECURITY_2_PUB_KEY_LENGTH            35
#define SECURITY_2_NET_KEY_GET_LENGTH        3
#define SECURITY_2_NET_KEY_REPORT_LENGTH     19
#define SECURITY_2_NET_KEY_VERIFY_LENGTH     2
#define SECURITY_2_TRANSFER_END_LENGTH       3
#define SECURITY_2_KEX_FAIL_LENGTH           3

#define SECURITY_2_EC_PUBLIC_KEY_LENGTH      32

#define SECURITY_2_KEX_SET_RESERVED_MASK 0xFC //< Bitmask of the reserved bits in Kex Set
#define ZW_LR_MAX_NODE_ID     4000
#define ZW_LR_MIN_NODE_ID     256
#define IS_LR_NODE(nodeid) (((nodeid) >= ZW_LR_MIN_NODE_ID) && ((nodeid) <= ZW_LR_MAX_NODE_ID))

/**
 * @}
 */
#endif // _S2_CLASSCMD_H_
