// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
#![doc(html_no_source)]

extern crate glob;
use self::glob::glob;

mod logging;
use clap::{App, Arg};
use logging::Logging;
use serde::{Deserialize, Serialize};
use std::env;
use std::fmt::Display;
use std::io::ErrorKind;
use std::path::Path;
use std::{fs::File, io::BufReader, path::PathBuf};

#[derive(Serialize, Deserialize, Debug)]
struct BindingsTarget {
    name: String,
    block_list: Option<String>,
    allow_list: Option<String>,
    headers: Vec<String>,
    include_dirs: Vec<PathBuf>,
    out_file: PathBuf,
}

#[derive(Debug)]
pub struct ReadError {
    err: std::io::Error,
    path: PathBuf,
}

impl Display for ReadError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{} for {}", self.err, self.path.to_string_lossy())
    }
}

fn main() {
    let matches = App::new("Unify Binding generator")
            .about("generates rust binding files based on an input config. this config is generated from cmake code. see cmake function `generate_bindings`")
            .arg(Arg::new("verbose")
            .short('v')
            .long("verbose"))
            .arg(Arg::new("config")
            .short('c')
            .long("config")
            .value_name("config.json")
            .help("Set a custom config file")
            .multiple_values(true)
            .takes_value(true)
            .required(true))
            // this program is called as subcommand. cargo forwards the first argument
            // so remove it, in order to parse it correctly.
            .get_matches_from(env::args().skip(1));

    // config is a required argument, so unwrap always succeeds here.
    let configs = matches.values_of("config").unwrap();
    let redirect = !matches.is_present("verbose");

    // if there is a problem with redirecting stdout
    // just continue, worst case we dont have output.
    let mut logging = None;
    match Logging::new(redirect) {
        Ok(instance) => logging = Some(instance),
        Err(e) => println!("logging broken! {}", e),
    }

    for cfg in configs {
        // exit program if config cannot be loaded
        let target = BindingsTarget::from_json(PathBuf::from(cfg)).expect("not a valid config.");

        for tgt in target {
            match tgt.generate_bindings() {
                Ok(f) => {
                    if let Some(l) = &mut logging {
                        let log_line =
                            format!("\u{1F680}\t[{}] -- {}", tgt.name, f.to_string_lossy());
                        l.log(log_line);
                    }
                }
                Err(e) => {
                    if let Some(l) = &mut logging {
                        l.error();
                    }
                    println!("error running generator: {}", e);
                    std::process::exit(exitcode::USAGE);
                }
            }
        }
    }

    if let Some(l) = &mut logging {
        l.dump();
    }
    std::process::exit(exitcode::OK);
}

impl BindingsTarget {
    pub fn from_json(bindgen_config: PathBuf) -> Result<Vec<BindingsTarget>, ReadError> {
        let file = File::open(bindgen_config.clone()).map_err(|e| ReadError {
            err: e,
            path: bindgen_config.clone(),
        })?;
        let reader = BufReader::new(file);
        let mut targets: Vec<BindingsTarget> =
            serde_json::from_reader(reader).map_err(|e| ReadError {
                err: std::io::Error::from(e),
                path: bindgen_config.clone(),
            })?;

        for target in &mut targets {
            let headers = BindingsTarget::glob_headers(target).map_err(|e| ReadError {
                err: e,
                path: bindgen_config.clone(),
            })?;
            target.headers = headers
                .iter()
                .map(|f| f.to_string_lossy().to_string())
                .collect();
        }

        Ok(targets)
    }

    pub fn glob_headers(target: &BindingsTarget) -> std::io::Result<Vec<PathBuf>> {
        let mut result = Vec::new();

        for header in &target.headers {
            if header.contains('*') {
                for item in glob(header.as_str())
                    .map_err(|e| std::io::Error::new(ErrorKind::InvalidInput, e))?
                {
                    let header =
                        item.map_err(|e| std::io::Error::new(ErrorKind::InvalidInput, e))?;
                    result.push(header);
                }
            } else {
                result.push(PathBuf::from(header));
            }
        }
        Ok(result)
    }

    pub fn get_include_dirs(&self) -> Vec<PathBuf> {
        let mut includes = self.include_dirs.clone();
        includes.sort();
        includes.dedup();
        includes.into_iter().filter(|path| path.is_dir()).collect()
    }

    /// generates an bindings file
    pub fn generate_bindings(&self) -> std::io::Result<&Path> {
        //println!("cargo:rustc-env=BINDGEN_EXTRA_CLANG_ARGS=-x c++ -std=c++14");
        let mut bindings = bindgen::Builder::default()
            .derive_default(true)
            .opaque_type("std::.*")
            .default_alias_style(bindgen::AliasVariation::TypeAlias)
            .default_enum_style(bindgen::EnumVariation::Rust {
                non_exhaustive: false,
            })
            .derive_default(true)
            .prepend_enum_name(false)
            .translate_enum_integer_types(true)
            .dynamic_link_require_all(true)
            .clang_arg("-v")
            .respect_cxx_access_specs(true)
            .layout_tests(false)
            .generate_comments(false);

        if let Some(allow) = &self.allow_list {
            bindings = bindings
                .allowlist_type(allow)
                .allowlist_function(allow)
                .allowlist_var(allow);
        }

        if let Some(block) = &self.block_list {
            bindings = bindings
                .blocklist_function(block)
                .blocklist_type(block)
                .blocklist_item(block);
        }

        for include in &self.get_include_dirs() {
            println!(
                "[{}] - adding include {}",
                self.name,
                include.to_string_lossy()
            );
            bindings = bindings.clang_arg(format!("-I{}", include.to_str().unwrap()));
        }

        for header in &self.headers {
            if !PathBuf::from(header).exists() {
                println!(
                    "[{}] omitting header {}, does not exists",
                    self.name, header
                );
            } else {
                println!("[{}] - adding header {}", self.name, header);
                bindings = bindings.header(header);
            }
        }

        bindings
            .generate()
            .map_err(|_| {
                std::io::Error::new(
                    ErrorKind::Other,
                    format!("error in binding generator for {}", &self.name),
                )
            })?
            .write_to_file(&self.out_file)
            .and(Ok(&self.out_file))
    }
}
