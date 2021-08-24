/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file attribute_resolver_rule_test.h
 * @brief TODO: Write brief for attribute_resolver_rule_test
 *
 * TODO: Write component description for attribute_resolver_rule_test
 *
 * @{
 */
 
#ifndef ATTRIBUTE_RESOLVER_RULE_TEST_H
#define ATTRIBUTE_RESOLVER_RULE_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "attribute_store.h"
#include "clock.h"
 sl_status_t get0_func(attribute_store_node_t node, uint8_t* frame, uint16_t *frame_len);
 sl_status_t get1_func(attribute_store_node_t node, uint8_t* frame, uint16_t *frame_len);
 sl_status_t set0_func(attribute_store_node_t node, uint8_t* frame, uint16_t *frame_len);
 sl_status_t set1_func(attribute_store_node_t node, uint8_t* frame, uint16_t *frame_len);

void rule_complete(attribute_store_node_t node,clock_time_t tx_time);


#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_RESOLVER_RULE_TEST_H
/** @} end attribute_resolver_rule_test */
