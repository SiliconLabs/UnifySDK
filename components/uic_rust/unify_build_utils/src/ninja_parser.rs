use super::compiler_arguments::*;
use anyhow::{Context, Result, ensure};
use regex::{Regex, RegexBuilder};
use std::{
    ffi::OsStr,
    path::{Path, PathBuf},
};

impl CompilerArguments {
 /// Loads linking information from ninja by parsing the build.ninja file
    pub fn from_ninja<P: AsRef<Path>>(cwd: P, phony_target: &str) -> Result<Self> {
        let ninja_build = cwd.as_ref().join("build.ninja");
    ensure!(
        ninja_build.exists(),
        "cannot find ninja build. run ninja first"
    );

    let content = std::fs::read_to_string(&ninja_build)?;
        let link_content =
            fuzzy_match_build_commands(&content, phony_target).with_context(|| {
                format!(
                    "could not find any build targets with name {}",
                    phony_target
                )
            })?;

        let link_list: Vec<PathBuf> = get_items_as_list("LINK_LIBRARIES", &link_content)?
            .iter()
            .map(AsRef::as_ref)
            // -Wl flags are not split correctly, fix it
            .map(|x: &str| x.find("-Wl").map_or(x, |i| &x[..i]))
            // remove -l prefixes if there are any
            .map(|x| x.strip_prefix("-l").unwrap_or(x))
            .map(PathBuf::from)
            .collect();

        let mut link_libraries: Vec<String> = link_list
            .iter()
            .map(|x| x.file_stem().unwrap_or(x.as_os_str()))
            .filter_map(OsStr::to_str)
            // strip lib prefix if any
            .map(|x| x.strip_prefix("lib").unwrap_or(x))
            .filter(|s| !s.is_empty())
            .map(Into::into)
            .collect();
        if cfg!(target_os = "macos") {
            link_libraries.push("c++".to_string());
        } else {
            link_libraries.push("stdc++".to_string());
        }
        link_libraries.sort();
        link_libraries.dedup();

        let mut canonicalized: Vec<PathBuf> = link_list
            .iter()
            .filter_map(|p| std::fs::canonicalize(cwd.as_ref().join(p)).ok())
            .collect();
        canonicalized.sort();
        canonicalized.dedup();

        let mut search_paths: Vec<PathBuf> = canonicalized
            .iter()
            .map(AsRef::as_ref)
            .filter_map(Path::parent)
            .map(Into::into)
            .collect();
        search_paths.sort();
        search_paths.dedup();

        let mut include_directories: Vec<PathBuf> = get_items_as_list("INCLUDES", &content)?
            .iter()
            .map(AsRef::as_ref)
            .map(|x: &str| x.strip_prefix("-I").unwrap_or(x))
            .map(|x: &str| x.strip_suffix(".").unwrap_or(x))
            .filter(|s| !s.is_empty())
            .map(PathBuf::from)
            .filter_map(|p| {
                if !p.has_root() {
                    std::fs::canonicalize(cwd.as_ref().join(p)).ok()
                } else {
                    Some(p)
                }
            })
            .collect();
        include_directories.sort();
        include_directories.dedup();

        Ok(CompilerArguments {
            link_libraries,
            search_paths,
            include_directories,
            build_dir: cwd.as_ref().into(),
        })
    }
}

/// returns a list of items corresponding to the passed key. items will be seperated by the
/// whitespace character
fn get_items_as_list<P: AsRef<str>>(key: P, content: P) -> Result<Vec<String>> {
    let regex = format!(r"\s*{}\s*=(.+)", key.as_ref());
    let re = Regex::new(&regex)?;
    let list: Vec<String> = re
        .captures_iter(content.as_ref())
        // glob all entries to one string
        .map(|cap| cap[1].to_string())
        .fold(String::new(), |acc, x| (acc + &x))
        .split_whitespace()
        .map(str::trim)
        .map(Into::into)
        .collect();
    Ok(list)
}

