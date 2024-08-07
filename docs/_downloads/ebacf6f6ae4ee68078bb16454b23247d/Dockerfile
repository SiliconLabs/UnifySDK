ARG FROM="debian:bookworm"
FROM ${FROM}

SHELL ["/bin/bash", "-c"]

ARG USER_ID=1000
ARG GROUP_ID=1000
# Set to true to install clang toolchain
ARG CLANG=false
# Architectures
ARG ARCH="arm64"
ENV HEADLESS_HOST=true

ARG DEBIAN_FRONTEND=noninteractive

# configure additional arches in dpkg/apt
RUN for arch in ${ARCH} ; do \
  dpkg --add-architecture $arch ; \
  done

COPY host_dependencies.apt /tmp/host_dependencies.apt
RUN readarray -d ' ' -t HOST_DEPS < <(cat /tmp/host_dependencies.apt | awk -F ' ' '{print $1}') \
  && apt update && xargs apt install --no-install-recommends -y < <(echo ${HOST_DEPS[@]})

COPY target_dependencies.apt /tmp/target_dependencies.apt
RUN readarray -d ' ' -t TARGET_DEPS < <(cat /tmp/target_dependencies.apt | awk -F ' ' '{print $1}') \
  && apt update && xargs apt install --no-install-recommends -y \
  $(for dep in ${TARGET_DEPS[@]} ; do \
    for arch in ${ARCH} ; do echo $dep:$arch ; done ; \
  done) \
  $(for arch in ${ARCH} ; do echo crossbuild-essential-${arch} ; done)\
  && rm -rf /var/lib/apt/lists/*

# Install pip3 packages
# --break-system-packages : https://stackoverflow.com/a/75722775/7231626 needed since bookworm update
RUN pip3 install --break-system-packages gcovr==5.0 diff-cover==9.1.0 \
  pybars3==0.9.7 PyMeta3==0.5.1 xmltodict==0.12.0 \
  sphinx==5.1.0 breathe==4.34.0 myst-parser==0.18.0 linkify-it-py==2.0.0 sphinxcontrib-plantuml==0.24 sphinx-markdown-tables==0.0.17 sphinx-rtd-theme==1.0.0

COPY ./fetch_build_mosquitto.sh /tmp
RUN /tmp/fetch_build_mosquitto.sh "${ARCH}"

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

# Rust Version to install
ENV RUST_VERSION=1.71.0
# Rust and Cargo home directories
ENV RUSTUP_HOME=/opt/rustup-home
ENV CARGO_HOME=/opt/cargo-home
# Install Rust and Cargo
RUN curl https://sh.rustup.rs -sSf --output /tmp/sh.rustup.rs \
  && cd /tmp && chmod +x sh.rustup.rs \
  && if [[ $ARCH == *"armhf"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES armv7-unknown-linux-gnueabihf"; fi \
  && if [[ $ARCH == *"amd64"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES x86_64-unknown-linux-gnu"; fi \
  && if [[ $ARCH == *"arm64"* ]]; then export RUST_TRIPLES="$RUST_TRIPLES aarch64-unknown-linux-gnu"; fi \
  && ./sh.rustup.rs -y --profile minimal --target ${RUST_TRIPLES} --default-toolchain ${RUST_VERSION}\
  && rm /tmp/sh.rustup.rs \
  && /opt/cargo-home/bin/cargo install --version 0.1.13 --locked cargo2junit \
  && /opt/cargo-home/bin/cargo install --version 1.44.0 --locked cargo-deb \
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
RUN curl -L https://github.com/plantuml/plantuml/releases/download/v1.2022.0/plantuml-1.2022.0.jar --output /opt/plantuml.jar \
  && echo f1070c42b20e6a38015e52c10821a9db13bedca6b5d5bc6a6192fcab6e612691  /opt/plantuml.jar > /tmp/plantuml.jar.sha256 \
  && sha256sum -c /tmp/plantuml.jar.sha256
ENV PLANTUML_JAR_PATH=/opt/plantuml.jar

# Install ZAP and dependencies
RUN wget -O /tmp/zap.deb https://github.com/project-chip/zap/releases/download/v2023.05.04/zap-linux.deb \
  && echo a9ef0608035a106d4df4dd126490546cf8ba5d7cd13039a18d19cd3edd418a47 /tmp/zap.deb > /tmp/zap.deb.sha256 \
  && sha256sum -c /tmp/zap.deb.sha256\
  && apt update \
  && apt install -y --no-install-recommends /tmp/zap.deb \
  && rm -rf /var/lib/apt/lists/*

# Unpack SLC CLI
COPY uic-resources/linux/slc_cli_linux.zip /tmp/slc_cli_linux.zip
RUN unzip /tmp/slc_cli_linux.zip -d /opt \
  && chmod +x /opt/slc_cli/slc \
  && rm /tmp/slc_cli_linux.zip
ENV PATH="/opt/slc_cli:${PATH}"

# Install Clang toolchain
RUN if [ "$CLANG" = true ] ; then \
  curl -sL https://apt.llvm.org/llvm.sh --output /tmp/llvm.sh; \
  chmod +x /tmp/llvm.sh; \
  /tmp/llvm.sh 12; \
  fi

# Install yarn
RUN npm install yarn -g

RUN getent group ${GROUP_ID} || addgroup --gid ${GROUP_ID} user
RUN useradd -m -l -u $USER_ID -g $GROUP_ID -G sudo -p $(openssl passwd -1 user) -o -s /bin/bash user
RUN echo "user   ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
USER user
