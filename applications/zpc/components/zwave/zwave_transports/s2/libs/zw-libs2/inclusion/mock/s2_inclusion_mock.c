/**
 * @file s2_inclusion_mock.c
 * @copyright 2022 Silicon Laboratories Inc.
 */
#include <s2_protocol.h>

void s2_inclusion_challenge_response(struct S2 *p_context, uint8_t include, const uint8_t* p_response, uint8_t responseLength)
{

}

void s2_inclusion_notify_timeout(struct S2 *p_context)
{

}

void s2_inclusion_joining_start(struct S2 *p_context, s2_connection_t *p_connection, uint8_t csa)
{

}

void s2_inclusion_abort(struct S2 *p_context)
{

}

uint8_t s2_inclusion_init(uint8_t schemes, uint8_t curves, uint8_t keys)
{
  return 1;
}

void s2_inclusion_set_event_handler(s2_event_handler_t evt_handler)
{

}

void s2_inclusion_neighbor_discovery_complete(struct S2 *p_context)
{

}
