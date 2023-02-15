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
use std::path::PathBuf;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    link_libunify();

    Ok(())
}

/// Link against libunify
fn link_libunify() {
    let p = match (
        option_env!("CARGO_LIBUNIFY_DIR"),
        option_env!("UNIFY_BINARY_DIR"),
    ) {
        (Some(s), _) => {
            // Search in the libunify install output
            let mut p = PathBuf::from(s);
            p.push("lib");
            println!(
                "cargo:warning=Looking to link against libunify in {}",
                p.display()
            );
            assert!(p.is_dir(), "{} is not a directory", p.display());
            let mut so = p.clone();
            so.push("libunify.so");
            if !so.is_file() {
                println!(
                    "cargo:warning=Did not find libunify. We expected to find it at {}",
                    so.display()
                )
            }
            p
        }
        (None, Some(s)) => {
            // Search within the monorepo
            let mut p = PathBuf::from(s);
            p.push("components");
            if !p.is_dir() {
                println!("cargo:warning=The directory {} does not exist", p.display());
            }
            let mut so = p.clone();
            so.push("libunify.so");
            if !so.is_file() {
                println!(
                    "cargo:warning=Did not find libunify. We expected to find it at {}",
                    so.display()
                );
            }
            p
        }
        (None, None) => {
            panic!("Missing environment variable. Expected 'CARGO_LIBUNIFY_DIR' or 'CARGO_MANIFEST_DIR'.")
        }
    };
    let p = p.to_str().unwrap();
    println!("cargo:rustc-link-arg=-Wl,-rpath,{}", p);
    println!("cargo:rustc-link-search=native={}", p);
    println!("cargo:rustc-link-lib=unify");
}
