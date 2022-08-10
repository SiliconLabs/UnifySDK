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
//
///////////////////////////////////////////////////////////////////////////////
use proc_macro2::TokenStream;
use quote::{format_ident, ToTokens};

pub struct InitHandlerComposer {
    instance_name: syn::Ident,
    support_func: syn::Expr,
    control_func: syn::Expr,
    command_class_type_id: syn::TypePath,
    command_class_name: syn::TypePath,
    command_class_type: syn::TypePath,
    minimal_scheme: syn::TypePath,
    comments: syn::TypePath,
    version: syn::TypePath,
    manual_security_validation: syn::TypePath,
    init_func_name: syn::Ident,
}

impl InitHandlerComposer {
    pub fn new(
        instance_name: syn::Ident,
        support_func: syn::Expr,
        control_func: syn::Expr,
        command_class_type_id: syn::TypePath,
        command_class_name: syn::TypePath,
        command_class_type: syn::TypePath,
        minimal_scheme: syn::TypePath,
        comments: syn::TypePath,
        version: syn::TypePath,
        manual_security_validation: syn::TypePath,
    ) -> Self {
        let init_func_name = format_ident!("init_{}", &instance_name);

        InitHandlerComposer {
            instance_name,
            support_func,
            control_func,
            command_class_type_id,
            command_class_name,
            command_class_type,
            minimal_scheme,
            comments,
            version,
            manual_security_validation,
            init_func_name,
        }
    }

    pub fn get_func_name(&self) -> syn::Ident {
        self.init_func_name.clone()
    }

    fn compose_string_buffers(&self, tokens: &mut TokenStream) -> (syn::Ident, syn::Ident) {
        let cc_static_instance_name = format_ident!("cc_{}_name", &self.instance_name);
        let comments_static_instance_name = format_ident!("cc_{}_comment", &self.instance_name);

        let cc_static_instance = raw_static_buffer_null_terminated_tokens(
            &cc_static_instance_name,
            &self.command_class_name,
        );

        let comment_static_instance = raw_static_buffer_null_terminated_tokens(
            &comments_static_instance_name,
            &self.comments,
        );

        cc_static_instance.to_tokens(tokens);
        comment_static_instance.to_tokens(tokens);

        (cc_static_instance_name, comments_static_instance_name)
    }
}

impl ToTokens for InitHandlerComposer {
    fn to_tokens(&self, tokens: &mut proc_macro2::TokenStream) {
        let (cc_static_instance, comment_static_instance) = self.compose_string_buffers(tokens);
        let init_func_name = &self.init_func_name;
        let command_class_name = &self.command_class_name;
        let comment = &self.comments;
        let support_func = &self.support_func;
        let control_func = &self.control_func;
        let minimal_scheme = &self.minimal_scheme;
        let command_class_type_id = &self.command_class_type_id;
        let command_class_type = &self.command_class_type;
        let version = &self.version;
        let manual_security_validation = &self.manual_security_validation;

        let output = quote::quote! {
            use crate::rust_command_handlers::zwave_command_class_definitions::ZwaveCommandClassInfo;
                #[doc(hidden)]
                pub unsafe fn #init_func_name() -> sl_status_t {
                    static mut init: bool = false;
                    if init {
                        return 0;
                    } else {
                        init = true;
                    }

                    // copy the string literals to byte arrays so that we can null terminate them properly.
                    // the byte arrays are declared static. Since we need to make sure the lifetime of the buffers exceed
                    // the lifetime of the zwave_command_handler component as we pass a raw c_char pointer to the C interface.
                    #cc_static_instance[..#command_class_name.len()].copy_from_slice(#command_class_name.as_bytes());
                    #comment_static_instance[..#comment.len()].copy_from_slice(#comment.as_bytes());

                    let new_handler = crate::zwave_command_handler_sys::zwave_command_handler_t {
                        support_handler: #support_func,
                        control_handler: #control_func,
                        minimal_scheme: #minimal_scheme.into(),
                        command_class: #command_class_type_id,
                        command_class_name: #cc_static_instance.as_ptr() as *const std::os::raw::c_char,
                        comments: #comment_static_instance.as_ptr() as *const std::os::raw::c_char,
                        version: #version,
                        manual_security_validation: #manual_security_validation,
                    };

                    let status = crate::zwave_command_handler_sys::zwave_command_handler_register_handler(new_handler);
                    #command_class_type::on_init();
                    status
                }
        };
        output.to_tokens(tokens);
    }
}

fn raw_static_buffer_null_terminated_tokens<T: quote::ToTokens>(
    name: &syn::Ident,
    const_str: &T,
) -> TokenStream {
    // the debug message is printed into the source as a doc macro instead of the short version `//`
    // this way we can quote our tokens inside the message.
    let msg = format!(
        "Total buffer size is sizeof({}) + 1, to accommodate for the the null termination",
        name.to_string(),
    );

    let output = quote::quote! {
        #[doc = #msg]
        static mut #name: [u8; #const_str.len() + 1] = [0; #const_str.len()+1];
    };

    output.to_token_stream()
}
