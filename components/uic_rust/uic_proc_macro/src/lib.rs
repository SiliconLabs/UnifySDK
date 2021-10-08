use proc_macro::TokenStream;
use proc_macro2::Span;
use quote::{quote, ToTokens};
use syn::{
    parse_macro_input, punctuated::Punctuated, token, Block, ExprUnsafe, FnArg, PatIdent, Path,
    PathSegment, Stmt, Token,
};
/// This proc_macro generates an c wrapper function for a given rust function.
/// The purpose for this is to be able to easily pass it over ffi to C code.
/// # Example
/// ``` ignore
/// #[generate_extern_c]
/// fn my_rust_func() { println!("func!"); }
/// ```
/// Will expand to the following C signature:
/// ``` ignore
/// extern "C" fn my_rust_func_c(){
///   unsafe {
///       my_rust_func();
///   }
/// }
///
/// fn my_rust_func() { println!("func!"); }
/// ```
/// # Usage
/// The generated c function will be named `<function_name>_c`. See Example above
/// Make sure you pass the correct function name as callback argument in your extern c function.
///
/// # Supported arguments
/// this function will make sure the arguments get converted correctly from Rust types to C types.
/// For now, only some type conversions are implmenetend. these are the following:
/// * String
/// ``` ignore
/// fn string_arg(text: String);
/// // String gets converted as c_char pointer
/// extern "C" fn string_arg_c(text: *const ::os::raw::c_char);
/// ```
/// * &str
/// ``` ignore
/// fn str_arg(text: &str);
/// // &str gets converted as c_char pointer
/// extern "C" fn str_arg_c(text: *const ::os::raw::c_char);
/// ```
/// * &mut usize
/// ``` ignore
/// fn usize_arg(size: &mut usize);
/// // &mut usize gets converted as c_ulong pointer
/// extern "C" fn usize_arg_c(text: *const ::os::raw::c_ulong);
/// ```
/// * &mut u64
/// ``` ignore
/// fn u64_arg(size: u64);
/// // &mut u64 gets converted as c_ulong pointer
/// extern "C" fn u64_arg_c(text: *const ::os::raw::c_ulong);
/// ```
/// * &\[u8\]
/// ``` ignore
/// fn byte_slice_arg(slice: &[u8]);
/// // &\[u8\] gets converted as 2 arguments!
/// extern "C" fn byte_slice_arg_c(data: *u8, length: u16);
/// note that a byte slice gets expanded into 2 native c arguments!
/// * &\mut [u8\]
/// ``` ignore
/// fn byte_slice_arg(slice: &mut [u8]);
/// // &\[u8\] gets converted as 2 arguments!
/// extern "C" fn byte_slice_arg_c(data: *mut u8, length: u16);
/// ```
/// note that a byte slice gets expanded into 2 native c arguments!
/// # [as_extern_c]
/// This macro is to be used together with [as_extern_c]. This macro converts
/// your Rust function name to the correct extern C function name.
/// ``` ignore
/// // we have an arbitrary function that takes a c function as argument
/// extern "C" fn my_c_func(callback: Option<extern "C" fn()>);
/// // declare the callback we want to pass as c function:
/// #[generate_extern_c]
/// fn my_rust_callback(){ println("hello callback!");}
/// // use `as_extern_c` to fit the pieces together
/// fn main(){
///   my_c_func(as_extern_c!(my_rust_callback));
///}
#[proc_macro_attribute]
pub fn generate_extern_c(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let clone = item.clone();
    let mut native_function = parse_macro_input!(clone as syn::ItemFn);
    transmute_to_c_wrapper(&mut native_function);

    let original_code: proc_macro2::TokenStream = item.into();
    let output = quote::quote! {
      #[no_mangle]
      unsafe extern "C" #native_function
      #original_code
    };
    // let str = output.to_string();
    // let output = quote! {compile_error!(#str)};
    output.into()
}

