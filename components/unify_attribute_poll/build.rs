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
fn main() -> Result<(), Box<dyn std::error::Error>>{
    // link against libunify
    println!("cargo:rustc-link-arg=-Wl,-rpath,{}/components", std::env::var("UNIFY_BINARY_DIR")?);
    println!("cargo:rustc-link-search=native={}/components", std::env::var("UNIFY_BINARY_DIR")?);
    println!("cargo:rustc-link-lib=unify");
    Ok(())
}

