
#include <list>
#include <map>
#include <algorithm>

#include "sl_status.h"
#include "sl_log.h"
#include "zigpc_common_zigbee.h"

#include "zigpc_datastore.hpp"
#include "zigpc_common_util.hpp"

typedef std::list<zigbee_binding_t> binding_table; 

static std::map<zigbee_eui64_uint_t, binding_table> binding_map; //NOSONAR - needs to be non-const to work properly

sl_status_t zigpc_datastore::binding::add_binding( 
        zigbee_eui64_uint_t source,
        zigbee_binding_t binding)
{
    binding_table specific_binding_table = binding_map[source];
    specific_binding_table.push_back(binding);
    binding_map[source] = specific_binding_table;

    return SL_STATUS_OK;
}

sl_status_t zigpc_datastore::binding::remove_binding(
        zigbee_eui64_uint_t source,
        zigbee_binding_t binding)
{
    binding_table specific_binding_table = binding_map[source];
   
    auto binding_index = 
        std::find(
                specific_binding_table.begin(),
                specific_binding_table.end(),
                binding);

    if( binding_index != specific_binding_table.end())
    {
        specific_binding_table.remove(binding);
    }

    return SL_STATUS_OK;
}

sl_status_t zigpc_datastore::binding::read_binding_table(
        zigbee_eui64_uint_t source,
        std::list<zigbee_binding_t>& binding_table_dest)
{
    sl_status_t status = SL_STATUS_OK;
    
    auto binding_table_index = binding_map.find(source); 

    if(binding_table_index != binding_map.end())
    {
        binding_table_dest = binding_map[source];
        status = SL_STATUS_OK;
    }
    else
    {
        status = SL_STATUS_FAIL;
    }

    return status;
}

size_t zigpc_datastore::binding::read_binding_count(zigbee_eui64_uint_t source)
{
    binding_table specific_binding_table = binding_map[source];
    return specific_binding_table.size();
}

