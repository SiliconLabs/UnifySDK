use super::compiler_arguments::*;
use anyhow::{ensure, Context, Result};
use once_cell::sync::Lazy;
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
            .map(|x| x.file_name().unwrap_or(x.as_os_str()))
            .filter_map(OsStr::to_str)
            .map(file_stem_library)
            .filter(|name| !name.is_empty())
            .map(str::to_string)
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
            .filter(|p| p.parent().is_some())
            .map(|p| cwd.as_ref().join(p))
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

        let mut defines: Vec<String> = get_items_as_list("DEFINES", &link_content)?;
        defines.sort();
        defines.dedup();
        let flags = get_items("FLAGS", &link_content).map(dedupe_linker_arguments)?;

        Ok(CompilerArguments {
            link_libraries,
            search_paths,
            include_directories,
            defines,
            flags,
            build_dir: cwd.as_ref().into(),
        })
    }
}
/// Scans a linker text for flags and tries to dedupe them.
/// given the following text:
///
///```bash
///-lFoo -Wl,rpath, /asdf/asdf/
///```
///returned is :
///
///```ignore
///["-lFoo", "-Wl,rpath, /asdf/asdf"]
///```
fn dedupe_linker_arguments<S: AsRef<str>>(link_line: S) -> Vec<String> {
    let text = link_line.as_ref();
    if text.find(' ').is_none() {
        return vec![text.to_string()];
    }

    let mut result = Vec::new();
    let mut first = true;
    let mut argument = String::new();

    let mut words = text.split_whitespace();
    while let Some(word) = words.next() {
        // on the first iteration of the while loop we want to skip
        // adding. since we are adding words leading the - character.
        if word.starts_with('-') && !first {
            result.push(argument.clone());
            argument.clear();
        }

        if !argument.is_empty() {
            argument.push(' ');
        }
        argument.push_str(word);

        first = false;
    }

    // append trailing link option
    result.push(argument);
    result.sort();
    result.dedup();
    result
}

/// match the file-stem of a library given its filename
///
/// # Arguments
///
/// captures_ read
fn file_stem_library(file_name: &str) -> &str {
    //capture the stem but leave out the optional version post-fix together with the file extension
    //the "lib" prefix gets omitted if present
    static REGEX: Lazy<std::result::Result<Regex, regex::Error>> =
        Lazy::new(|| Regex::new(r"^(?:lib)?(.*?)(?:\.)"));
    REGEX
        .as_ref()
        .ok()
        .and_then(|re| {
            let mut capture_locations = re.capture_locations();
            re.captures_read(&mut capture_locations, file_name);
            capture_locations.get(1)
        })
        .map(|(start, end)| &file_name[start..end])
        .unwrap_or(file_name)
}

fn get_items<P: AsRef<str>>(key: P, content: P) -> Result<String> {
    let regex = format!(r"\s*{}\s*=(.+)", key.as_ref());
    let mut items = Regex::new(&regex)?
        .captures_iter(content.as_ref())
        // glob all entries to one string
        .map(|cap| cap[1].trim().to_string())
        .collect::<Vec<String>>();
    items.sort();
    items.dedup();
    Ok(items
        .iter()
        .fold(String::new(), |acc, x| format!("{} {} ", acc, &x)))
}

/// returns a list of items corresponding to the passed key. items will be seperated by the
/// whitespace character
fn get_items_as_list<P: AsRef<str>>(key: P, content: P) -> Result<Vec<String>> {
    let list: Vec<String> = get_items(key, content)?
        .split_whitespace()
        .map(str::trim)
        .map(Into::into)
        .collect();
    Ok(list)
}

/// fuzzy matches build targets typically found inside the build.ninja file.
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
        let expected = " CXX_COMPILER__uic_attribute_store_ /Users/svradema/git/uic/components/uic_attribute_store/src/attribute_store.cpp || cmake_object_order_depends_target_uic_attribute_store\n  DEFINES = -DBOOST_PROGRAM_OPTIONS_DYN_LINK -DBOOST_PROGRAM_OPTIONS_NO_LIB\n  DEP_FILE = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src/attribute_store.cpp.o.d\n  FLAGS = -pipe -Werror -Wall -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX12.3.sdk -fPIC -std=gnu++17:\n  INCLUDES = -I/Users/svradema/git/uic/include -I/Users/svradema/git/uic/build/include -I/Users/svradema/git/uic/components/uic_attribute_store/include -I/Users/svradema/git/uic/components/uic_attribute_store/src -I/Users/svradema/git/uic/components/uic_datastore/include -I/Users/svradema/git/uic/components/uic_log/include -I/Users/svradema/git/uic/components/uic_log/../../include -I/Users/svradema/git/uic/components/uic_config/include -I/Users/svradema/git/uic/components/uic_contiki/core -I/Users/svradema/git/uic/components/uic_contiki/core/dev -I/Users/svradema/git/uic/components/uic_contiki/core/lib -I/Users/svradema/git/uic/components/uic_contiki/core/sys -I/Users/svradema/git/uic/components/uic_contiki/cpu/native -I/Users/svradema/git/uic/components/uic_contiki/include -I/Users/svradema/git/uic/components/uic_contiki/platform/posix -isystem /usr/local/include\n  OBJECT_DIR = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir\n  OBJECT_FILE_DIR = components/uic_attribute_store/CMakeFiles/uic_attribute_store.dir/src\n".to_string();
        assert_eq!(expected, result.unwrap());
    }

    #[test]
    fn multiple_targets_test() {
        let content =
            "build components/foo: qwertyqwerty\nbla\n\nbuild baz: bazbaz\n\nbuild foo/bar: jeez";
        let result = fuzzy_match_build_commands(content, "foo");
        assert_eq!(" qwertyqwerty\nbla\n jeez\n", result.unwrap());
    }

    #[test]
    fn library_file_stem_test() {
        assert_eq!("yaml-cpp", file_stem_library("libyaml-cpp.0.1.2.dylib"));
        assert_eq!("sqlite3", file_stem_library("sqlite3"));
    }

    #[test]
    fn dedupe_linker_arguments_test() {
        let text = "-lfoo";
        assert_eq!(vec![text], dedupe_linker_arguments(text));
        let text = "-bar -lfoo";
        assert_eq!(vec!["-bar", "-lfoo"], dedupe_linker_arguments(text));
        let text = "-lfoo -Wl,asdf, asdf/dfs/ -bar";
        assert_eq!(
            vec!["-Wl,asdf, asdf/dfs/", "-bar", "-lfoo"],
            dedupe_linker_arguments(text)
        );
        let text = "-bar -lfoo -bar";
        assert_eq!(vec!["-bar", "-lfoo"], dedupe_linker_arguments(text));
    }
}
