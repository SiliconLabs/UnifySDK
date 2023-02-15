{ pkgs ? import <nixpkgs> { }
, filter
}:
with pkgs;
stdenv.mkDerivation {
  name = "common";
  src = filter {
    root = ./.;
    include = [
      "cmake"
      "include"
      "scripts"
      ./copyright
    ];
  };

  dontBuild = true;
  installPhase = ''
    mkdir -p $out
    cp -r . $out
  '';
  dontFixup = true;
  dontStrip = true;

  meta = with lib; {
    description = "SiliconLabs Unify SDK common files";
    homepage = "https://github.com/SiliconLabs/UnifySDK";
    license = "www.silabs.com/about-us/legal/master-software-license-agreement";
    maintainers = with maintainers; [ ehnielse ];
  };
}
