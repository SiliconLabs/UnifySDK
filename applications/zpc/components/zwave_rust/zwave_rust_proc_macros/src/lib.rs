// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
#![doc(html_no_source)]

use convert_case::{Case, Casing};
use once_cell::sync::OnceCell;
use proc_macro::TokenStream;
use quote::TokenStreamExt;
use std::sync::Mutex;
use syn::{parse_macro_input, DeriveInput, Ident};

static INIT_HANDLERS: OnceCell<Mutex<Vec<String>>> = OnceCell::new();

/// By using this macro you declare your struct to be used by the zwave_command_classes component.
/// It expands and generates all the code needed so it can be properly linked into the zwave_command_classes library.
/// This means that the loading and calling of this handler is handed over to c code.
/// # Usage
/// * The macro is to be declared on the struct definition.
/// * It requires as argument a `ZwaveHandlerConfig` object.
/// * The struct needs to have the [Default] trait implemented
/// * The struct needs to have the `on_control`, `on_support` methods implemented.
/// the names are hardcoded for now. make sure to implement them exactly.
/// # Example
/// ```ignore
/// use crate::{
///    ZwaveConnectionInfo,
///    ZwaveHandlerConfig,
/// };
/// use zwave_controller_sys::zwave_controller_encapsulation_scheme_t;
///
/// const HANDLER_CONFIG: ZwaveHandlerConfig = ZwaveHandlerConfig {
///     minimal_scheme: zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
///     command_class: 0,
///     command_class_name: "Firmware Update",
///     comments: "",
///     version: 0,
///     manual_security_validation: false,
/// };
///
/// #[zwave_command_class(HANDLER_CONFIG)]
/// #[derive(Default)]
/// struct MyHandler {}
/// impl MyHandler {
///     pub fn on_control(&self, info: ZwaveConnectionInfo, frame: &[u8]) -> sl_status_t {
///         todo!()
///     }
///
///     pub fn on_support(&self, info: ZwaveConnectionInfo, frame: &[u8]) -> sl_status_t {
///         todo!()
///     }
///}
/// ```
///
/// # Custom data types
/// per default, frames are passed to the functions as [u8] slices. However there is an option to overload these arguments
/// with types of your own choice. It intended use is to build typesafe and easy to understand frames.
/// To do so, you need to 1. define your own enum, 2. implement the [From] trait for that enum, 3. update the
/// `on_control` and `on_support` methods with that custom enum.
/// ## Example
/// heres an example showing point 1 and 2:
/// ```ignore
/// pub struct CustomType;
///
/// // example of an custom frame type
/// // see how it can compose out of all different forms of data types
/// pub enum FooBarFrame {
///     None,
///     Foo { bar: u32 },
///     DynamicFrame(Vec<u32>),
///     FrameWithCustomData(CustomType),
/// }
///
/// // as described in step 2 the From trait needs to be implemented
/// // This is where the parsing and conversion of the raw u8 slice happens!
/// impl From<&[u8]> for FooBarFrame {
///    fn from(_: &[u8]) -> Self {
///        FooBarFrame::None
///    }
/// }
/// ```
/// after definition, the thing that is left to do is to update the zwave handler.
/// using previous example:
/// ```ignore
/// pub fn on_control(&self, info: ZwaveConnectionInfo, frame: &[u8]) -> sl_status_t
/// ```
/// becomes
/// ```ignore
/// pub fn on_control(&self, info: ZwaveConnectionInfo, frame: FooBarFrame) -> sl_status_t
/// ```
/// # zwave_command_classes
/// This macro is part of the `zwave_command_classes` component in the zpc. It depends on interfaces of this crate.
/// this means that any definitions or interfaces that are changed inside `zwave_command_classes` should be updated in this
/// marco as well.
///
/// # initialize_zwave_command_classes
/// note that in order for this macro to work, [macro@initialize_zwave_command_classes] need to be declared in your crate. read more at its
/// function description.
#[proc_macro_attribute]
pub fn zwave_command_class(attr: TokenStream, item: TokenStream) -> TokenStream {
    // these are all tokens which are used to create the output code
    let original_code: proc_macro2::TokenStream = item.clone().into();
    let typ = parse_macro_input!(item as DeriveInput).ident;
    let config = quote::format_ident!("{}", attr.into_iter().next().unwrap().to_string());
    let snake_case = quote::format_ident!("{}", typ.to_string().to_case(Case::Snake));
    let control = quote::format_ident!("{}_control", &snake_case);
    let support = quote::format_ident!("{}_support", &snake_case);
    let control_handler =
        extern_c_handler(&snake_case, &quote::format_ident!("on_control"), &control);
    let support_handler =
        extern_c_handler(&snake_case, &quote::format_ident!("on_support"), &support);
    let init_func = quote::format_ident!("{}_init", &snake_case);
    let typ_str = typ.to_string();

    let output = quote::quote! {

        // we know that the handlers will be called from a single thread.
        // therefore we drop Send + Sync requirements for our static instance
        unsafe impl Send for #typ{}
        unsafe impl Sync for #typ{}

        // types inside OnceCells need to implement debug
        impl std::fmt::Debug for #typ {
            fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
                write!(f, #typ_str)
            }
        }

        // declare instance of command class handler statically.
        // its wrapped in an once_cell to workaround constant object initialisation requirement for statics.
        // Since we use it in unsafe and non threaded code, we can declare it mutable as well.
        static mut #snake_case: once_cell::sync::OnceCell<#typ>= once_cell::sync::OnceCell::new();

        #control_handler
        #support_handler

        #[doc(hidden)]
        pub unsafe fn #init_func() -> sl_status_t {
            static mut initialized: bool = false;
            // Handle multiple initializations e.g. when zwave_set_default is executed
            if initialized {
                return 0;
            }
            initialized = true;
            #snake_case.set(#typ::default()).unwrap();

            // fill c zwave_command_handler_sys_t and pass handler
            let new_handler = zwave_command_handler_sys::zwave_command_handler_t {
                support_handler: unify_proc_macro::as_extern_c!(#support),
                control_handler: unify_proc_macro::as_extern_c!(#control),
                minimal_scheme: #config.minimal_scheme,
                command_class: #config.command_class,
                command_class_name: #config.command_class_name.as_ptr() as *const std::os::raw::c_char,
                comments: #config.comments.as_ptr() as *const std::os::raw::c_char,
                version: #config.version,
                manual_security_validation: #config.manual_security_validation,
            };
            // Call my init
            let status = zwave_command_handler_sys::zwave_command_handler_register_handler(new_handler);
            #snake_case.get_mut().unwrap().on_init(status)
            //status
        }

        #original_code
    };

    append_init_handler(init_func);

    output.into()
}

