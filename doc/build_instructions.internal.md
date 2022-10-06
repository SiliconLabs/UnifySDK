# Build Instructions Internal

## Debian Buster

This document describes how to use a VM or native for building the Unify Host SDK.

See `Dockerfile` for dependencies if this document gets outdated.

### Install Dependencies on Ubuntu 20.04

``` bash
add-apt-repository ppa:mosquitto-dev/mosquitto-ppa
apt-get update
apt install --no-install-recommends build-essential ninja-build \
    git git-lfs python3 python3-pip \
    ruby graphviz plantuml gcovr zip pkg-config curl \
    libmosquitto-dev libsqlite3-dev libedit-dev libyaml-cpp-dev \
    libboost-program-options-dev libboost-log-dev libboost-regex-dev \
    nlohmann-json3-dev clang

# Install Homebrew:
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install various tools from brew to get newer version than available using apt
brew install cmake doxygen

# Install Rust (Check the docker/Dockerfile for which version to use):
# Latest stable can be installed by doing `curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh` instead
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain 1.58.1

# Install PyBars3
python3 -m pip install PyBars3

# Install GUI dependencies
curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | sudo apt-key add - \
  && echo "deb https://dl.yarnpkg.com/debian/ stable main" | sudo tee /etc/apt/sources.list.d/yarn.list \
  && sudo apt-get update \
  && sudo apt-get install --no-install-recommends -y curl  ca-certificates git dpkg fakeroot \
  && curl -sL https://deb.nodesource.com/setup_12.x | sudo bash - \
  && sudo apt-get install --no-install-recommends -y nodejs rpm \
  && sudo apt-get install --no-install-recommends -y yarn

# Install ZAP (From Unify source files there is a debian package installer)
uic/docker/uic-resources$ apt-get install ./zap_*_amd64.deb
```

### Build on Debian Buster or Ubuntu 20.04

Within the Unify Host SDK source directory, run the following command (the following
example uses Ninja for building, but you can use make instead):

``` bash
mkdir build
cd build
cmake -GNinja ..
ninja
```

## MacOs (Mojave)

**NOTE: Building on MacOs is unofficial and not directly supported
and not all unit tests are executed when building on MacOs.**

### Install Dependencies on MacOs

Prerequisites:

* Install Homebrew (<https://brew.sh/)>.
* Install zap-*.dmg

To acquire the correct zap-*.dmg go to [uic-resources](https://stash.silabs.com/projects/UIC/repos/uic-resources) then find the latest link to jenkins zap build in commit messages. This could look like this:

```text
Updated ZAP to 2021-04-06 JBN_1444

Build: http://iot-jenkins-master:8080/job/ZAP/job/silabs/1444/

ZAP commit hash: 59cbc37628f608695f2d4a38755980792c6ce382

Reason to update:
- the "array" field is properly parsed for command arguments
```

Where one can acquire the zap-*.dmg at **<http://iot-jenkins-master:8080/job/ZAP/job/silabs/1444/>** (**IMPORTANT:** Please go find the latest link your self)

``` bash
brew install cmake boost python doxygen graphviz ninja ruby mosquitto sqlite yaml-cpp nlohmann-json mactex

# Dev-GUI Dependencies
brew install nvm
mkdir ~/.nvm

# Install node version 12
nvm install 12
nvm use 12
npm install --global yarn
```

### Build on MacOs

Within the Unify Host SDK source directory, run the following:

``` bash
mkdir build
cd build
cmake -GNinja ..
ninja
```

### Applications output

These build instructions will build and compile the following applications:

* **Z-Wave Protocol Controller**
* **Zigbee Protocol Controller**
* **Unify Framework Provisioning List**
* **Unify Framework Developer GUI**
