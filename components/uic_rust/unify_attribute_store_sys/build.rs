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
fn main() {
    // these exports are only used when an exe is compiled. e.g. integration
    // tests or final binaries!
    println!("cargo:rustc-link-lib=dylib=uic_attribute_store");
    println!("cargo:rustc-link-lib=dylib=uic_datastore");
}
