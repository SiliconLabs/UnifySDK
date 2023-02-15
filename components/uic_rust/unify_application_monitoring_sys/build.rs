////////////////////
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
    let include = load_environment("unify_application_monitoring")?.include_directories;
    let binding_file =
        PathBuf::from(std::env::var("OUT_DIR").expect("OUT_DIR is always set during build stage"))
            .join("binding.rs");

    let header = match (
        option_env!("CARGO_LIBUNIFY_DIR"),
        option_env!("CARGO_MANIFEST_DIR"),
    ) {
        (Some(s), _) => {
            // Search for dependencies in the libunify install output
            let mut p = PathBuf::from(s);
            assert!(p.is_dir());
            p.push("include");
            assert!(p.is_dir(), "{} is not a directory", p.display());
            p.push("uic");
            assert!(p.is_dir(), "{} is not a directory", p.display());
            format!("{}/*.h", p.to_str().unwrap())
        }
        (None, Some(s)) => {
            // Search for dependencies within the monorepo
            let mut p = PathBuf::from(s);
            assert!(p.is_dir());
            assert!(p.pop());
            assert!(p.pop());
            p.push("unify_application_monitoring");
            assert!(p.is_dir(), "{} is not a directory", p.display());
            p.push("include");
            assert!(p.is_dir(), "{} is not a directory", p.display());

            format!("{}/*.h", p.to_str().unwrap())
        }
        (None, None) => {
            panic!("Missing environment variable. Expected 'CARGO_LIBUNIFY_DIR' or 'CARGO_MANIFEST_DIR'.")
        }
    };

    generate_bindings(
        &binding_file,
        &include,
        Some("unify_application_monitoring.*"),
        None,
        &[header],
    )
}
