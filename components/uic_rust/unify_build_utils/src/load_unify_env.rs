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

/// This function loads the build command for a given ninja target into a
/// structure named [CompilerArguments]. The target name is fuzzy matched,
/// meaning that there is no guarantees of getting back an exclusive list. This
/// function implies a hard dependency on Ninja build files.
/// > this function relies on the environment variable `UNIFY_BINARY_DIR`
///
/// # Arguments
///
/// * `ninja_target` a target name that is fuzzed against ninja compile units
///
pub fn load_environment<S: AsRef<str> + Display>(ninja_target: S) -> Result<CompilerArguments> {
    let build_dir: PathBuf = std::env::var("UNIFY_BINARY_DIR")
        .context("missing `UNIFY_BINARY_DIR` environment variable")?
        .into();

    ensure!(
        build_dir.exists(),
        "cannot find `UNIFY_BINARY_DIR`{}",
        build_dir.to_string_lossy()
    );

    let mut deps = CompilerArguments::from_ninja(&build_dir, ninja_target.as_ref())?;
    deps.filter_forbidden_words(&["test", "mock", "stub", "unity"]);

    if let Ok(s) = std::env::var("CARGO_LIBUNIFY_DIR") {
        //deps.include_directories.clear();
        deps.include_directories
            .push(PathBuf::from(&s).join("include/uic"));
        deps.include_directories
            .push(PathBuf::from(&s).join("include/uic/sys"));
        deps.include_directories
            .push(PathBuf::from(&s).join("include/uic/lib"));
    }

    dump_as_yaml(&deps, &build_dir)?;
    // make sure this build script is called again when the ninja file changes
    println!(
        "cargo:rerun-if-changed={}",
        build_dir.join("build.ninja").to_string_lossy()
    );

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
    println!(
        "cargo:warning=load_unify_env.rs: Writing compiler arguments for {} to {}",
        std::env::var("CARGO_PKG_NAME")?,
        file_name.to_string_lossy()
    );
    let yaml_file = std::fs::File::create(file_name)?;
    build_ninja.to_writer(yaml_file)
}
