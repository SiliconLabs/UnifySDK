# Portable Environment GUI

The application rely on some platform specific tools, that are to be packaged and placed relative to the executable in a subfolder called _`./resources`_.

## Prepare runtime dependencies
To get started, move to portable runtime directory.
```console
cd ./portable_runtime
```

### Install tool dependencies
```console
sudo apt-get install nodejs npm libwebkit2gtk-4.0-dev build-essential mingw-w64 curl wget file \
libssl-dev libgtk-3-dev libayatana-appindicator3-dev librsvg2-dev gcc-mingw-w64
curl --proto '=https' --tlsv1.2 https://sh.rustup.rs -sSf  --output /tmp/sh.rustup.rs \
&& cd /tmp && chmod +x sh.rustup.rs && ./sh.rustup.rs --default-toolchain 1.71.0
```

### Install needed resources
```console
python3 ./scripts/prepare-resources.py --target <windows/linux>  --unify_path <path the unify zip containing .deb packages>
```

### Install GUI dependencies
To install GUI specific dependencies execute the below in _`unify_portable_gui`_ folder:

```console
npm install && npm run tauri build
```

## Build Portable GUI
_For Windows_:
```console
rustup target add x86_64-pc-windows-gnu
cargo build --target x86_64-pc-windows-gnu --release
```
_For Linux_:
```console
cargo build --target x86_64-unknown-linux-gnu --release
```
