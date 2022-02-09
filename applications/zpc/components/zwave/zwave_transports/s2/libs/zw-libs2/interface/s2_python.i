/* File: example.i */
%module s2

%{
#define SWIG_FILE_WITH_INIT
#include "s2.h"

struct callbacks{
	void (*S2_send_done_event)(struct S2* ctxt, s2_tx_status_t status);	
	void (*S2_msg_received_event)(struct S2* ctxt,s2_connection_t* src , uint8_t* buf, uint16_t len);
	uint8_t (*S2_send_frame)(struct S2* ctxt,s2_connection_t* conn, uint8_t* buf, uint16_t len);
	void (*S2_set_timeout)(struct S2* ctxt, uint32_t interval);	
	void (*S2_get_hw_random)(uint8_t *buf, uint8_t len);
};

struct callbacks* cbs;

void S2_send_done_event(struct S2* ctxt, s2_tx_status_t status){
}

void S2_msg_received_event(struct S2* ctxt,s2_connection_t* src , uint8_t* buf, uint16_t len){
}

uint8_t S2_send_frame(struct S2* ctxt,s2_connection_t* conn, uint8_t* buf, uint16_t len){
  return 0;
}


void S2_set_timeout(struct S2* ctxt, uint32_t interval){
}

void S2_get_hw_random(uint8_t *buf, uint8_t len) {
}

void S2_set_callbacks(struct callbacks* cb) {
  cbs = cb;
}

%}


void
S2_init_ctx(struct S2* ctx, uint8_t* net_key, const uint8_t* s2_classes,
    uint8_t n_s2_classes,node_t nodeid, uint32_t home);

uint8_t S2_send_data(struct S2* ctx, s2_connection_t* dst ,uint8_t* buf, uint16_t len );

void S2_set_callbacks(struct callbacks* cb);

struct callbacks{
	void (*S2_send_done_event)(struct S2* ctxt, s2_tx_status_t status);	
	void (*S2_msg_received_event)(struct S2* ctxt,s2_connection_t* src , uint8_t* buf, uint16_t len);
	uint8_t (*S2_send_frame)(struct S2* ctxt,s2_connection_t* conn, uint8_t* buf, uint16_t len);
	void (*S2_set_timeout)(struct S2* ctxt, uint32_t interval);	
	void (*S2_get_hw_random)(uint8_t *buf, uint8_t len);
};

