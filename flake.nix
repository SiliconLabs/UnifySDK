{
  description = "Unify SDK flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    rust-overlay.url = "github:oxalica/rust-overlay";
    flake-utils.url = "github:numtide/flake-utils";
    nix-filter.url = github:numtide/nix-filter;
    zw-testframework = {
      url = "git+ssh://git@stash.silabs.com/z-wave/zw-testframework?rev=cbbf779dfddcabd1470e5f8d7208f903fe716129";
      flake = false;
    };
    ths-cmock = {
      url = "git+ssh://git@stash.silabs.com/z-wave/ths_cmock?rev=0d7a1a1c1ca1a51c96b8f606153daf31cebea6d9";
      flake = false;
    };
    ths-unity = {
      url = "git+ssh://git@stash.silabs.com/z-wave/ths_unity?rev=b0032caca4402da692548f2ee296d3b1b1251ca0";
      flake = false;
    };
    zw-libs2 = {
      url = "git+ssh://git@stash.silabs.com/uic/zw-libs2-flake?rev=4c25c776f99b0e2422cd5588cb11c6f28b267b9a";
    };
    uic-resources = {
      url = "git+ssh://git@stash.silabs.com/uic/uic-resources?ref=develop";
      flake = false;
    };
    gecko-sdk = {
      url = "github:SiliconLabs/gecko_sdk?rev=d4854d2ff06a828161d7032efb1736d0deda40b1";
      flake = false;
    };
    cpcd = {
      url = "git+ssh://git@stash.silabs.com/uic/cpc-daemon-flake.git?rev=e40cf898b8bb80aad70e1349539a8aaf889ea99d";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    { self
    , nixpkgs
    , rust-overlay
    , flake-utils
    , nix-filter
    , zw-testframework
    , ths-cmock
    , ths-unity
    , zw-libs2
    , uic-resources
    , gecko-sdk
    , cpcd
    }:

    flake-utils.lib.eachSystem [ "x86_64-linux" ]
      (system:
      let
        overlays = [ 
          (import rust-overlay)
          (import ./overlay.nix)
        ];
        pkgs = import nixpkgs {
          inherit system overlays;
        };
        filter = nix-filter.lib.filter;
      in
      rec
      {
        packages = rec {
          common = import ./common.nix { inherit pkgs filter; };
          libunify = import ./libunify.nix { inherit pkgs filter cpcd common zw-testframework ths-cmock ths-unity; };
          zpc = import ./zpc.nix { inherit pkgs cpcd common libunify zw-libs2 ths-unity; };
          default = libunify;
        };
        devShells.default = import ./shell.nix { inherit pkgs cpcd zw-testframework ths-cmock ths-unity gecko-sdk packages; };
        apps.zpc = flake-utils.lib.mkApp { drv = packages.zpc; };      
      }
      );
}
