/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @defgroup zwave_s2_nonce_management Security 2 Nonce Management
 * @ingroup zwave_transports
 * @brief Allows to store and restore S2 SPAN and MPAN states.
 *
 * This component provides an API to store and restore S2 SPAN and MPAN.
 * This should only be used at initialization or exit.
 *
 * @{
 */

#ifndef ZWAVE_S2_NONCE_MANAGEMENT_H
#define ZWAVE_S2_NONCE_MANAGEMENT_H

#include "sl_status.h"
#include "zwave_node_id_definitions.h"
#include "zwave_controller_types.h"

/// Length of the key in the CTR_DRBG
#define CTR_DRBG_KEY_LENGTH 16
/// Length of the internal state of the CTR_DRBG
#define CTR_DRBG_INTERNAL_STATE_LENGTH 16
/// Length of the inner state of an MPAN.
#define MPAN_INNER_STATE_LENGTH 16

/**
 * @brief Structure holding a SPAN entry.
 * Refer to LibS2 for field descriptions.
 */
typedef struct span_entry {
  uint8_t df;
  uint8_t working_state[CTR_DRBG_INTERNAL_STATE_LENGTH];
  uint8_t key[CTR_DRBG_KEY_LENGTH];
  uint8_t rx_sequence;
  uint8_t tx_sequence;
  uint8_t class_id;
} span_entry_t;

/**
 * @brief Structure holding an MPAN entry.
 * Refer to LibS2 for field descriptions.
 */
typedef struct mpan_entry {
  uint8_t inner_state[MPAN_INNER_STATE_LENGTH];
  zwave_node_id_t owner_node_id;
  uint8_t group_id;
  uint8_t class_id;
} mpan_entry_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fetches a Span entry object from the S2 SPAN table
 *
 * If the SPAN is not fully established, the data will not be returned.
 *
 * @param node_id [in]    Z-Wave Remote NodeID for that SPAN entry
 * @param span_data [out]  S2 Span data to be copied in the span table
 *
 * @returns SL_STATUS_OK if the span_data was found and returned,
 * SL_STATUS_NOT_FOUND otherwise.
 */
sl_status_t zwave_s2_get_span_data(zwave_node_id_t node_id,
                                   span_entry_t *span_data);

/**
 * @brief Copies a Span entry object into an entry of the S2 SPAN table
 *
 * @param span_data [in]  S2 Span data to be copied in the span table
 * @param node_id         Z-Wave Remote NodeID for that SPAN entry
 */
void zwave_s2_set_span_table(zwave_node_id_t node_id,
                             const span_entry_t *span_data);

/**
 * @brief Fetches an MPAN entry object from the S2 MPAN table
 *
 * If the MPAN is not fully established, the data will not be returned.
 *
 * @param owner_node_id [in]  The Z-Wave NodeID owning the MPAN group.
 * @param group_id [in]       The Z-Wave Group ID.
 * @param mpan_data [out]     The MPAN data.
 *
 * @returns SL_STATUS_OK if the mpan_data was found and returned,
 * SL_STATUS_NOT_FOUND otherwise.
 */
sl_status_t zwave_s2_get_mpan_data(zwave_node_id_t owner_node_id,
                                   zwave_multicast_group_id_t group_id,
                                   mpan_entry_t *mpan_data);

/**
 * @brief Copies an MPAN entry object into an entry of the S2 MPAN table
 *
 * @param owner_node_id [in]  The Z-Wave NodeID owning the MPAN group.
 * @param group_id [in]       The Z-Wave Group ID.
 * @param mpan_data [in]      S2 MPAN data to be copied in the MPAN table
 */
void zwave_s2_set_mpan_data(zwave_node_id_t owner_node_id,
                            zwave_multicast_group_id_t group_id,
                            const mpan_entry_t *mpan_data);
/**
 * @brief Resets the SPAN for node_id
 * by setting its state to SPAN_NOT_USED
 *
 * @param node_id [in]  The Z-Wave NodeID with which the SPAN needs to be reset
 */
void zwave_s2_reset_span(zwave_node_id_t node_id);

/**
 * @brief Resets the MPAN entry matching owner_node_id and group_id
 * by setting its state to MPAN_NOT_USED
 *
 * @param owner_node_id [in]  The Z-Wave NodeID owning the MPAN group.
 * @param group_id [in]       The Z-Wave Group ID.
 */

void zwave_s2_reset_mpan(zwave_node_id_t owner_node_id,
                         zwave_multicast_group_id_t group_id);
#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S2_NONCE_MANAGEMENT_H
        /** @} end zwave_s2_nonce_management */
