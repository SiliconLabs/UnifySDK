# Portable environment

At the moment the Portable Runtime is comprised of a command line application (CLI), _`unify_portable_cli`_, with support for Linux, MacOS and Windows on the x86_64 architecture.
There is also an experimental GUI, _unify_portable_gui_, that for the time being, has only been tested on Windows.

The applications are written in Rust and rely on some platform specific tools, that are to be packaged and placed relative to the executable in a subfolder called _`./resources`_.

## Prepare runtime dependencies
The runtime dependencies for the applications, can be downloaded by running the Python script located in _`./scripts/prepare-resources.py`_.
This script contains links for the various dependencies and as such downloads and extracts them to the correct location in the local filesystem.

It makes sense to run this before starting to compile the application, as one can then just run `cargo build --target x86_64-unknown-linux-gnu --release --package unify_portable_cli` from the top level `portable_runtime` folder.

The script has some arguments that needs to be specified so it knows where to place the dependencies in the filesystem.
You should specify which target OS you are building for with the `--target` argument.
With the `--build_type` argument you specify whether it is a `debug` or a `release` build.
The last argument is for the case where you decice to build the Rust project in a different build folder.
For this case, you can specify the location of the top level build folder, usually named `target`in Rust, with the argument `--build_folder`.

## Package the Portable Runtime

The `scripts` folder contains a simple packaging script for generating a deployment package for the Portable Runtime.
The script should be executed within ´uic/portable_runtime´ like so:

```bash
portable_runtime$ python3 ./scripts/package_portable_environments.py --target windows --build_type release
```

This will produce a compressed archive, a *.zip file for Windows and *.tar.bz2 file for Linux & MacOS, of the CLI tool and the related runtime dependencies.
