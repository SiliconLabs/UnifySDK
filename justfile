#!/usr/bin/env -S just --justfile
# ^ A shebang isn't required, but allows a justfile to be executed
#   like a script, with `./justfile test`, for example.

_default:
    @just --list

_nix command:
	@nix --experimental-features 'nix-command flakes' {{command}} 

# Remove temporary directories
clean:
	@rm -rf build install result result-* source
	@echo -e "\033[97;1mnote:\033m \033[;0mRemoved temporary directories\033[0m" 

# Initialise submodules
init:
	@git submodule update --recursive --init --depth 1
	@echo -e "\033[97;1mnote:\033m \033[;0mSubmodules were initialised\033[0m" 
    
# Development shell
develop package='default':
	@just _nix "develop .#{{package}}"

_build package options='--debugger --print-build-logs --print-out-paths':
	@nix --experimental-features 'nix-command flakes' build {{options}} .#{{package}}

# Build package
build package='libunify':
	@just _build {{package}}
	
# Rebuild package
rebuild package='libunify':
	@just _build {{package}} '--debugger --print-build-logs --print-out-paths --rebuild'

# Format the nix files
fmt:
	@nixpkgs-fmt *.nix

# Run an application
run package='zpc': 
	@just _nix "run .#{{package}}"

# Run --help for an application
help package='zpc': 
	@just _nix "run .#{{package}} -- --help"

# Check the nix files
check *options:
	@just _nix "flake check {{options}}"

# Show what packages can be built
show: (_nix "flake show")

# Search logs
logs:
	@fd '.*\.drv' /nix/store | fzf | xargs nix log --extra-experimental-features nix-command

# Render dependency graph of ./result to ./deps.png
deps package='libunify':
	@just _build {{package}}
	@nix-store -q --graph result | dot -T png -o deps.png -Ktwopi -Goverlap=false -Gsplines=true

# Build a package derivation for some target or fetch a store path
cross package='libunify' target='raspberryPi':
	@just _nix "build .#pkgsCross.{{target}}.{{package}}"
