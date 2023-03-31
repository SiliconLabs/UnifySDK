FROM joseluisq/rust-linux-darwin-builder:1.65.0

ARG DEBIAN_FRONTEND=noninteractive

RUN apt update \
 && apt install --no-install-recommends -y g++-mingw-w64-x86-64 python3 python3-requests unzip zip \
 && rm -rf /var/lib/apt/lists/*

ARG USERNAME=user
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

RUN chown -R $USER_UID:$USER_GID /root

USER $USERNAME

RUN ln -s /root/.cargo ~ && ln -s /root/.rustup ~

ENV PATH="$PATH:/home/$USERNAME/.cargo/bin"

RUN rustup target add x86_64-pc-windows-gnu && \
    rustup toolchain install stable-x86_64-pc-windows-gnu
