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
use convert_case::{Case, Casing};
use proc_macro2::TokenStream;
use quote::ToTokens;
use syn::parse_quote;

use super::{
    command_class_handler_composer::CommandClassHandlerComposer,
    init_handler_composer::InitHandlerComposer,
};

macro_rules! get_rvalue_handler_expr {
    ($handler_name: ident, $enabled: ident) => {
         match $enabled {
            true => {
                let f = $handler_name.as_ref().unwrap().get_function_name();
                parse_quote!(Some(#f))
            },
            false => parse_quote!(None),
        }
    };
}

pub struct CommandClassComposer {
    init_composer: InitHandlerComposer,
    on_support_handler: Option<CommandClassHandlerComposer>,
    on_control_handler: Option<CommandClassHandlerComposer>,
}

impl CommandClassComposer {
    pub fn new(
        command_class_type: &syn::TypePath,
        command_class_type_id: syn::TypePath,
        command_class_name: syn::TypePath,
        comments: syn::TypePath,
        version: syn::TypePath,
        minimal_scheme: syn::TypePath,
        manual_security_validation: syn::TypePath,
        has_config_support: bool,
        has_config_control: bool,
    ) -> Self {
        let instance_name: syn::Ident = quote::format_ident!(
            "{}",
            command_class_type
                .path
                .segments
                .last()
                .expect("types should have at least one segment")
                .ident
                .to_string()
                .to_case(Case::Snake)
        );

        let on_support_handler = has_config_support.then(|| {
            CommandClassHandlerComposer::new(
                instance_name.clone(),
                syn::parse_quote!(on_support),
                command_class_type.clone(),
            )
        });

        let on_control_handler = has_config_control.then(|| {
            CommandClassHandlerComposer::new(
                instance_name.clone(),
                syn::parse_quote!(on_control),
                command_class_type.clone(),
            )
        });

        // DRY
        let support_func = get_rvalue_handler_expr!(on_support_handler, has_config_support);
        let control_func = get_rvalue_handler_expr!(on_control_handler, has_config_control);

        let init_composer = InitHandlerComposer::new(
            instance_name.clone(),
            support_func,
            control_func,
            command_class_type_id,
            command_class_name,
            command_class_type.clone(),
            minimal_scheme,
            comments,
            version,
            manual_security_validation,
        );

        CommandClassComposer {
            init_composer,
            on_control_handler,
            on_support_handler,
        }
    }

    pub fn get_init_func_name(&self) -> syn::Ident {
        self.init_composer.get_func_name()
    }
}

impl ToTokens for CommandClassComposer {
    fn to_tokens(&self, tokens: &mut TokenStream) {
        self.init_composer.to_tokens(tokens);
        self.on_control_handler.to_tokens(tokens);
        self.on_support_handler.to_tokens(tokens);
    }
}
