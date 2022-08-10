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

/// this macro exports link information in order for the compiler to
/// successfully link against the specified native C libraries. More
/// specifically, the unify_env.source is used to setup the following:
/// - link search paths         specifies to the linker where various native
///   libraries can be found
/// - rpath's                   the linker uses this to write the library paths
///     into rust binaries to the dynamic runtime linker knows where to load
///     these libraries from
/// - user specified libraries  tell the linker which libraries to link against
///
/// NOTE: if a search path is missing, they should be appended in the
/// unify_env.source file and not here!
///
/// # Arguments
///
/// this macro accepts a combination of arguments which are all optional. Not
/// specifying any arguments means that at least the unify environment is
/// correctly loaded. Use one of the following arguments to complete your setup:
/// * dylib "<lib>"  token to specify which library to link against
/// * or staticlib "<lib>" token to specify a static lib to link against.
/// >note that any variadic length of tokens is accepted by this macro!
///
/// # Usage
///
/// ```rust
/// # use unify_build_utils::load_unify_environment;
/// load_unify_environment!(
///     dylib "foo",
///     staticlib "bar",
///     dylib "baz"
/// );
/// ```
///
/// * for rust compilation targets that in theory do not need linkage in order
/// to build. e.g. shared libraries, link libraries are ignored.
/// * link libraries are transitivity propagated to the final binary, rpath's
/// are not. read more here: <https://github.com/rust-lang/cargo/pull/7811>.
/// * this macro relies on environment UNIFY_BINARY_DIR to be set correctly.
#[macro_export]
macro_rules! load_unify_environment {
    // base case for dylib declarations
    (@internal dylib $name:literal) => {{
        println!("cargo:rustc-link-lib=dylib={}", $name);
    }};

    // base case for static lib declarations
    (@internal staticlib $name:literal) => {{
        println!("cargo:rustc-link-lib=static={}", $name);
    }};

    (@internal ) => {{
        // this match is here to support macro invocation with no arguments
    }};

    // take one lib argument of the variable argument list and call the base
    // case for it repeat recursively for the others until we reached the base
    // case as well
    (@internal $t:ident $name:literal, $($t2:ident $name2:literal),+) => {{
        load_unify_environment!(@internal $t $name);
        load_unify_environment!(@internal $($t2 $name2),+)
    }};

    // this is the entry point of this macro
    ($($t2:ident $name2:literal),*) => {{
        if let Ok(rpaths) = std::env::var("UNIFY_LIB_PATH") {
            for path in rpaths.split(':') {
                println!("cargo:rustc-link-arg=-Wl,-rpath,{}", path);
                println!("cargo:rustc-link-search=native={}", path);
            }
        } else {
            println!(
                "cargo:warning=Skipping loading of Unify build environment. UNIFY_LIB_PATH not set!"
            );
        }

        load_unify_environment!(@internal $($t2 $name2),*);
    }};
}
