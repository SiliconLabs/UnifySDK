ARG FROM="debian:buster"
FROM ${FROM}

SHELL ["/bin/bash", "-c"]

ARG USER_ID=1000
ARG GROUP_ID=1000
# Set to true to install clang toolchain
ARG CLANG=false
# Architectures
ARG ARCH="armhf"
ENV HEADLESS_HOST=true

# Target dependenciesls
ARG TARGET_DEPS="libssl-dev libsystemd-dev libwrap0-dev uthash-dev \
  libsqlite3-dev libedit-dev libyaml-cpp0.6 libmosquitto-dev\
  libreadline-dev libncurses5-dev libncursesw5-dev libyaml-cpp-dev \
  libboost-atomic-dev libboost-chrono-dev libboost-date-time-dev \
  libboost-filesystem-dev libboost-regex-dev libboost-program-options-dev \
  libboost-serialization-dev libboost-system-dev libboost-thread-dev \
  libboost-log-dev procps"

# Host and Multiarch dependencies
ARG HOST_DEPS="ninja-build git git-lfs zip unzip curl pkg-config \
  xsltproc flex bison valgrind \
  python3 python3-pip python3-setuptools python3-wheel \
  ruby ruby-dev sudo \
  mscgen graphviz \
  gcc g++ gdb-multiarch \
  qemu-user binfmt-support qemu-user-static \
  build-essential devscripts debhelper \
  rsync acl zlib1g-dev python3-dev libxml2-dev libxslt-dev\
  openjdk-11-jre\
  clang nlohmann-json3-dev"

ENV DEBIAN_FRONTEND=noninteractive

# configure additional arches in dpkg/apt
RUN for arch in ${ARCH} ; do \
  dpkg --add-architecture $arch ; \
  done

# Do the apt-get install of all the above configured packages
RUN apt-get update \
  && apt-get install --no-install-recommends -y curl gnupg2 ca-certificates \
  #&& curl -sL http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key | apt-key add - \
  #&& curl -sL http://repo.mosquitto.org/debian/mosquitto-buster.list > /etc/apt/sources.list.d/mosquitto-buster.list \
  && apt-get update \
  && apt-get install --no-install-recommends -y \
  ${HOST_DEPS} \
  $(for dep in ${TARGET_DEPS} ; do \
  for arch in ${ARCH} ; do echo $dep:$arch ; done ; \
  done) \
  $(for arch in ${ARCH} ; do echo crossbuild-essential-${arch} ; done)\
  && rm -rf /var/lib/apt/lists/*

# Install cmake 3.19.2
RUN curl -L https://github.com/Kitware/CMake/releases/download/v3.19.2/cmake-3.19.2-Linux-x86_64.sh --output /tmp/cmake-3.19.2-Linux-x86_64.sh \
  && cd /tmp \
  && echo 2fc84c1bd5a5fa8850426905a76147fbf897cf67ef324b009bcdb7eceafa9662 cmake-3.19.2-Linux-x86_64.sh > /tmp/cmake-3.19.2-Linux-x86_64.sh.sha256 \
  && sha256sum -c cmake-3.19.2-Linux-x86_64.sh.sha256 \
  && chmod +x /tmp/cmake*.sh \
  && /tmp/cmake*.sh --prefix=/usr/local --skip-license \
  && rm /tmp/cmake*

# Install pip3 packages
RUN pip3 install gcovr \
  pybars3==0.9.7 PyMeta3==0.5.1 xmltodict==0.12.0

# Install doxygen 1.8.20 to fix issues with preprocessor defines in code blocks in markdown files
RUN curl -L https://github.com/doxygen/doxygen/archive/Release_1_8_20.zip --output /tmp/doxygen.zip \
  && cd /tmp/ \
  && echo 1d31d5dd3b486aef55ba77d16ef3307a9d936518725b639c5709a1d859f0f732a4e9a13f457340d4f18bd8a07bd1c96ba5cc91abaedd69896142331e19fc8540 doxygen.zip > /tmp/doxygen.zip.sha512 \
  && sha512sum -c doxygen.zip.sha512 \
  && unzip doxygen.zip \
  && cd /tmp/doxygen*/ \
  && mkdir build && cd build \
  && cmake -GNinja .. \
  && ninja && ninja install \
  && rm -rf /tmp/doxygen*

# Install doxygen latex had to install additional packages to accomodate for doxygen 1.8.20 version
RUN apt-get update \
  && apt-get install --no-install-recommends -y cm-super-minimal ghostscript texlive-extra-utils \ 
  texlive-font-utils texlive-fonts-recommended texlive-latex-extra \
  texlive-latex-recommended texlive-plain-generic 


# manually compile and install mosquitto v2.0.11. 
ARG armhf_cmake_params='-DCMAKE_C_COMPILER=/usr/bin/arm-linux-gnueabihf-gcc -DCMAKE_CXX_COMPILER=/usr/bin/arm-linux-gnueabihf-g++ -DPACK_DEBIAN_PACKAGE_ARCHITECTURE=armhf \
  -DCPACK_FILE_NAME_EXTRA=buster -DPKG_CONFIG_EXECUTABLE=usr/bin/arm-linux-gnueabihf-pkg-config -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=armhf -DCONFIGURE_HOST=arm-linux'

