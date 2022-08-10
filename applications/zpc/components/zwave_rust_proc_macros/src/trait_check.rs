// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

use proc_macro::TokenStream;
use quote::ToTokens;
use syn::spanned::Spanned;

/// This class checks an TokenStream for validity and returns the `trait name`
/// ident + the `trait implementer name` ident. a TokenStream is to be
/// considered valid if:
/// * the type of the TokenStream is [syn::ItemImpl]
/// * the `trait implementer name` is one of the provided names
pub struct TraitCheck {
    trait_names: Vec<syn::Ident>,
    pub self_name: Option<syn::Ident>,
}

impl TraitCheck {
    pub fn new(trait_names: Vec<&str>) -> Self {
        let trait_names = trait_names
            .into_iter()
            .map(syn::parse_str::<syn::Ident>)
            .map(Result::unwrap)
            .collect::<Vec<syn::Ident>>();
        TraitCheck {
            trait_names: trait_names,
            self_name: None,
        }
    }

    /// Parse and verify a Token stream. a TokenStream is to be
    /// considered valid if:
    /// * the type of the TokenStream is [syn::ItemImpl]
    /// * the `trait implementer name` is one of the provided names
    ///
    /// # Arguments
    ///
    /// * `item`    tokens of the code the macro is on
    ///
    /// # Returns
    ///
    /// * Ok(trait implementation AST, type implementing interface) A tuple with a trait implementation AST and the type implementing the trait
    /// * Err(tokenstream)                                          Tokenstream containing an compile_error message
    pub fn verify_trait_implementation(
        &mut self,
        item: proc_macro::TokenStream,
    ) -> Result<(syn::ItemImpl, syn::TypePath), TokenStream> {
        // Since our macro goes on top of the `impl X for Y` block we try to parse
        // our input tokens to [syn::ItemImpl] Verify if it parses successful,
        // otherwise return an `compile_error!`
        let ast_tree = match syn::parse::<syn::ItemImpl>(item) {
            Ok(n) => n,
            Err(e) => {
                let msg = format!(
                    "parse error {}\n\
                #[zwave_command_class] can only be placed on valid trait implementations:\n\n\t\
                #[zwave_command_class]\n\t\
                `impl T for X {{ .. }}`\n\t\t\
                where T= {:#?}",
                    e,
                    self.trait_names
                        .iter()
                        .map(ToString::to_string)
                        .collect::<Vec<String>>()
                );

                return Err(quote::quote! {
                    compile_error!(#msg);
                }
                .into());
            }
        };

        // Verify if macro is placed on top of the ZWAVE_HANDLER_TRAIT_IDENT trait.
        // other wise return tokens which write an compile error.
        if let Err(e) = self.verify_if_macro_is_on_trait_impl(&ast_tree) {
            return Err(e.into());
        }

        // Verify on what kind of token the user implemented the trait on. we derive
        // the command class name from this. So we need to make sure we have a type
        // we can work with.
        let command_class_type: syn::TypePath = match &*ast_tree.self_ty {
            syn::Type::Path(p) => p.clone(),
            not_supported => {
                return Err(syn::Error::new(
                    not_supported.span(),
                    "currently only path types are supported",
                )
                .to_compile_error()
                .into());
            }
        };

        Ok((ast_tree, command_class_type))
    }

    fn verify_if_macro_is_on_trait_impl(
        &mut self,
        ast_tree: &syn::ItemImpl,
    ) -> Result<(), proc_macro2::TokenStream> {
        let error_msg = format!(
            "macro `#[zwave_command_class]` can only be used on an impl item implementing {:#?}",
            self.trait_names
                .iter()
                .map(ToString::to_string)
                .collect::<Vec<String>>()
        );

        if let Err(ts) = ast_tree
            .trait_
            .as_ref()
            .ok_or(ast_tree.self_ty.to_token_stream())
            .and_then(|x| self.trait_name_is_correct(x))
        {
            Err(syn::Error::new(ts.span(), error_msg).to_compile_error())
        } else {
            Ok(())
        }
    }

    fn trait_name_is_correct(
        &mut self,
        trait_tokens: &(Option<syn::Token![!]>, syn::Path, syn::Token![for]),
    ) -> Result<(), proc_macro2::TokenStream> {
        let ident = &trait_tokens
            .1
            .segments
            .last()
            .expect("path should always have at least one segment")
            .ident;

        self.self_name = Some(ident.clone());

        if !self.trait_names.contains(ident) {
            Err(trait_tokens.1.to_token_stream())
        } else {
            Ok(())
        }
    }
}
