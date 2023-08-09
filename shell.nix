{ pkgs ? import <nixpkgs> { }
, cpcd
, zw-testframework
, ths-cmock
, ths-unity
, gecko-sdk
, packages
}:
with pkgs;
gcc10Stdenv.mkDerivation rec {
  name = "Unify SDK development shell";
  src = self;

  # Build input: boost
  boost_pinned = pkgs.boost174.overrideAttrs (o: {
    enableMultiThreaded = true;
    enableStatic = true;
  });
  boost = boost_pinned.override { stdenv = pkgs.gcc10Stdenv; };
  cmakeFlags = [
    "-GNinja"
    # The find boost script that comes with cmake prioritizes installations in the system.
    # Disable this so the version in the nix store can be found.
    "-DBoost_NO_BOOST_CMAKE=TRUE"
    "-DCMAKE_INSTALL_PREFIX=install"
    "-DCOMMON_LOCATION=${packages.common}"
    "-DZW-TESTFRAMEWORK_LOCATION=${zw-testframework}"
    "-DTHS-CMOCK_LOCATION=${ths-cmock}"
    "-DTHS-UNITY_LOCATION=${ths-unity}"
    "-DCPCD_LOCATION=${cpcd}"
    "-DGSDK_LOCATION=${gecko-sdk}"
    "-DUNIFY-TESTFRAMEWORK_LOCATION=${self}/components/testframework"
    "-DBUILD_TESTING=false"
    # Opt out of building any and all components
    "-DBUILD_ZPC=OFF"
    "-DBUILD_ZIGPC=OFF"
    "-DBUILD_DEV_GUI=OFF"
    "-DBUILD_UIC_DEMO=OFF"
    "-DBUILD_UPVL=OFF"
    "-DBUILD_GMS=OFF"
    "-DBUILD_IMAGE_PROVIDER=OFF"
    "-DBUILD_AOXPC=OFF"
    "-DBUILD_POSITIONING=OFF"
    "-DBUILD_NAL=OFF"
    "-DBUILD_UPTI_CAP=OFF"
    "-DBUILD_UPTI_WRITER=OFF"
    "-DBUILD_CPCD=OFF"
    "-DBUILD_ZIGBEED=OFF"
    "-DBUILD_OTBR=OFF"
    "-DBUILD_MATTER_BRIDGE=OFF"
    "-DBUILD_EPC=OFF"
    "-DBUILD_EMD=OFF"
  ];
  enableParallelBuilding = true;

  nativeBuildInputs = [
    cmake
    ninja
    just
    clang
    python-pinned.packages
  ];
  buildInputs = [
    # dev tool
    bacon
    sd
    nixpkgs-fmt
    # build tools
    boost
    nodejs
    # klunge
    git
    graphviz
    doxygen
    mbedtls
    (libyamlcpp.override { stdenv = pkgs.gcc10Stdenv; })
    nlohmann_json
    (gtest.override { stdenv = pkgs.gcc10Stdenv; })
    mosquitto
    sqlite
    ruby
    libedit
    cpcd.packages.${system}.cpcd
    rust-bin.stable."1.65.0".default
  ];
  LIBCLANG_PATH = "${pkgs.llvmPackages.libclang.lib}/lib";

  shellHook = ''
    alias build='cmake -S . -B build ${ builtins.concatStringsSep " " cmakeFlags } && ninja -C build install'
  '';
}