RUN git clone --depth 1 --branch v2.0.11 https://github.com/eclipse/mosquitto.git /tmp/mosquitto && mkdir /tmp/mosquitto/build && cd /tmp/mosquitto/build \
  && cmake -DWITH_APPS=OFF -DWITH_PLUGINS=OFF -DDOCUMENTATION=OFF `if [ "${ARCH}" == "armhf" ]; then echo ${armhf_cmake_params}; else echo ""; fi` .. && make && sudo make install

# ld.so.cache is causing segfault grief with qemu and endianness differences:
# https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=731082. Fortunately, qemu
# will redirect file operations to the ld prefix given with QEMU_LD_PREFIX if
# the file in question exists there. Fortunately again, on Debian due to their
# multiarch magic, ld.so does not seem to need to actually reside there. So we
# only have to make sure that a (dummy) ld.so.cache exists in the prefix to
# avoid ld.so croaking on the host's ld.so.cache.
RUN mkdir -p /dummyroot/etc
RUN touch /dummyroot/etc/ld.so.cache
ENV QEMU_LD_PREFIX=/dummyroot

# Rust and Cargo home directories
ENV RUSTUP_HOME=/opt/rustup-home
ENV CARGO_HOME=/opt/cargo-home
# Install Rust and Cargo
RUN curl https://sh.rustup.rs -sSf --output /tmp/sh.rustup.rs \
  && cd /tmp && chmod +x sh.rustup.rs \
  && if [[ $ARCH == *"armhf"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES armv7-unknown-linux-gnueabihf"; fi \
  && if [[ $ARCH == *"amd64"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES x86_64-unknown-linux-gnu"; fi \
  && ./sh.rustup.rs -y --profile minimal --target ${RUST_TRIPLES} \
  && rm /tmp/sh.rustup.rs \
  && /opt/cargo-home/bin/cargo install cargo-deb cargo2junit \
  && chmod -R a+rw ${RUSTUP_HOME} ${CARGO_HOME} \
  && find ${RUSTUP_HOME} ${CARGO_HOME} -type d -exec chmod a+x {} \;
ENV PATH="${CARGO_HOME}/bin:${PATH}"

# SonarScanner (used on Jenkins)
RUN curl -sL https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-4.5.0.2216.zip --output /opt/sonar-scanner-cli.zip \
  && echo a271a933d14da6e8705d58996d30afd0b4afc93c0bfe957eb377bed808c4fa89 /opt/sonar-scanner-cli.zip > /tmp/sonar-scanner-cli.zip.sha256 \
  && sha256sum -c /tmp/sonar-scanner-cli.zip.sha256 \
  && unzip /opt/sonar-scanner-cli.zip -d /opt/ \
  && rm /opt/sonar-scanner-cli.zip \
  && mv /opt/sonar-scanner-* /opt/sonar-scanner-cli
ENV PATH="/opt/sonar-scanner-cli/bin:${PATH}"

# Plantuml
RUN curl -L https://sourceforge.net/projects/plantuml/files/1.2021.0/plantuml.1.2021.0.jar/download --output /opt/plantuml.jar \
  && echo 40446bb1307c18ef81f0366497546eaeef41c3eb8bf688d06c9df6d6c6ea230b /opt/plantuml.jar > /tmp/plantuml.jar.sha256 \
  && sha256sum -c /tmp/plantuml.jar.sha256
ENV PLANTUML_JAR_PATH=/opt/plantuml.jar

# DEV_UI dependencies
RUN curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add - \
  && echo "deb https://dl.yarnpkg.com/debian/ stable main" > /etc/apt/sources.list.d/yarn.list \
  && apt-get update \
  && apt-get install --no-install-recommends -y curl  ca-certificates git dpkg fakeroot \
  && curl -sL https://deb.nodesource.com/setup_12.x | bash - \
  && apt-get install --no-install-recommends -y nodejs rpm \
  && apt-get install --no-install-recommends -y yarn \
  && rm -rf /var/lib/apt/lists/*

# Install ZAP and dependencies
# TODO: replace this zap package with one only containing zap generate,
# and remove weird dependencies to X, asound and xvfb when this is done
COPY uic-resources/deb/zap_0.99.6_amd64.deb /tmp/zap_0.99.6_amd64.deb
RUN apt-get update \
  && apt-get install -y --no-install-recommends /tmp/zap_0.99.6_amd64.deb \
  && apt-get install -y --no-install-recommends libx11-xcb1 libxcb-dri3-0 libdrm2 libgbm1 libasound2 xvfb xauth \
  && rm -rf /var/lib/apt/lists/*

# Install Clang toolchain
RUN if [ "$CLANG" = true ] ; then \
  apt-get update \
  && apt-get install -y --no-install-recommends lsb-release wget software-properties-common \
  && curl -sL https://apt.llvm.org/llvm.sh --output /tmp/llvm.sh \
  && chmod +x /tmp/llvm.sh \
  && /tmp/llvm.sh 12 \
  ; fi


RUN getent group ${GROUP_ID} || addgroup --gid ${GROUP_ID} user
RUN useradd -m -l -u $USER_ID -g $GROUP_ID -G sudo -p $(openssl passwd -1 user) -o -s /bin/bash user
RUN echo "user   ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
USER user
