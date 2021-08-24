// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

extern crate glob;
use self::glob::glob;

use serde::{Deserialize, Serialize};
use std::io::Result;
use std::{fs::File, io::BufReader, path::PathBuf};

#[derive(Serialize, Deserialize, Debug)]
pub struct LinkLibrary {
    name: String,
    search_path: String,
    typ: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct BindingsTarget {
    name: String,
    target_source_dir: PathBuf,
    allow_list: String,
    block_list: String,
    link_libraries: Vec<LinkLibrary>,
    include_dirs: Vec<PathBuf>,
    input_config: Option<PathBuf>,
}

impl BindingsTarget {
    pub fn from_json(bindgen_config: String) -> Result<BindingsTarget> {
        let file = File::open(std::path::PathBuf::from(bindgen_config.clone()))?;
        let reader = BufReader::new(file);
        let mut tgt: BindingsTarget = serde_json::from_reader(reader)?;
        tgt.input_config = Some(std::path::PathBuf::from(bindgen_config));
        Ok(tgt)
    }

    pub fn get_output_file(&self) -> PathBuf {
        PathBuf::from(format!("{}/{}_bindings.rs", env!("LIBUIC_DIR"), &self.name))
    }

    pub fn glob_headers(&self) -> Vec<PathBuf> {
        let mut result = Vec::new();
        let glb = format!("{}/include/*.h", self.target_source_dir.to_str().unwrap());
        for item in glob(glb.as_str()).unwrap() {
            let mut header = PathBuf::new();
            match item {
                Err(e) => println!("cargo:warning={}", e),
                Ok(hdr) => header = hdr,
            }
            println!(
                "cargo:rerun-if-changed={}",
                &header.clone().to_str().unwrap()
            );
            result.push(header);
        }
        result
    }

    pub fn is_valid(&self) -> Option<()> {
        if !self.target_source_dir.exists() {
            println!(
                "cargo:warning= stopping bindgen for {} could not find include dir {}",
                self.name,
                self.target_source_dir.to_string_lossy()
            );
            return None;
        }

        Some(())
    }

    /// export native c library configuration to rust cargo.
    /// it enables the rust enviroment, e.g. running tests
    pub fn write_link_configuration(&self) {
        println!(
            "cargo:rerun-if-changed={}",
            self.input_config.clone().unwrap().to_str().unwrap()
        );

        for lib in &self.link_libraries {
            println!("cargo:rustc-link-search=native={}", &lib.search_path);
            println!("cargo:rustc-link-lib={}={}", lib.typ, lib.name);
        }

        #[cfg(target_os = "macos")]
        println!("cargo:rustc-link-lib=dylib=c++");
        #[cfg(target_os = "linux")]
        println!("cargo:rustc-link-lib=dylib=stdc++");

        let joined = self
            .link_libraries
            .iter()
            .map(|l| l.search_path.clone())
            .collect::<Vec<String>>()
            .join(":");
        println!("cargo:rustc-env=LD_LIBRARY_PATH={}", joined);
    }

    pub fn get_include_dirs(&self) -> Vec<PathBuf> {
        let mut includes = self.include_dirs.clone();
        includes.sort();
        includes.dedup();
        includes.into_iter().filter(|path| path.is_dir()).collect()
    }
}

/// generates an bindings file
pub fn generate_bindings(target: BindingsTarget) {
    println!(
        "cargo:rerun-if-changed={}",
        &target.input_config.clone().unwrap().to_str().unwrap()
    );

    target.is_valid().expect("target not valid!");

    //println!("cargo:rustc-env=BINDGEN_EXTRA_CLANG_ARGS=-x c++ -std=c++14");
    let mut bindings = bindgen::Builder::default()
        .derive_default(true)
        .opaque_type("std::.*")
        .allowlist_type(target.allow_list.clone())
        .allowlist_function(target.allow_list.clone())
        .allowlist_var(target.allow_list.clone())
        .default_alias_style(bindgen::AliasVariation::TypeAlias)
        .default_enum_style(bindgen::EnumVariation::Rust {
            non_exhaustive: false,
        })
        .prepend_enum_name(false)
        .translate_enum_integer_types(true)
        .dynamic_link_require_all(true)
        .clang_arg("-v")
        .respect_cxx_access_specs(true);

    if !target.block_list.is_empty() {
        bindings = bindings
            .blocklist_function(target.block_list.clone())
            .blocklist_type(target.block_list.clone())
            .blocklist_item(target.block_list.clone());
    }

    for include in &target.get_include_dirs() {
        bindings = bindings.clang_arg(format!("-I{}", include.to_str().unwrap()));
    }

    for header in &target.glob_headers() {
        bindings = bindings.header(header.to_string_lossy());
    }

    bindings
        .generate()
        .unwrap()
        .write_to_file(target.get_output_file())
        .unwrap()
}
