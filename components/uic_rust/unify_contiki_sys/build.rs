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
    let include = load_environment("uic_contiki")?.include_directories;
    let binding_file =
        PathBuf::from(std::env::var("OUT_DIR").expect("OUT_DIR is always set during build stage"))
            .join("binding.rs");

    let headers = match (
        option_env!("CARGO_LIBUNIFY_DIR"),
        option_env!("CARGO_MANIFEST_DIR"),
    ) {
        (Some(s), _) => {
            // Search for dependencies in the libunify install output
            let clock: PathBuf = [s, "include", "uic", "sys", "clock.h"].iter().collect();
            assert!(clock.is_file());
            let ctimer: PathBuf = [s, "include", "uic", "sys", "ctimer.h"].iter().collect();
            assert!(ctimer.is_file());
            let process: PathBuf = [s, "include", "uic", "sys", "process.h"].iter().collect();
            assert!(process.is_file());
            let request_poller: PathBuf = [s, "include", "uic", "include", "request_poller.h"]
                .iter()
                .collect();
            assert!(request_poller.is_file());

            [
                clock.to_str().unwrap().into(),
                ctimer.to_str().unwrap().into(),
                process.to_str().unwrap().into(),
                request_poller.to_str().unwrap().into(),
            ]
        }
        (None, Some(s)) => {
            // Search for dependencies within the monorepo
            let mut p = PathBuf::from(s);
            assert!(p.is_dir());
            assert!(p.pop());
            assert!(p.pop());
            p.push("uic_contiki");
            let p = p.to_str().unwrap();
            [
                format!("{}/core/sys/clock.h", p),
                format!("{}/core/sys/ctimer.h", p),
                format!("{}/core/sys/process.h", p),
                format!("{}/include/request_poller.h", p),
            ]
        }
        (None, None) => {
            panic!("Missing environment variable. Expected 'CARGO_LIBUNIFY_DIR' or 'CARGO_MANIFEST_DIR'.")
        }
    };

    generate_bindings(
        &binding_file,
        &include,
        // Some("process.*|ctimer_.*|CLOCK_SECOND|.*_poll_.*|clock_.*"),
        None,
        None,
        &headers,
    )
}
