{ pkgs ? import <nixpkgs> { }
, filter
, cpcd
, common
, zw-testframework
, ths-cmock
, ths-unity
}:
with pkgs;
(rustPlatform.buildRustPackage.override { stdenv = pkgs.gcc10Stdenv; }) rec {
  name = "libunify";
  version = "0.1.0";
  src = ./components;

  cargoLock = {
    lockFile = ./components/Cargo.lock;
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
    clang
  ];
  buildInputs = [
    python310
    boost
    (libyamlcpp.override { stdenv = pkgs.gcc10Stdenv; })
    nlohmann_json
    (gtest.override { stdenv = pkgs.gcc10Stdenv; })
    mosquitto
    sqlite
    ruby
    libedit
    doxygen
    rust-bin.stable."1.65.0".default
    cpcd.packages.${system}.cpcd
  ];
  cmakeFlags = [
    "-GNinja"
    # The find boost script that comes with cmake prioritizes installations in the system. 
    # Disable this so the version in the nix store can be found.
    "-DBoost_NO_BOOST_CMAKE=TRUE"
    "-DCMAKE_INSTALL_PREFIX=install"
    "-DCOMMON_LOCATION=${common}"
    "-DZW-TESTFRAMEWORK_LOCATION=${zw-testframework}"
    "-DTHS-CMOCK_LOCATION=${ths-cmock}"
    "-DTHS-UNITY_LOCATION=${ths-unity}"
    "-DUNIFY-TESTFRAMEWORK_LOCATION=${self}/components/testframework"
    "-DBUILD_TESTING=false"
  ];
  enableParallelBuilding = true;

  # Bindgen
  LIBCLANG_PATH = "${pkgs.llvmPackages.libclang.lib}/lib";
  doCheck = false;

  preConfigure = ''
    export HOME=$(mktemp -d)
    export COMMON_LOCATION=${common}
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
    ${"-isystem ${stdenv.cc.cc}/include/c++/${pkgs.lib.getVersion stdenv.cc.cc} -isystem ${stdenv.cc.cc}/include/c++/${pkgs.lib.getVersion stdenv.cc.cc}/${stdenv.hostPlatform.config} -idirafter ${stdenv.cc.cc}/lib/gcc/${stdenv.hostPlatform.config}/${pkgs.lib.getVersion stdenv.cc.cc}/include"}"
  '';
  buildPhase = ''
    cmake -S . -B build $cmakeFlags
    ninja -C build unify
  '';
  installPhase = ''
    ninja -C build install unify
    mkdir -p $out
    mv $HOME/../components/install/* $out
    mkdir -p $rust
    cp -r $src/uic_rust $rust
    cp -r $src/unify_attribute_poll $rust
    mkdir -p $testframework
    cp -r $src/testframework/* $testframework
  '';
  outputs = [ "out" "rust" "testframework" ];
  dontFixup = true;
  dontStrip = true;

  meta = with lib; {
    description = "SiliconLabs Unify SDK library";
    homepage = "https://github.com/SiliconLabs/UnifySDK";
    license = "www.silabs.com/about-us/legal/master-software-license-agreement";
    maintainers = with maintainers; [ ehnielse ];
  };
}
