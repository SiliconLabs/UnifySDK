# Unify Rust Development

In the UnifySDK we believe that rust will accelerate our development time while
providing more quality and safety. Currently few components are written in
Rust, but we are committed to put in a continues effort to migrate more
components. This document describes how to setup, build and develop rust
components for the Unify Framework.

> The docker images provided by the Unify Host SDK contain already everything
> you need to program Rust.

The complexity of building rust components and merging them with other build
artifacts is handled by the cmake build-system. This orchestration is not
dependent on any other 3th party software and does not requires a specific rust
version. This should keep maintenance hassle-free.
In cmake, rust compilation is seen as a isolated process that once it starts,
runs until the end. When successful, the artifacts of this build get imported
again as cmake targets. Inside cmake, take a look at the functions,
cargo_build, which is located in `cmake/modules/CMakeCargo.cmake`. It describes
how it works. The other way around, we want to make use of our c-code base in
rust. In order to do so we need to bind to the C interface and link to it. This
is all done using the provided cmake scripts.

## Using cargo standalone

If you used to work on rust projects you might be more comfortable using cargo
only to develop the rust parts. Note that the official way of building remains
via the cmake build-system. Therefore calling cargo directly should only be done
in local development scenarios.

> Make sure to do a full build via cmake prior using cargo standalone. This way
> you are sure all the native libs are present on your hard drive.

### Setting up environment

The following variables need to be set when running cargo standalone:
| variable        | description                                               |
|-----------------|-----------------------------------------------------------|
|UNIFY_BINARY_DIR | needs to point to cmake's build directory
|VERSION_STR      | needs to point to an version. for dev purposes an
arbitrary version can be chosen   |

Example:

```bash
cd build ninja export UNIFY_BINARY_DIR=$PWD export VERSION_STR="foo123"
```

## Creating new rust projects

Fitting rust components into the codebase comes with some additional cost, think
of marshaling data back and forth and the increase of complexity in the
build-system. Therefore we strongly advice to carefully pick which contenders
are right to code in rust. Generally a good contender is an component which is
for the most part decoupled with the rest of the code-base, or makes use of the
already available rust interfaces.

### Creating Unit tests

- Inside Rust there is a distinction between integration test and unit-tests,
  familiarize yourself with the concept
  [here](https://doc.rust-lang.org/book/ch11-03-test-organization.html). Please
  follow up on the recommendations and put integration tests inside
  `rust_project/tests`
- Usually unit tests blow up the source files quite quickly, to put unit tests
  in a separate file could help you maintaining order in your source files.

## C/C++ Binding generation

To call native C components 4 steps needs to be taken. You can read more about
the overall concept
[here](https://docs.rust-embedded.org/book/interoperability/c-with-rust.html).
In short, we want to create a small wrapper project that imports the c symbols
so that they can referenced correctly in Rust. Next to that, we need to tell the
Rust compiler how to link towards these symbols. We have tooling in place that
generates this symbol definitions for you.

### 1. Declare a cargo build

Add the cargo project as part of the cmake build. We created a cmake function
which does this for you. Refer to `cargo_build` for more info on usage.

### 2. Define link libraries

This can be via link-lib arguments inside your build.rs file. Click here for
[more info](https://doc.rust-lang.org/cargo/reference/build-scripts.html).
- Inside the main function of this build.rs file you print the library to link
  towards as followed:

```rust
fn main() { // these exports are only used when an exe is compiled. e.g.
integration // tests or final binaries!
println!("cargo:rustc-link-lib=dylib=uic_log"); }
```

Alternatively, we have a ninja importer. Which can read an build.ninja file and
extract the link information for you. See
unify_build_utils::compiler_arguments.

For usage browse through the components/uic_rust folder, there are enough
examples to be found.

### 3. Configure bindings

Configuration of bindings is done in the corresponding build.rs files. See
unify_build_utils::configure_bindings.

### 4. Include bindings

See unify_build_utils::include_binding macro

(here uic_log is taken as an example) your project should now build correctly!

### _sys crates

A good practice is to create a separate project which handles the import and
linkage of a particular C library. The name of such a project is typically
post-fixed with `_sys`. This way the transition between C to Rust stays
contained in one place. Other projects can depend on this _sys crate to
interface with C.

## Rust to C/C++

Linking of Rust libraries inside C is also possible. An example is the
`rust_command_classes` project. The following steps need to be executed to
successfully link Rust libraries to C:

- Make sure the Rust project is know to cmake. You do this by calling function
  `cargo_build` for your Rust project. See the in code documentation for more
  information about the usage of this command.
- Declare your Rust project as a static native library. Add the following to
  your `Cargo.toml` file:

```toml
[lib] crate-type = ["staticlib"]
```

- In cmake, add a dependency to your Rust project. You want to add it to the
  target_link_libraries list in cmake. use the name `<rust_project>` to
  reference to the Rust project. For example:

```cmake target_link_libraries( zpc ... rust_command_handlers) ```

> At the moment C headers have to be written manually, we don't have a generator
> in place to generate them.


