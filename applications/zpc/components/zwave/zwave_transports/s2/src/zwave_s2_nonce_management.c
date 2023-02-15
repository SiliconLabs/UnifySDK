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
#include "zwave_s2_nonce_management.h"
#include "zwave_s2_internal.h"

// Includes from LibS2
#include "s2_protocol.h"

// Generic includes
#include <string.h>

// Includes from the Z-Wave Controller
#include "zwave_network_management.h"
#include "sl_log.h"

#define LOG_TAG "zwave_s2_nonce_management"

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Copies a Span entry object into an entry of the S2 Context SPAN table
 *
 * We assume here that we copy SPANS that are in the SPAN_NEGOTIATED state.
 *
 * @param entry [out]     S2 SPAN table entry pointer
 * @param span_data [in]  S2 Span data to be copied in the span table
 * @param node_id [in]    Z-Wave Remote NodeID for that SPAN entry
 */
static void zwave_s2_configure_span_table_entry(struct SPAN *entry,
                                                const span_entry_t *span_data,
                                                zwave_node_id_t node_id)
{
  entry->d.rng.df = span_data->df;
  memcpy(entry->d.rng.k, span_data->key, CTR_DRBG_KEY_LENGTH);
  memcpy(entry->d.rng.v,
         span_data->working_state,
         CTR_DRBG_INTERNAL_STATE_LENGTH);
  entry->rx_seq   = span_data->rx_sequence;
  entry->tx_seq   = span_data->tx_sequence;
  entry->lnode    = zwave_network_management_get_node_id();
  entry->rnode    = node_id;
  entry->class_id = span_data->class_id;
  entry->state    = SPAN_NEGOTIATED;
}

/**
 * @brief Copies a MPAN entry object into an entry of the S2 MPAN table
 *
 * We assume here that we copy MPANs that are in the MPAN_SET state.
 *
 * @param entry [out]     S2 MPAN table entry pointer
 * @param mpan_data [in]  S2 MPAN data to be copied in the MPAN table
 * @param owner_node_id [in]    Z-Wave Remote NodeID for that MPAN entry
 */
static void zwave_s2_configure_mpan_table_entry(struct MPAN *entry,
                                                const mpan_entry_t *mpan_data,
                                                zwave_node_id_t owner_node_id)
{
  entry->class_id = mpan_data->class_id;
  entry->group_id = mpan_data->group_id;
  entry->owner_id = owner_node_id;
  entry->state    = MPAN_SET;
  memcpy(entry->inner_state, mpan_data->inner_state, MPAN_INNER_STATE_LENGTH);
}

///////////////////////////////////////////////////////////////////////////////
// Public API
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_s2_get_span_data(zwave_node_id_t node_id,
                                   span_entry_t *span_data)
{
  for (size_t i = 0; i < SPAN_TABLE_SIZE; i++) {
    if (s2_ctx->span_table[i].state == SPAN_NEGOTIATED
        && s2_ctx->span_table[i].rnode == node_id) {
      span_data->df = s2_ctx->span_table[i].d.rng.df;
      memcpy(span_data->key,
             s2_ctx->span_table[i].d.rng.k,
             CTR_DRBG_KEY_LENGTH);
      memcpy(span_data->working_state,
             s2_ctx->span_table[i].d.rng.v,
             CTR_DRBG_INTERNAL_STATE_LENGTH);
      span_data->rx_sequence = s2_ctx->span_table[i].rx_seq;
      span_data->tx_sequence = s2_ctx->span_table[i].tx_seq;
      span_data->class_id    = s2_ctx->span_table[i].class_id;
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

void zwave_s2_reset_span(zwave_node_id_t node_id)
{
  for (size_t i = 0; i < SPAN_TABLE_SIZE; i++) {
    if (s2_ctx->span_table[i].rnode == node_id) {
      s2_ctx->span_table[i].state = SPAN_NOT_USED;
      sl_log_debug(LOG_TAG,
                   "Success with reset of SPAN with NodeID: %d\n",
                   node_id);
      return;
    }
  }
}

void zwave_s2_reset_mpan(zwave_node_id_t owner_node_id,
                                  zwave_multicast_group_id_t group_id)
{
  for (size_t i = 0; i < MPAN_TABLE_SIZE; i++) {
    if (s2_ctx->mpan_table[i].owner_id == owner_node_id
        && s2_ctx->mpan_table[i].group_id == group_id) {
      s2_ctx->mpan_table[i].state = MPAN_NOT_USED;
      sl_log_debug(
        LOG_TAG,
        "Success with reset of MPAN with NodeID: %d and GroupID: %d\n",
        owner_node_id,
        group_id);
      return;
    }
  }
}

void zwave_s2_set_span_table(zwave_node_id_t node_id,
                             const span_entry_t *span_data)
{
  for (size_t i = 0; i < SPAN_TABLE_SIZE; i++) {
    if (s2_ctx->span_table[i].rnode == node_id) {
      zwave_s2_configure_span_table_entry(&s2_ctx->span_table[i],
                                          span_data,
                                          node_id);
      return;
    }
  }
  // If we did not find an entry with the NodeID we are looking,
  // use an empty slot
  for (size_t i = 0; i < SPAN_TABLE_SIZE; i++) {
    if (s2_ctx->span_table[i].state == SPAN_NOT_USED) {
      zwave_s2_configure_span_table_entry(&s2_ctx->span_table[i],
                                          span_data,
                                          node_id);
      return;
    }
  }

  // If we get here, we cannot accept more resources.
}

sl_status_t zwave_s2_get_mpan_data(zwave_node_id_t owner_node_id,
                                   zwave_multicast_group_id_t group_id,
                                   mpan_entry_t *mpan_data)
{
  for (size_t i = 0; i < MPAN_TABLE_SIZE; i++) {
    if ((s2_ctx->mpan_table[i].state == MPAN_SET)
        && (s2_ctx->mpan_table[i].group_id == group_id)
        && (s2_ctx->mpan_table[i].owner_id == owner_node_id)) {
      mpan_data->class_id      = s2_ctx->mpan_table[i].class_id;
      mpan_data->group_id      = s2_ctx->mpan_table[i].group_id;
      mpan_data->owner_node_id = s2_ctx->mpan_table[i].owner_id;

      memcpy(mpan_data->inner_state,
             s2_ctx->mpan_table[i].inner_state,
             MPAN_INNER_STATE_LENGTH);
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

void zwave_s2_set_mpan_data(zwave_node_id_t owner_node_id,
                            zwave_multicast_group_id_t group_id,
                            const mpan_entry_t *mpan_data)
{
  for (size_t i = 0; i < MPAN_TABLE_SIZE; i++) {
    if ((s2_ctx->mpan_table[i].group_id == group_id)
        && (s2_ctx->mpan_table[i].owner_id == owner_node_id)) {
      zwave_s2_configure_mpan_table_entry(&s2_ctx->mpan_table[i],
                                          mpan_data,
                                          owner_node_id);
      return;
    }
  }
  // If we did not find an entry with the GroupID/Owner ID that we are looking for,
  // use an empty slot
  for (size_t i = 0; i < MPAN_TABLE_SIZE; i++) {
    if (s2_ctx->mpan_table[i].state == MPAN_NOT_USED) {
      zwave_s2_configure_mpan_table_entry(&s2_ctx->mpan_table[i],
                                          mpan_data,
                                          owner_node_id);
      return;
    }
  }
  // If we get here, we cannot accept more resources.
}
