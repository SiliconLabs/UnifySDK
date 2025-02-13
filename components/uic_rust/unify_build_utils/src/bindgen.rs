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
use anyhow::{anyhow, bail};
use anyhow::{Context, Result};
use glob::glob;
use std::path::Path;
use std::path::PathBuf;

/// This function expands wildcards given they are present in an array item.
/// for more specific info on the capabilities please refer to [glob::glob]
fn expand_headers<S: AsRef<str>>(headers: &[S]) -> Result<Vec<PathBuf>> {
    let mut vec = vec![];

    for h in headers {
        if h.as_ref().contains('*') {
            for p in glob(h.as_ref())? {
                vec.push(p?);
            }
        } else {
            vec.push(PathBuf::try_from(h.as_ref())?);
        }
    }

    Ok(vec)
}

/// This function generates a rust binding file given a list of C input headers.
/// It can be used to interface and marshall data between rust and C. Note that these bindings are
/// categorized as unsafe rust. therefore its strongly adviced that bindings files are wrapped in
/// an component that makes safe rust code out of the generated binding file.
///
/// # Arguments
///
/// * `out_filename`    bindings will be written to this path. any existing file will be
/// overwritten.
/// > Important: be aware that if you place the binding file in a hardcoded place you high likely
/// introduce a race condition, multiple rustc processes are trying to read and generate this
/// file at the same time. A good location would be a path somewhere located inside "OUT_DIR".
/// * `include dirs`    a list of include directories. This list need to be *complete*. This means
/// all includes inside c files are resolvable by this list.
///
/// given a binding is desired of `foo.h`:
/// ```c
/// // foo.h
///#include "baz.h"
/// ```
///and `baz.h` is located inside of folder `bar`. The correct include list would be the following:
///
///```no_run
///# use unify_build_utils::generate_bindings;
///# use std::path::PathBuf;
///let include1: PathBuf = ".".into();
///let include2: PathBuf = "bar".into();
///let file = PathBuf::from("foo.rs");
///generate_bindings(&file, &[include1, include2], None, None, &["foo.h"]).unwrap()
///```
///
///* `allow_list`   filter which functions, types, or variables are allowed to be generated
///bindings of. Recommended is to use this argument. Passing `None` will generate bindings of
///everything, including system includes. which significantly degrades the performance.
///* `block_list`    filter to block any functions, types or variables to be generated.
///* `headers`    list of C header files to generate rust bindings of.
///
///# Example
///
///* foo.h:
///
///```c
/// #ifndef FOO_H
/// #define FOO_H
///
/// int foo();
/// int bar();
///
/// #endif
///```
///
///```no_run
///# use std::path::PathBuf;
///# use unify_build_utils::generate_bindings;
/// let out_file = PathBuf::from(std::env::var("OUT_DIR").unwrap()).join("foo.rs");
/// generate_bindings(&out_file, &[], Some("foo"), None, &["codebase/component/foo.h"]).unwrap()
///```
///
/// will generate a binding file containing roughly the following:
///
/// ```
/// extern "C" {
///   pub fn foo() -> core::ffi::c_int;
/// }
/// ```
pub fn generate_bindings<P: AsRef<Path>, S: AsRef<str>>(
    out_filename: &P,
    include_dirs: &[P],
    allow_list: Option<&'static str>,
    block_list: Option<&'static str>,
    headers: &[S],
) -> Result<()> {
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
        .generate_comments(false)
        .use_core()
        .size_t_is_usize(true)
        .ctypes_prefix("core::ffi");

    if let Some(allow) = allow_list.as_ref() {
        bindings = bindings
            .allowlist_type(allow)
            .allowlist_function(allow)
            .allowlist_var(allow);
    }

    if let Some(block) = block_list.as_ref() {
        bindings = bindings
            .blocklist_function(block)
            .blocklist_type(block)
            .blocklist_item(block);
    }

    for include in include_dirs {
        bindings = bindings.clang_arg(format!("-I{}", include.as_ref().to_string_lossy()));
    }

    let expanded = expand_headers(headers)?;
    if expanded.contains(&PathBuf::from("")) {
        bail!("headers argument not entered correctly");
    }

    for header in expanded {
        //println!("cargo:warning=headertje:{}", header.to_string_lossy());
        bindings = bindings.header(header.to_string_lossy());
    }

    bindings
        .generate()
        .map_err(|e| {
            anyhow!(format!(
                "error in binding generator for {} {}",
                e,
                out_filename.as_ref().to_string_lossy()
            ))
        })?
        .write_to_file(out_filename)
        .context("failed to write to file")
}