fn extern_c_handler(
    name: &Ident,
    member_name: &Ident,
    func_name: &Ident,
) -> proc_macro2::TokenStream {
    quote::quote! {
        #[unify_proc_macro::generate_extern_c]
        fn #func_name(
            connection: *const zwave_controller_sys::zwave_controller_connection_info_t,
            data: &[u8],
        ) -> sl_status_t {
            use std::convert::TryInto;
            use unify_log_sys::{declare_app_name, log_warning};
            declare_app_name!("rust_command_classes");
            match data.try_into() {
                Ok(frame) => {
                    unsafe { #name.get_mut().unwrap().#member_name((*connection).into(), frame) }
                },
                Err(e) => {
                    log_warning!("could not convert incoming frame: {}", e);
                    1
                }
            }

        }
    }
}

/// this macro exports the initialisation of rust handlers to one C-function.
/// this symbol needs to be hooked up in C code in order for rust handlers to work.
///
/// make sure its called after declaration of the handlers tagged with:
/// [#\[zwave_command_class\]](macro@zwave_command_class)
///
/// C-symbol: `zwave_command_class_init_rust_handlers`
#[proc_macro]
pub fn initialize_zwave_command_classes(_item: TokenStream) -> TokenStream {
    // this function parses all initialisation functions and outputs them in one function.
    let mut init_funcs = quote::quote! {};
    let mutex = INIT_HANDLERS.get_or_init(|| Mutex::new(Vec::new()));
    if let Ok(ref mut list) = mutex.try_lock() {
        init_funcs.append_all((*list).iter().map(|fn_name| {
            let declr_mod = quote::format_ident!("{}", &fn_name[..fn_name.len() - 5]);
            let declr = quote::format_ident!("{}", &fn_name);
            quote::quote! {
                status |= #declr_mod::#declr();
            }
        }));
    }

    let expanded = quote::quote! {
        #[no_mangle]
        pub unsafe extern "C" fn zwave_command_class_init_rust_handlers() -> u32 {
            let mut status = 0;
            #init_funcs


            status
        }
    };
    expanded.into()
}

fn append_init_handler(fn_name: Ident) {
    let mutex = INIT_HANDLERS.get_or_init(|| Mutex::new(Vec::new()));
    if let Ok(ref mut list) = mutex.try_lock() {
        (*list).push(fn_name.to_string());
    }
}
