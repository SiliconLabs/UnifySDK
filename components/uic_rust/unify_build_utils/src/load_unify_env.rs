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
use super::*;
use anyhow::{ensure, Context, Result};
use std::{fmt::Display, path::PathBuf};

pub fn load_environment<S: AsRef<str> + Display>(ninja_target: S) -> Result<CompilerArguments> {
    let build_dir: PathBuf = std::env::var("UNIFY_BINARY_DIR")
        .context("missing `UNIFY_BINARY_DIR` environment variable")?
        .into();

    ensure!(
        build_dir.exists(),
        "cannot find `UNIFY_BINARY_DIR`{}",
        build_dir.to_string_lossy()
    );

    let mut deps = CompilerArguments::from_ninja(&build_dir , ninja_target.as_ref())?;
    deps.filter_forbidden_words(&["test", "mock", "stub", "unity"]);

    for path in &deps.search_paths {
        println!("cargo:rustc-link-arg=-Wl,-rpath,{}", path.to_string_lossy());
        println!("cargo:rustc-link-search=native={}", path.to_string_lossy());
    }

    for lib in &deps.link_libraries {
        println!("cargo:rustc-link-lib={}", lib);
    }

    dump_as_yaml(&deps, &build_dir)?;
    Ok(deps)
}

// for debug purposes
fn dump_as_yaml(build_ninja: &CompilerArguments, build_dir: &std::path::Path) -> Result<()> {
    let file_name = build_dir.join(format!(
        "{}/{}.yaml",
        std::env::var("OUT_DIR")?,
        std::env::var("CARGO_PKG_NAME")?
    ));

    std::fs::create_dir_all(file_name.parent().expect("yaml has at least one parent"))?;
    let yaml_file = std::fs::File::create(file_name)?;
    build_ninja.to_writer(yaml_file)
}