/// macro to be used in combination with [generate_extern_c]
/// This macro converts a given Rust function to the correct extern C function name.
/// See [generate_extern_c] for more information.
#[proc_macro]
pub fn as_extern_c(item: TokenStream) -> TokenStream {
    let input = parse_macro_input!(item as syn::Ident);
    let new_name = quote::format_ident!("{}_c", input);
    let output = quote! {Some(#new_name)};
    output.into()
}

/// this function:
/// * renames the given function
/// * replaces body with code required to call the rust function.
fn transmute_to_c_wrapper(func: &mut syn::ItemFn) {
    func.block.stmts.clear();
    func.block.stmts.push(compose_actual_call(func));
    func.sig.ident = quote::format_ident!("{}_c", func.sig.ident);
    convert_args_to_c_types(&mut func.sig.inputs);
}

/// this function replaces rust types to c types in a punctuated list.
/// this list is used to update a function signature with c argument types.
fn convert_args_to_c_types(args: &mut Punctuated<FnArg, Token![,]>) {
    let mut inserts: Vec<(usize, syn::PatType)> = Vec::new();
    for n in 0..args.len() {
        // box_patterns matching is unfortunatly only available
        // in rust nightly
        if let FnArg::Typed(path_type) = &mut args[n] {
            match path_type.ty.to_token_stream().to_string().as_str() {
                "& str" | "String" => {
                    path_type.ty = Box::new(char_ptr());
                }
                // "usize" | "u64" => {
                //     path_type.ty = Box::new(size_t());
                // }
                "&[u8]" | "& [u8]" => {
                    let mut size_clone = path_type.clone();
                    size_clone.ty = Box::new(u16_t());
                    size_clone.pat = Box::new(syn::Pat::Ident(PatIdent {
                        attrs: Vec::new(),
                        by_ref: None,
                        mutability: None,
                        ident: quote::format_ident!("size{}", n),
                        subpat: None,
                    }));
                    inserts.push((n + 1, size_clone));
                    path_type.ty = Box::new(u8_ptr(false));
                }
                "&mut [u8]" | "& mut [u8]" => {
                    let mut size_clone = path_type.clone();
                    size_clone.ty = Box::new(u16_t());
                    size_clone.pat = Box::new(syn::Pat::Ident(PatIdent {
                        attrs: Vec::new(),
                        by_ref: None,
                        mutability: None,
                        ident: quote::format_ident!("size{}", n),
                        subpat: None,
                    }));
                    inserts.push((n + 1, size_clone));
                    path_type.ty = Box::new(u8_ptr(true));
                }
                _ => {
                    // do not change type of not supported types
                    //println!("ignore type {} for argument parsing", not_implemented);
                }
            }
        }
    }

    for (k, v) in inserts.into_iter().rev() {
        args.insert(k, FnArg::Typed(v));
    }
}

/// this function is responsible for injecting the correct conversion methods for
/// the given functions argument types, From C types to Rust types.
/// If there is no conversion know, just pass it along.
/// returns a punctuaded list of code. will be used to pass to the actual rust function
fn conversion_code_of_arguments(
    args: &Punctuated<FnArg, Token![,]>,
) -> Punctuated<proc_macro2::TokenStream, Token![,]> {
    let mut list: Punctuated<proc_macro2::TokenStream, Token![,]> = Punctuated::new();
    for n in 0..args.len() {
        if let FnArg::Typed(path_type) = &args[n] {
            let name = path_type.pat.to_token_stream();
            match path_type.ty.to_token_stream().to_string().as_str() {
                "& str" => {
                    list.push(quote! {std::ffi::CStr::from_ptr(#name).to_str().unwrap()});
                }
                "String" => {
                    list.push(
                        quote! {std::ffi::CStr::from_ptr(#name).to_string_lossy().into_owned()},
                    );
                }
                "usize" | "u64" => {
                    list.push(quote! {std::convert::TryInto::try_into(#name).unwrap()});
                }
                "&[u8]" | "& [u8]" => {
                    let sizename = quote::format_ident!("size{}", n);
                    list.push(quote! {std::slice::from_raw_parts(#name, #sizename as usize)});
                }
                "&mut [u8]" | "& mut [u8]" => {
                    let sizename = quote::format_ident!("size{}", n);
                    list.push(
                        quote! {&mut std::slice::from_raw_parts_mut(#name, #sizename as usize)},
                    );
                }
                _ => {
                    list.push(quote! {#name});
                }
            }
        }
    }
    list
}

/// creates the body of the c wrapper function.
/// Call to the rust function is wrapped in an unsafe block because of the
// unsafe c to rust conversions.
fn compose_actual_call(func: &syn::ItemFn) -> Stmt {
    let converted_args_list = conversion_code_of_arguments(&func.sig.inputs);
    let name = func.sig.ident.clone();
    let body = quote! {
        #name(#converted_args_list)
    };

    syn::Stmt::Expr(syn::Expr::Unsafe(ExprUnsafe {
        attrs: Vec::new(),
        unsafe_token: token::Unsafe(Span::call_site()),
        block: Block {
            brace_token: token::Brace(Span::call_site()),
            stmts: vec![Stmt::Item(syn::Item::Verbatim(body))],
        },
    }))
}

fn char_ptr() -> syn::Type {
    let mut segments: Punctuated<PathSegment, Token![::]> = Punctuated::new();
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("std", Span::call_site()),
        arguments: syn::PathArguments::None,
    });
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("os", Span::call_site()),
        arguments: syn::PathArguments::None,
    });
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("raw", Span::call_site()),
        arguments: syn::PathArguments::None,
    });
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("c_char", Span::call_site()),
        arguments: syn::PathArguments::None,
    });

    syn::Type::Ptr(syn::TypePtr {
        star_token: token::Star(Span::call_site()),
        const_token: Some(token::Const(Span::call_site())),
        mutability: None,
        elem: Box::new(syn::Type::Path(syn::TypePath {
            qself: None,
            path: syn::Path {
                leading_colon: Some(token::Colon2(Span::call_site())),
                segments,
            },
        })),
    })
}

#[allow(dead_code)]
fn c_ulong_ptr() -> syn::Type {
    let mut segments: Punctuated<PathSegment, Token![::]> = Punctuated::new();
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("std", Span::call_site()),
        arguments: syn::PathArguments::None,
    });
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("os", Span::call_site()),
        arguments: syn::PathArguments::None,
    });
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("raw", Span::call_site()),
        arguments: syn::PathArguments::None,
    });
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("c_ulong", Span::call_site()),
        arguments: syn::PathArguments::None,
    });

    syn::Type::Ptr(syn::TypePtr {
        star_token: token::Star(Span::call_site()),
        const_token: Some(token::Const(Span::call_site())),
        mutability: None,
        elem: Box::new(syn::Type::Path(syn::TypePath {
            qself: None,
            path: syn::Path {
                leading_colon: Some(token::Colon2(Span::call_site())),
                segments,
            },
        })),
    })
}

fn u8_ptr(mutable: bool) -> syn::Type {
    let mut mutability = None;
    if mutable {
        mutability = Some(token::Mut(Span::call_site()));
    }

    let mut segments: Punctuated<PathSegment, Token![::]> = Punctuated::new();
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("u8", Span::call_site()),
        arguments: syn::PathArguments::None,
    });
    syn::Type::Ptr(syn::TypePtr {
        star_token: token::Star(Span::call_site()),
        const_token: None,
        mutability,
        elem: Box::new(syn::Type::Path(syn::TypePath {
            qself: None,
            path: syn::Path {
                leading_colon: None,
                segments,
            },
        })),
    })
}

fn u16_t() -> syn::Type {
    let mut segments: Punctuated<PathSegment, Token![::]> = Punctuated::new();
    segments.push(syn::PathSegment {
        ident: syn::Ident::new("u16", Span::call_site()),
        arguments: syn::PathArguments::None,
    });
    syn::Type::Path(syn::TypePath {
        qself: None,
        path: Path {
            leading_colon: None,
            segments,
        },
    })
}
