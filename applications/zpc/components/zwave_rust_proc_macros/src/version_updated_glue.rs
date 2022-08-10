use proc_macro2::{Span, TokenStream};

use crate::extern_c_hook;

const CONST_ATTRIBUTE_TYPE_ID: &str = "ATTRIBUTE_STORE_TYPE";

pub fn version_updated_glue(
    items: &[syn::ImplItem],
    trait_class_name: &syn::TypePath,
) -> TokenStream {
    let callback_name = quote::format_ident!(
        "__internal_{}_callback",
        trait_class_name.path.segments.last().unwrap().ident
    );

    let attribute_id = syn::Ident::new(CONST_ATTRIBUTE_TYPE_ID, Span::call_site());
    let _ = &items
        .iter()
        .filter_map(contains_constant_declaration)
        .find(|i| i.ident == attribute_id)
        .expect("all trait properties to be implemented");

    let register_callback: syn::Stmt = syn::parse_quote!(
        unify_attribute_store_sys::attribute_store_register_callback_by_type_and_state(
            Some(#callback_name),
            #trait_class_name::#attribute_id,
            unify_attribute_store_sys::attribute_store_node_value_state_t::REPORTED_ATTRIBUTE
        );
    );
    extern_c_hook::append_c_hook(register_callback);

    quote::quote! {
        #[doc(hidden)]
        pub unsafe extern "C" fn #callback_name(
            node: unify_attribute_store_sys::attribute_store_node_t,
            change: unify_attribute_store_sys::attribute_store_change_t
        ) {
            let attribute_store = unify_middleware::attribute_store_or_return_with!();
            let attr = attribute_store.from_handle(node);
            match attr.get_reported::<u8>() {
                Ok(version) => #trait_class_name::on_attribute_version(attr.parent(), version),
                _ => {},
            }
        }
    }
    .into()
}

fn contains_constant_declaration(item: &syn::ImplItem) -> Option<&syn::ImplItemConst> {
    match item {
        syn::ImplItem::Const(x) => Some(x),
        _ => None,
    }
}
