use crate::command_class_composer::CommandClassComposer;
use crate::extern_c_hook::append_c_hook;
use proc_macro2::TokenStream;
use quote::ToTokens;
use syn::parse_quote;
const INCLUDE_SUPPORT_IDENT: &'static str = "INCLUDE_SUPPORT";
const INCLUDE_CONTROL_IDENT: &'static str = "INCLUDE_CONTROL";

pub fn zwave_handler_glue(
    command_class_type: &syn::TypePath,
    ast_tree: &syn::ItemImpl,
) -> TokenStream {
    // parse_quote! can parse given tokens to an corresponding syn node. In this
    // case, we all will parse them to a `TypePath` since we need to build up
    // the constants from the zwave command class trait. e.g.
    // `FooFrame::VERSION` These constants we need to generate our code
    let cc_type_id = parse_quote!(#command_class_type::COMMAND_CLASS_TYPE_ID);
    let cc_name = parse_quote!(#command_class_type::COMMAND_CLASS_NAME);
    let cc_version = parse_quote!(#command_class_type::VERSION);
    let cc_comment = parse_quote!(#command_class_type::COMMENTS);
    let cc_minimal_scheme = parse_quote!(#command_class_type::MINIMAL_SCHEME);
    let cc_manual_security_validation =
        parse_quote!(#command_class_type::MANUAL_SECURITY_VALIDATION);

    let (has_config_support, has_control_support) = support_control_configured(&ast_tree);

    let composer = CommandClassComposer::new(
        &command_class_type,
        cc_type_id,
        cc_name,
        cc_comment,
        cc_version,
        cc_minimal_scheme,
        cc_manual_security_validation,
        has_config_support,
        has_control_support,
    );

    let init_func_name = composer.get_init_func_name();
    let init_stmt: syn::Stmt = syn::parse_quote!(#init_func_name(););
    append_c_hook(init_stmt);

    // convert from syntax tree's back again to a token stream
    let mut tokens = TokenStream::new();
    composer.to_tokens(&mut tokens);
    tokens
}

fn support_control_configured(ast_tree: &syn::ItemImpl) -> (bool, bool) {
    let mut control = false;
    let mut support = false;

    for const_val in ast_tree
        .items
        .iter()
        .filter_map(contains_constant_declaration)
    {
        if const_val.ident == INCLUDE_CONTROL_IDENT && const_val.expr == parse_quote!(true) {
            control = true;
        }
        if const_val.ident == INCLUDE_SUPPORT_IDENT && const_val.expr == parse_quote!(true) {
            support = true;
        }
    }

    (support, control)
}

fn contains_constant_declaration(item: &syn::ImplItem) -> Option<&syn::ImplItemConst> {
    match item {
        syn::ImplItem::Const(x) => Some(x),
        _ => None,
    }
}
