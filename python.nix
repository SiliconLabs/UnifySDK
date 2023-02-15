{ lib
, stdenv
, python310
, python310Packages
}:

stdenv.mkDerivation rec {
  name = "uic_python_dependencies";
  pymeta3 = python310Packages.buildPythonPackage rec {
    pname = "PyMeta3";
    version = "0.5.1";
    src = python310Packages.fetchPypi {
      inherit pname version;
      sha256 = "GL2jJtmpu/WHv8DuC8loZJZNeLBnKIvPVdTZhoHQW8s=";
    };
    doCheck = false;
  };
  pybars3 = python310Packages.buildPythonPackage rec {
    pname = "pybars3";
    version = "0.9.7";
    src = python310Packages.fetchPypi {
      inherit pname version;
      sha256 = "ashH6QXlO5xbk2rxEskQR14nv3Z/efRSjBb5rx7A4lI=";
    };
    propagatedBuildInputs = with python310Packages; [ pymeta3 ];
    doCheck = false;
  };
  pythonExtra = python310.buildEnv.override {
    extraLibs = [
      pybars3
      pymeta3
    ];
  };
  packages = pythonExtra.withPackages (p: with p; [
    pybars3
    pymeta3
    gcovr
    xmltodict
    sphinx
    breathe
    # myst-parser # dependency issue: python3.8-pandas-1.4.3 is broken
    linkify-it-py
    # sphinxcontrib-plantuml # dependency issue: openjdk-headless-16+36 is marked as unsecure
    # sphinx-markdown-tables
    # sphinx-rtd-theme
  ]);
}