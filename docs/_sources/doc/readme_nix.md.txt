# Nix (Experimental)

We are adding support for [`nix flakes`](https://nixos.wiki/wiki/Flakes) to make use of the [nix package manager](https://nixos.org/manual/nix/stable/). 
This aims to let developers leverage the benefits of using a [purely functional package manager](https://nixos.org/manual/nix/stable/).

> Why use `nix`? In short, it gives you deterministic and reproducible builds at native speeds and an augmented shell with all the development dependencies.
> In contrast, our container does not give you these guarantees and may perform significantly worse than native, and the container constrains your development environment.

We have added a set of conveniences in a `justfile`.
You can see all the recipes by running `> just`.

## Build

To build `libunify` run `> just build`. 
To build `zpc` run `> just build zpc`.
You can see the all the outputs provided by the flake by running `> just show`.

You can build with `nix` without enabling flakes by running the commands that the `justfile` provides conveniences for, like building the default derivation.

```bash
nix --experimental-features 'nix-command flakes' build
```
This is equivalent to

```bash
nix --experimental-features 'nix-command flakes' build .#libunify
```

because `libunify` is the default derivation.

If [building](https://nixos.org/manual/nix/unstable/command-ref/nix-build.html#description) succeeds you'll see the result symlinked to `./result`.

## Run

The application in the project can likewise be run in a pleasant and easy way leveraging `nix`. You can run `zpc` with `> just run zpc`.

> You can run applications without having to check out the project with `> nix run github:SiliconLabs/UnifySDK#zpc` once this work gets published.

## Development environment

### First-class experience

Use [`direnv`](https://direnv.net/) combined with [`nix-direnv`](https://github.com/nix-community/nix-direnv) to automatically augment your shell when you navigate to the project directory.

Create `.direnv.rc`

```bash
───────┬───────────────────────────────────────────────────────────────────
       │ File: .direnv.rc
───────┼───────────────────────────────────────────────────────────────────
   1   │ if [ -f $HOME/.nix-profile/share/nix-direnv/direnvrc ]; then
   2   │     source $HOME/.nix-profile/share/nix-direnv/direnvrc
   3   │ fi
───────┴───────────────────────────────────────────────────────────────────
```

and give `direnv` permission to load the `.envrc` present in the project directory.

```bash
> direnv allow
```

### Nix development shell

To start the interactive development shell run `> just develop`. 
This drops you into the dev shell with all the dependencies you need to develop for any and all aspects of the Unify SDK. 
_It will take a while to build the third party dependencies from source the first time you do this._

## Dependencies

- [`nix`](https://nixos.org/download.html)

## Optional dependencies

- [`just`](https://github.com/casey/just#installation)
- [`direnv`](https://direnv.net/)
- [`nix-direnv`](https://github.com/nix-community/nix-direnv)
