
#include "zigpc_gateway_notify.h"
#include "zigpc_binding_response.h"
#include "zigpc_binding_setup.h"

sl_status_t zigpc_binding_setup()
{
    //register observer for bind/unbind response
    sl_status_t status = 
        zigpc_gateway_register_observer(
                ZIGPC_GATEWAY_NOTIFY_BIND_UNBIND_RESPONSE ,
                zigpc_binding_bind_unbind_callback);

    return status;
}

int zigpc_binding_shutdown(void)
{

    //unregister observer
    zigpc_gateway_unregister_observer(
            ZIGPC_GATEWAY_NOTIFY_BIND_UNBIND_RESPONSE,
            zigpc_binding_bind_unbind_callback);

    return 0;
}
