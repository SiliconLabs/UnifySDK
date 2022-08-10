///////////////////////////////////////////////////////////////////////////////
// # License
// <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
///////////////////////////////////////////////////////////////////////////////
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
///////////////////////////////////////////////////////////////////////////////

use proc_macro2::TokenStream;
use quote::ToTokens;

pub struct CommandClassHandlerComposer {
    func_name: syn::Ident,
    member_name: syn::Ident,
    cc_type: syn::TypePath,
}

impl CommandClassHandlerComposer {
    pub fn new(instance_name: syn::Ident, member_name: syn::Ident, cc_type: syn::TypePath) -> Self {
        let func_name = quote::format_ident!("handle_{}_{}", instance_name, member_name);
        CommandClassHandlerComposer {
            func_name,
            member_name,
            cc_type,
        }
    }

    pub fn get_function_name(&self) -> syn::Ident {
        self.func_name.clone()
    }
}

impl ToTokens for CommandClassHandlerComposer {
    fn to_tokens(&self, tokens: &mut TokenStream) {
        let func_name = &self.func_name;
        let cc_type = &self.cc_type;
        let member_name = &self.member_name;
        let output = quote::quote! {
            #[doc(hidden)]
            #[no_mangle]
            unsafe extern "C" fn #func_name(
                connection: *const crate::zwave_controller_sys::zwave_controller_connection_info_t,
                frame_data: *const u8,
                frame_length: u16,
            ) -> sl_status_t {
                let rust_buf = core::slice::from_raw_parts(frame_data, frame_length as usize);
                use std::convert::TryFrom;
                match #cc_type::try_from(rust_buf) {
                    Ok(cc_frame) => {
                        match cc_frame.#member_name(&*connection) {
                            Ok(_) => unify_sl_status_sys::SL_STATUS_OK,
                            Err(sl_status) => sl_status
                        }
                    },
                    Err(_) => 1,
                }
            }
        };
        output.to_tokens(tokens);
    }
}
