{
  description = "Unify SDK flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    rust-overlay.url = "github:oxalica/rust-overlay";
    flake-utils.url = "github:numtide/flake-utils";
    nix-filter.url = github:numtide/nix-filter;
    zw-testframework = {
      url = "./components/testframework/libs/testframework";
      flake = false;
    };
    ths-cmock = {
      url = "./components/testframework/libs/cmock";
      flake = false;
    };
    ths-unity = {
      url = "./components/testframework/libs/cmock/vendor/unity";
      flake = false;
    };
    gecko-sdk = {
      url = "github:SiliconLabs/gecko_sdk?rev=d4854d2ff06a828161d7032efb1736d0deda40b1";
      flake = false;
    };
    cpcd = {
      url = "github:SiliconLabs/unify-sdk-flakes?rev=7de893c04150bfcb0f25823fc105bcae514cb2bc";
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
          zpc = import ./zpc.nix { inherit pkgs cpcd common libunify ths-unity; };
          default = libunify;
        };
        devShells.default = import ./shell.nix { inherit pkgs cpcd zw-testframework ths-cmock ths-unity gecko-sdk packages; };
        apps.zpc = flake-utils.lib.mkApp { drv = packages.zpc; };      
      }
      );
}