/// Fuzzy matches build targets typically found inside the build.ninja file.
///
/// # Arguments
///
/// `content`       text to search in
/// `target_fuzz`   complete or partial target name. These target names correspond to target objects
///                 build by ninja's phony command.
///
/// # Returns
///
/// String containing all build commands matching the target_fuzz input. these build commands are
/// concatenated by newlines.
///
fn fuzzy_match_build_commands<P: AsRef<str>>(content: &str, target_fuzz: P) -> Result<String> {
    let regex = format!("^build(.*{}.*):", target_fuzz.as_ref());
    let re = RegexBuilder::new(&regex)
        .multi_line(true)
        .build()
        .with_context(|| format!("regex failure using {}", regex))?;

    let mut result = String::new();
    let end = content.len();
    let mut begin = 0;

    while begin != end {
        let mut capture_locations = re.capture_locations();
        let found = match re.captures_read(&mut capture_locations, &content[begin..end]) {
            None => break,
            Some(x) => x,
        };

        let start = begin + found.end();
        // find the end of the build command text, which are seperated by newlines.
        let last = content[start..].find("\n\n").unwrap_or(end - start);
        result.push_str(&content[start..start + last]);
        result.push('\n');

        // workaround: Since the build command describes how the file will be compiled
        // the actual binary will not be returned in the result string. The most crutial
        // piece of link information. Therefore inject it here:
        let target_lib = capture_locations
            .get(1)
            .map(|(s, e)| &content[begin + s..begin + e])
            .map(str::trim);

        if let Some(lib) = target_lib {
            if lib.ends_with(".a") || lib.ends_with(".so") || lib.ends_with(".dylib") {
                let link_lib_str = format!("LINK_LIBRARIES= {} \n", lib);
                result.push_str(&link_lib_str);
            }
        }

        begin = start + last;
    }
    Ok(result)
}

#[cfg(test)]
mod tests {
    use super::*;
    static VALID_CONTENT1: &'static str = "
# =============================================================================
# Write statements declared in CMakeLists.txt:
# /Users/svradema/git/uic/components/CMakeLists.txt
# =============================================================================

# =============================================================================
# Object build statements for STATIC_LIBRARY target uic_attribute_store


#############################################
# Order-only phony target for uic_attribute_store

build cmake_object_order_depends_target_uic_attribute_store: phony || cmake_object_order_depends_target_uic_config cmake_object_order_depends_target_uic_contiki_platform cmake_object_order_depends_target_uic_datastore

build components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src/attribute_store.cpp.o: CXX_COMPILER__uic_attribute_store_ /Users/svradema/git/uic/components/uic_attribute_store/src/attribute_store.cpp || cmake_object_order_depends_target_uic_attribute_store
  DEFINES = -DBOOST_PROGRAM_OPTIONS_DYN_LINK -DBOOST_PROGRAM_OPTIONS_NO_LIB
  DEP_FILE = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src/attribute_store.cpp.o.d
  FLAGS = -pipe -Werror -Wall -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX12.3.sdk -fPIC -std=gnu++17:
  INCLUDES = -I/Users/svradema/git/uic/include -I/Users/svradema/git/uic/build/include -I/Users/svradema/git/uic/components/uic_attribute_store/include -I/Users/svradema/git/uic/components/uic_attribute_store/src -I/Users/svradema/git/uic/components/uic_datastore/include -I/Users/svradema/git/uic/components/uic_log/include -I/Users/svradema/git/uic/components/uic_log/../../include -I/Users/svradema/git/uic/components/uic_config/include -I/Users/svradema/git/uic/components/uic_contiki/core -I/Users/svradema/git/uic/components/uic_contiki/core/dev -I/Users/svradema/git/uic/components/uic_contiki/core/lib -I/Users/svradema/git/uic/components/uic_contiki/core/sys -I/Users/svradema/git/uic/components/uic_contiki/cpu/native -I/Users/svradema/git/uic/components/uic_contiki/include -I/Users/svradema/git/uic/components/uic_contiki/platform/posix -isystem /usr/local/include
  OBJECT_DIR = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir
  OBJECT_FILE_DIR = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src

build components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src/attribute_store_callbacks.cpp.o: CXX_COMPILER__uic_attribute_store_ /Users/svradema/git/uic/components/uic_attribute_store/src/attribute_store_callbacks.cpp || cmake_object_order_depends_target_uic_attribute_store
  DEFINES = -DBOOST_PROGRAM_OPTIONS_DYN_LINK -DBOOST_PROGRAM_OPTIONS_NO_LIB
  DEP_FILE = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src/attribute_store_callbacks.cpp.o.d
  FLAGS = -pipe -Werror -Wall -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX12.3.sdk -fPIC -std=gnu++17
  INCLUDES = -I/Users/svradema/git/uic/include -I/Users/svradema/git/uic/build/include -I/Users/svradema/git/uic/components/uic_attribute_store/include -I/Users/svradema/git/uic/components/uic_attribute_store/src -I/Users/svradema/git/uic/components/uic_datastore/include -I/Users/svradema/git/uic/components/uic_log/include -I/Users/svradema/git/uic/components/uic_log/../../include -I/Users/svradema/git/uic/components/uic_config/include -I/Users/svradema/git/uic/components/uic_contiki/core -I/Users/svradema/git/uic/components/uic_contiki/core/dev -I/Users/svradema/git/uic/components/uic_contiki/core/lib -I/Users/svradema/git/uic/components/uic_contiki/core/sys -I/Users/svradema/git/uic/components/uic_contiki/cpu/native -I/Users/svradema/git/uic/components/uic_contiki/include -I/Users/svradema/git/uic/components/uic_contiki/platform/posix -isystem /usr/local/include
  OBJECT_DIR = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir
  OBJECT_FILE_DIR = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src";

