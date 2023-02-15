///////////////////////////////////////////////////////////////////////////////
// # License
// <b>Copyright 2022  Silicon Laboratories Inc. www.silabs.com</b>
///////////////////////////////////////////////////////////////////////////////
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
//
///////////////////////////////////////////////////////////////////////////////
#![doc(html_no_source)]
unify_tools::include_binding!(
    concat!(env!("OUT_DIR"), "/binding.rs"),
    uic_attribute_resolver
);
use crate::uic_attribute_resolver::{attribute_resolver_function_t, sl_status_t};

pub fn attribute_resolver_register_rule(
    node_type: uic_attribute_resolver::attribute_store_type_t,
    set_func: attribute_resolver_function_t,
    get_func: attribute_resolver_function_t,
) -> sl_status_t {
    unsafe {
        uic_attribute_resolver::attribute_resolver_register_rule(node_type, set_func, get_func)
    }
}

pub fn is_node_pending_set_resolution(
    node: uic_attribute_resolver::attribute_store_node_t,
) -> bool {
    unsafe { uic_attribute_resolver::is_node_pending_set_resolution(node) }
}

pub fn attribute_resolver_restart_set_resolution(
    node: uic_attribute_resolver::attribute_store_node_t,
) {
    unsafe {
        let _ = uic_attribute_resolver::attribute_resolver_restart_set_resolution(node);
    }
}
