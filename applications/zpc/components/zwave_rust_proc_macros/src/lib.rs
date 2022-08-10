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

mod command_class_composer;
mod extern_c_hook;
mod trait_check;
mod version_updated_glue;
mod zwave_handler_glue;

use proc_macro::TokenStream;
use quote::ToTokens;
use trait_check::TraitCheck;
use version_updated_glue::*;
use zwave_handler_glue::*;

const ZWAVE_HANDLER_TRAIT_IDENT: &'static str = "CommandClassConfigurationTrait";
const ZWAVE_VERSION_TRAIT_IDENT: &'static str = "ZwaveVersionChangedTrait";

/// This macro procedurally generates all the boilerplate required to implement
/// a command class in rust. More specifically, this macro creates an adapter
/// directly on top of the C interface of the `unify_zwave_command_handler`
/// component. This means that command classes implemented with this macro
/// behave exactly the same as the command classes implemented in C.
///
/// # Modes
///
/// This macro supports 2 modes:
/// * `Zwave handler` configuration mode
/// * `On attribute version` mode
///
/// Which mode is selected depends on which trait interface this macro is
/// placed.
///
/// ## `Zwave handler` mode
///
/// In this mode the zwave handler gets setup using configuration that is
///  provided by the
/// [`CommandClassConfigurationTrait`](rust_command_class_frame_types::CommandClassConfigurationTrait).
/// Therefore the macro needs to be placed on top of the
/// [`CommandClassConfigurationTrait`] trait `impl` block. After filling
/// out the [`CommandClassConfigurationTrait`] your successful setup an
/// rust command class.
///
/// ### Example
///
/// ```rust, ignore
/// # use zwave_command_handler_sys::*;
/// # use zwave_controller_sys::*;
/// # use rust_command_class_frame_types::zwave_command_class_interface::zwave_command_handler_trait::CommandClassConfigurationTrait;
/// # use zwave_rust_proc_macros::zwave_command_class;
///
/// struct FooFrame;
///
/// #[zwave_command_class]
/// impl ZwaveVersionChangedTrait for FooFrame {
///     const COMMAND_CLASS_NAME:&'static str = "Foo Command Class";
///     const MINIMAL_SCHEME: zwave_controller_encapsulation_scheme_t =
///         zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
///     const COMMENTS: &'static str = "Foo frames actually don`t exits";
///     const MANUAL_SECURITY_VALIDATION: bool = true;
///     const INCLUDE_SUPPORT: bool = false;
///     const INCLUDE_CONTROL: bool = false;
///
///     fn on_init() -> sl_status_t{
///         todo!()
///     }
/// }
///
/// ```
///
/// ## `On attribute version` mode
///
/// In this mode the command class get's registered to the unify attribute store
/// with the provided `ATTRIBUTE_STORE_TYPE`. Attribute version updates can
/// happen every time when, for instance, the device just got an OTA update or
/// the network get a reset. In this case the function
/// [on_attribute_version](rust_command_class_frame_types::ZwaveVersionChangedTrait::on_attribute_version)
/// is called.
///
/// This macro is to be put on to of an [`ZwaveVersionChangedTrait`] trait impl
/// block
///
/// ### Example
///
///
/// ```rust, ignore
/// # use zwave_command_handler_sys::*;
/// # use unify_attribute_store_sys::*;
/// # use rust_command_class_frame_types::ZwaveVersionChangedTrait;
/// # use zwave_rust_proc_macros::zwave_command_class;
///
/// struct FooFrame;
///
/// #[zwave_command_class]
/// impl ZwaveVersionChangedTrait for FooFrame {
///     const ATTRIBUTE_STORE_TYPE: u32 = 0x1234;
///
///     fn on_attribute_version(ep: Attribute, version: u8) {
///         todo!()
///     }
/// }
///
/// ```
///
/// Placing the macro on other traits or places for that matter is not allowed
/// and will not compile
#[proc_macro_attribute]
pub fn zwave_command_class(_: TokenStream, item: TokenStream) -> TokenStream {
    // when in test, disable the generations of glue code
    if cfg!(test) {
        let mut out: proc_macro::TokenStream = "#[allow(dead_code)]".into_token_stream().into();
        out.extend(item);
        return out.into();
    }

    // On a high level, macro's work as followed;
    // 1. input tokens (aka `TokenStream`) gets parsed to a syntax tree.
    //    [syn::parse]
    // 2. Some modifications are done on the syntax tree's
    // 3. Syntax tree are de-parsed again in a `TokenStream`
    //
    // All token definitions of a rust syntax tree are defined in the syn crate.
    // `https://docs.rs/syn/latest/syn/`. You could parse the input TokenStream
    // as the the highest node in the tree, but a good practice is to open up
    // the documentation and see which specific types you need.
    //
    // Syn types have a `span` meta property attached to them. This tells the
    // compiler something about the location of a certain token. This is
    // extremely useful in a scenario we want to give feedback about a token, in
    // the case the user did something wrong. We can pass the span token to the
    // error msg, then the compiler will exactly highlight the offending token
    // the error occurred.

    let mut trait_check =
        TraitCheck::new(vec![ZWAVE_HANDLER_TRAIT_IDENT, ZWAVE_VERSION_TRAIT_IDENT]);

    let (ast_tree, trait_class_name) = match trait_check.verify_trait_implementation(item) {
        Ok(value) => value,
        Err(value) => return value,
    };

    let mut tokens = proc_macro2::TokenStream::new();
    ast_tree.to_tokens(&mut tokens);

    let self_name = trait_check.self_name.unwrap();
    if self_name.to_string() == ZWAVE_HANDLER_TRAIT_IDENT {
        // zwave handler mode
        zwave_handler_glue(&trait_class_name, &ast_tree).to_tokens(&mut tokens);
    } else if self_name.to_string() == ZWAVE_VERSION_TRAIT_IDENT {
        // attribute update mode
        version_updated_glue(&ast_tree.items, &trait_class_name).to_tokens(&mut tokens);
    } else {
        quote::quote! {
            compile_error!("trait checking instance broken");
        }
        .to_tokens(&mut tokens);
    }

    tokens.into()
}

/// This macro generates a C function. that is required to be compiled and
/// called on initialization in our C library. It contains initialization logic
/// of procedural macro's used in this particular module.
/// > failing to call this function prevents all procedural macros from working!
///
/// affected macro's:
/// * [#\[zwave_command_class\]](macro@zwave_command_class)
///
/// Make sure this macro is placed as last in the module to make sure it didn't
/// skip any preceding macro declarations.
//
/// This macro has a corresponding C header file that has to be called upon
/// initialization of the program
/// > C-symbol: `zwave_command_class_init_rust_handlers`
#[proc_macro]
pub fn extern_c_hook(_item: TokenStream) -> TokenStream {
    if cfg!(test) {
        return "".to_token_stream().into();
    } else {
        extern_c_hook::build_extern_c_hook().into()
    }
}