    #[test]
    fn glob_build_targets_test() {
        let result = fuzzy_match_build_commands(VALID_CONTENT1, "attribute_store.cpp.o");
        let expected = " CXX_COMPILER__uic_attribute_store_ /Users/svradema/git/uic/components/uic_attribute_store/src/attribute_store.cpp || cmake_object_order_depends_target_uic_attribute_store
  DEFINES = -DBOOST_PROGRAM_OPTIONS_DYN_LINK -DBOOST_PROGRAM_OPTIONS_NO_LIB
  DEP_FILE = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src/attribute_store.cpp.o.d
  FLAGS = -pipe -Werror -Wall -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX12.3.sdk -fPIC -std=gnu++17
  INCLUDES = -I/Users/svradema/git/uic/include -I/Users/svradema/git/uic/build/include -I/Users/svradema/git/uic/components/uic_attribute_store/include -I/Users/svradema/git/uic/components/uic_attribute_store/src -I/Users/svradema/git/uic/components/uic_datastore/include -I/Users/svradema/git/uic/components/uic_log/include -I/Users/svradema/git/uic/components/uic_log/../../include -I/Users/svradema/git/uic/components/uic_config/include -I/Users/svradema/git/uic/components/uic_contiki/core -I/Users/svradema/git/uic/components/uic_contiki/core/dev -I/Users/svradema/git/uic/components/uic_contiki/core/lib -I/Users/svradema/git/uic/components/uic_contiki/core/sys -I/Users/svradema/git/uic/components/uic_contiki/cpu/native -I/Users/svradema/git/uic/components/uic_contiki/include -I/Users/svradema/git/uic/components/uic_contiki/platform/posix -isystem /usr/local/include
  OBJECT_DIR = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir
  OBJECT_FILE_DIR = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src\nLINK_LIBRARIES= components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src/attribute_store.cpp.o\n".to_string();
        assert_eq!(expected, result.unwrap());
    }

    #[test]
    fn multiple_targets_test() {
        let content =
            "build components/foo: qwertyqwerty\nbla\n\nbuild baz: bazbaz\n\nbuild foo/bar: jeez";
        let result = fuzzy_match_build_commands(content, "foo");
        assert_eq!(
            " qwertyqwerty\nbla\nLINK_LIBRARIES= components/foo\n jeez\nLINK_LIBRARIES= foo/bar\n",
            result.unwrap()
        );
    }
}
