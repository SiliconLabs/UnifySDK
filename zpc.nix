{ pkgs ? import <nixpkgs> { }
, cpcd
, common
, libunify
  , zw-libs2
, ths-unity
}:
with pkgs;
(rustPlatform.buildRustPackage.override { stdenv = pkgs.gcc10Stdenv; }) rec {
  pname = "zpc";
  version = "0.1.0";
  src = ./applications/zpc;

  cargoLock = {
    lockFile = ./applications/zpc/Cargo.lock;
  };

  # Build inputs
  boost_pinned = pkgs.boost174.overrideAttrs (o: {
    enableMultiThreaded = true;
    enableStatic = true;
  });
  boost = boost_pinned.override { stdenv = pkgs.gcc10Stdenv; };
  nativeBuildInputs = [
    cmake
    ninja
    python-pinned.packages
    clang
  ];
  buildInputs = with pkgs; [
    libunify
    boost
    libedit
    nlohmann_json
    rust-bin.stable."1.65.0".default
  ];
  cmakeFlags = [
    "-GNinja"
    # The find boost script that comes with cmake prioritizes installations in the system. 
    # Disable this so the version in the nix store can be found.
    "-DBoost_NO_BOOST_CMAKE=TRUE"
    "-DCMAKE_INSTALL_PREFIX=install"
    "-DTHS-UNITY_LOCATION=${ths-unity}"
    "-DCOMMON_LOCATION=${common}"
    "-DZW-LIBS2_LOCATION=${zw-libs2.packages.${system}.zw-libs2}"
    "-DUNIFY-TESTFRAMEWORK_LOCATION=${libunify.testframework}"
    "-DLIBUNIFY_LOCATION=${libunify}"
    "-DBUILD_TESTING=OFF"
  ];
  enableParallelBuilding = true;

  # Bindgen
  LIBCLANG_PATH = "${pkgs.llvmPackages.libclang.lib}/lib";
  doCheck = false;

  preConfigure = ''
    export HOME=$(mktemp -d)
    export COMMON_LOCATION=${common}
    export CARGO_LIBUNIFY_DIR=${libunify}
    export RUST_BACKTRACE=1
  '';
  prePatch = ''
    sed -ri 's#\.{2}\/\.{2}\/\.{2}\/\.{2}\/components#${libunify.rust}#g' components/zwave_rust_proc_macros/Cargo.toml
    sed -ri 's#\.{2}\/\.{2}\/\.{2}\/\.{2}\/components#${libunify.rust}#g' components/zwave_rust_proc_macros_legacy/Cargo.toml
    sed -ri 's#\.{2}\/\.{2}\/\.{2}\/\.{2}\/components#${libunify.rust}#g' components/zpc_rust/Cargo.toml
  '';
  preBuild = ''
    # From: https://github.com/NixOS/nixpkgs/blob/1fab95f5190d087e66a3502481e34e15d62090aa/pkgs/applications/networking/browsers/firefox/common.nix#L247-L253
    # Set C flags for Rust's bindgen program. 
    # Unlike ordinary C compilation, bindgen does not invoke $CC directly. 
    # Instead it uses LLVM's libclang. 
    # To make sure all necessary flags are included we need to look in a few places.
        
    export BINDGEN_EXTRA_CLANG_ARGS="$(< ${stdenv.cc}/nix-support/libc-crt1-cflags) \
    $(< ${stdenv.cc}/nix-support/libc-cflags) \
    $(< ${stdenv.cc}/nix-support/cc-cflags) \
    $(< ${stdenv.cc}/nix-support/libcxx-cxxflags) \
    -x --c++ -std=c++14 \
    ${"-isystem ${stdenv.cc.cc}/include/c++/${pkgs.lib.getVersion stdenv.cc.cc} -isystem ${stdenv.cc.cc}/include/c++/${pkgs.lib.getVersion stdenv.cc.cc}/${stdenv.hostPlatform.config} -idirafter ${stdenv.cc.cc}/lib/gcc/${stdenv.hostPlatform.config}/${pkgs.lib.getVersion stdenv.cc.cc}/include"}"
  '';
  buildPhase = ''
    mkdir -p $out
    cmake -S . -B build $cmakeFlags
    ninja -C build $pname
  '';
  installPhase = ''
    ninja -C build install zpc
    mkdir -p $out
    mv $HOME/../zpc/install/* $out
  '';
  dontFixup = true;
  dontStrip = true;

  meta = with lib; {
    description = "SiliconLabs Unify SDK zpc";
    homepage = "https://github.com/SiliconLabs/UnifySDK";
    license = "www.silabs.com/about-us/legal/master-software-license-agreement";
    maintainers = with maintainers; [ ehnielse ];
  };
}
