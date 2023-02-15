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
use anyhow::{Context, Result};
use serde::{Deserialize, Serialize};
use std::io::Write;
use std::path::{Path, PathBuf};

/// This structure holds some properties in order to link against natively build components
/// It does not cover every configuration imaginable. But is designed with the purpose of linking
/// native C libraries into rust.
///
/// Take a look also at [CompilerArguments::from_ninja]
#[derive(Debug, Serialize, Deserialize)]
pub struct CompilerArguments {
    pub link_libraries: Vec<String>,
    pub search_paths: Vec<PathBuf>,
    pub include_directories: Vec<PathBuf>,
    pub defines: Vec<String>,
    pub flags: Vec<String>,
    pub build_dir: PathBuf,
}

impl CompilerArguments {
    /// serialize [Self] to a writer
    pub fn to_writer<W: Write>(&self, mut writer: W) -> Result<()> {
        serde_yaml::to_writer(&mut writer, &self).context("error writing")
    }

    /// this function filters out occurences of the provided words inside the entries of all
    /// members of this struct.
    pub fn filter_forbidden_words<S: AsRef<str>>(&mut self, forbidden_words: &[S]) {
        let has_forbidden_words = |word: &str| {
            for item in forbidden_words.iter().map(AsRef::as_ref) {
                if word.contains(item) {
                    return true;
                }
            }
            false
        };

        self.link_libraries
            .retain(|item| !has_forbidden_words(item));
    }

    /// deserialize from the provided YAML file
    pub fn from_yaml<P: AsRef<Path>>(path: P) -> Result<Self> {
        let file = std::fs::File::open(&path)
            .with_context(|| format!("could not open {}", path.as_ref().to_string_lossy()))?;
        let instance: Self = serde_yaml::from_reader(&file).with_context(|| {
            format!("error parsing {} to yaml", path.as_ref().to_string_lossy())
        })?;
        Ok(instance)
    }

    /// this function writes all link arguments to cargo using the stdout interface.
    pub fn export_to_cargo(&self, flags: bool) {
        for path in &self.search_paths {
            println!("cargo:rustc-link-arg=-Wl,-rpath,{}", path.to_string_lossy());
            println!("cargo:rustc-link-search=native={}", path.to_string_lossy());
        }

        for lib in &self.link_libraries {
            println!("cargo:rustc-link-lib={}", lib);
        }

        if flags {
            println!(
                "cargo:rustc-link-arg={} {}",
                self.defines.join(" "),
                self.flags.join(" "),
            );
        }
    }
}
