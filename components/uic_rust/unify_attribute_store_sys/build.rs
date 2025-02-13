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
use anyhow::Result;
use std::path::PathBuf;
use unify_build_utils::*;

fn main() -> Result<()> {
    // link against libunify
    println!("cargo:rustc-link-arg=-Wl,-rpath,{}/components", std::env::var("UNIFY_BINARY_DIR")?);
    println!("cargo:rustc-link-search=native={}/components", std::env::var("UNIFY_BINARY_DIR")?);
    println!("cargo:rustc-link-lib=unify");
    bindings()
}

fn bindings() -> Result<()> {
    let link_dependencies = load_environment("uic_attribute_store")?;

    let binding_file =
        PathBuf::from(std::env::var("OUT_DIR").expect("OUT_DIR is always set during build stage"))
            .join("uic_attribute_store.rs");

    let components_path = PathBuf::from(concat!(env!("CARGO_MANIFEST_DIR"), "/../.."));

    let uic_attribute_store = format!("{}/uic_attribute_store/include/*.h", components_path.to_string_lossy());

    let uic_attribute_utils = format!("{}/uic_attribute_utils/include/attribute_timeouts.h", components_path.to_string_lossy()); 

    let headers = [
        uic_attribute_store,
        uic_attribute_utils 
    ];

    generate_bindings(
        &binding_file,
        &link_dependencies.include_directories,
        Some("attribute_store_.*|ATTRIBUTE_STORE.*|.*attribute_timeout.*|CLOCK_SECOND"),
        None,
        &headers,
    )
}
