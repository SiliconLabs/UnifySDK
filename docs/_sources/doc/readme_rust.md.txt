# Unify Rust Development

The Unify Framework has support for programming in Rust. Several components are already written using Rust and more components will be added in the future. This document describes how to setup, build and develop rust components for the Unify Framework.

> The docker images provided by the Unify Host SDK contain already everything you need to program Rust.

## Overview

Rust has its own ecosystem regarding building and package management, see [Cargo](https://doc.rust-lang.org/cargo/). We tried to integrate this as best as possible to our own cmake build environment. The overall philosophy is to keep the amount of changes and configurations we need to do to cargo as simple as possible. This means we can leverage from cargo tooling without interference of our build-system.
<br>
At the moment of writing we have 2 components that make use of Rust and buildup with different rust crates:

- ZPC binary, \<project_root\>/applications/zpc/components/zpc_rust
- Unify libraries, \<project_root\>/components/uic_rust

But mix and matching Rust and C libraries is possible with little effort.

## Building

rust packages can be build by using the command in your build output folder (assuming ninja is used as build generator):

```bash
ninja cargo_build
```

or for building and running rust tests:

```bash
ninja cargo_test
```

take a look at [Using cargo standalone](## Using cargo standalone) for information about how to setup your environment to use native cargo commands.

## Build Chain

This section explains briefly the order in which rust components are build by the build system. As mentioned before, rust packages are build as one target. A full run of the build-system usually contains the following sequence of rust related build tasks, which are executed in the following order:<br>

1. Rust binding generator, `rust_bindgen`, generates the required binding files.
2. `cargo_build` is executed, building all rust binaries.
3. `cargo_test` will recompile test binaries and execute the tests.
4. `cargo_doc` will generate documentation of rust code. note that rust code also need to be compilable in this step as code snippets inside the comments is build as well.

> Even though the majority of rust libraries dont require this, all dependencies of the `cargo_build` target are build prior. Making a split between packages that require linkage and those who dont would provide more flexibility. This is something we want to investigate in the future.

## Using cargo standalone

Using cargo natively can be preferable as it provides more flexibility for your workflows. It comes with some caveats though, which will be described in more detail in this paragraph. Note that the official way of building remains via the cmake build-system. Therefore calling cargo directly should only be done in local development scenarios.

> Make sure to do a full build via cmake prior using cargo standalone. This way you are sure all the native libs are present on your hard drive.

### Setting up environment

On the first time cmake is run, a script (`unify_env.source`) is generated which describes the environment the build system uses to compile rust sources. Since we are going to bypass the build-system, we need to replicate this environment. You have a couple of options on how to source this environment, which depends on your preference. There are a couple of things to take into consideration before choosing:

- The environment script is platform dependant. This means that a generated script for armhf cannot be used to build for x86_64. If you cycle a lot between building different targets a dynamic load of this environment script would be desirable.
- Changes to the cmake build-system requires, worst-case, a reload of this script.

We advise one of the following options:<br>

1. Static loading. Directly source the file inside of your bash profile:

```bash
echo ". unify_root/my_build_folder/unify_env.source" >> ~/.bashrc
```

2. Dynamic loading. We created a script that scans for multiple build directories and asks you wich environment to load. You can execute it manually every time. or choose to add it to your shell profile. i.e.

 ```bash
echo "unify_root/scripts/source_env_prompt.sh" >> ~/.bashrc
```

executing it would look like followed:

```
maccph0015787:scripts svradema$ ./source_env_prompt.sh 
please select the correct environment to source:
  [0]    /Users/svradema/git/unify/build
  [1]    /Users/svradema/git/unify/build_arm
0
sourced!
```

You should be good to go after sourcing the environment. From here on refer to the official [Cargo documentation](https://doc.rust-lang.org/book/ch01-03-hello-cargo.html)

## Creating a new rust project

1. Use the `cargo init <project_name>` command to setup an empty rust project. Use `--lib` argument to init a library type.
2. Add your project to the cargo workspace by appending it to the top level `Cargo.toml` file.

> When creating a new sys crate, post-pend the name with *_sys*!

### Creating Unit tests

- Inside Rust there is a distinction between integration test and unit-tests, familiarize yourself with the concept [here](https://doc.rust-lang.org/book/ch11-03-test-organization.html). Please follow up on the recommendations and put integration tests inside `rust_project/tests`
- Usually unit tests blow up the source files quite quickly, to put unit tests in a separate file could help you maintaining order in your source files.

## C/C++ to Rust

In order to use C in rust some steps need to be taken. You can read more about the overall concept [here](https://docs.rust-embedded.org/book/interoperability/c-with-rust.html). In short, we want to create a small wrapper project that imports the c symbols so that they can referenced correctly in Rust. Next to that, we need to tell the rust compiler how to link towards these symbols. The "cargo-bindgen" is a tool that generates this symbol definitions for you. In the next paragraph is explained how to declare the C libraries in rust as well as how to configure the binding generator.

### Binding generator

The binding generator is configured and run by the cmake build-system. As input this generator requires a configuration file, which describes what c symbols to generate bindings for. See `generate_rust_bindings` documentation on usage. Inside rust we have an macro which can easily import the generated bindings file as source code to your source file. Take a look at `unify_tools::include_binding` inside cargo doc.
> Every time you update the `DEPENDS` argument of the binding configuration. resource your unify environment file!

> The binding generator will install itself on your system the first time you run a build. You can run it manually on the command line execute `cargo bindgen -h` on the commandline for more info on the usage.

### Declaring native libraries

link dependencies need to be declared as dictated by the official cargo documentation. [read more](https://docs.rust-embedded.org/book/interoperability/c-with-rust.html). Since the our build environment already resolves all paths to unify libraries, only library names have to be passed as followed:

- in the project you imported the bindings, create an build.rs file.
- Inside the main function of this build.rs file you print the library to link towards as followed:

```rust
fn main() {
    // these exports are only used when an exe is compiled. e.g. integration
    // tests or final binaries!
    println!("cargo:rustc-link-lib=dylib=uic_log");
}
```

(here uic_log is taken as an example)<br>
your project should now build correctly!

### _sys crates

A good practice is to create a separate project which handles the import and linkage of a particular C library. The name of such a project is typically post-fixed with `_sys`. This way the transition between C to Rust stays contained in one place. Other projects can depend on this _sys crate to interface with C.

## Rust to C/C++

Linking of rust libraries inside C is also possible. An example is the `rust_command_classes` project. The following steps need to be executed to successfully link Rust libraries to C:

- Make sure the rust project is know to cmake. you do this by calling function `cargo_build` for your rust project. See the in code documentation for more information about the usage of this command.
- Declare your rust project as a static native library. add the following to your `Cargo.toml` file:

```toml
[lib]
crate-type = ["staticlib"]
```

- In cmake, add a dependency to your rust project. You want to add it to the target_link_libraries list in cmake. use the name `<rust_project>_target` to reference to the rust project. example:

```cmake
target_link_libraries(
  zpc
   ...
  rust_command_handlers_target)
```

you should now be all set!

> At the moment C headers have to be written manually, we don't have a generator in place to generate them.

## Crate types

We define 4 different kinds of rust crates (build targets) in our code-base:<br>

- <b>_sys crates.</b> The purpose of this crate is to expose a native c library as a rust library into rust. They require the native library they are wrapping in order to build. They make use of the rust binding generator to import c symbols. Example: unify_log_sys, zwave_command_classes_sys etc.
- <b>Binary crates</b>. Standalone crates that build binaries. examples are GMS, UPVL, image_provider.
- <b>Library crates</b>. Output rust library files (.rlib), Examples: unify_middleware.
- <b>Native library crates</b>. Libraries that build into native library format. (e.g. librust_command_classes.a) so that they can be used in C code.

## Cmake targets

the following build targets are available in cmake:

- `cargo_build`
- `cargo_test`
- `cargo_doc`

## Appendix: Generating test coverage report

Generating test reports is currently not part of the buildsystem, like we have for our C/C++. However it can be enabled if desired by using the following steps:  

1. Enable nightly for Rust and add code coverage component  
`rustup component add llvm-tools-preview`
2. Build with extra flags  
   `export RUSTFLAGS="-Zinstrument-coverage" cargo test`  
   In our case we should just modify `CMakeCargo.cmake` to add the RUSTFLAGS and run  
   `ninja cargo_test`
3. Run grcov on the generated files and filter out unittest code itself  
   `grcov . --binary-path ./target/ -s . -t cobertura --ignore-not-existing -o ./target/coverage/rust_cobertura.xml --excl-start "#\[test\]|mod tests \{|mod test \{" --ignore "build/bindings/*" --ignore "*_tests.rs" --ignore "build/*" --ignore "unify_tools/*" --ignore "*/build.rs" --ignore "*_test/*"`
   The output format can also be html if you replace `cobertura` with `html` and pass a folder path `-o ./target/coverage` instead.
4. (Optional) Combine the Rust cobertura report with the one for C  
E.g. `npx cobertura-merge -o gcov_results.xml package1=c_cobertura.xml package2=rust_cobertura.xml`
5. (Optional) Feed the final xml report to SonarQube

### Limitations

- Branch is not supported yet (ref: <https://github.com/taiki-e/cargo-llvm-cov/issues/8> and <https://github.com/rust-lang/rust/issues/79649>)
- Not supported on ARM platform (ref: <https://github.com/mozilla/grcov/issues/508>)
