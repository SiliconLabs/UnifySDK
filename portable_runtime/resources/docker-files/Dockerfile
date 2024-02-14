FROM debian:bullseye

# Input arguments
ARG DEBIAN_PACKAGES=deb-packages.zip

# Copying over artfiacts
WORKDIR /unify
COPY ${DEBIAN_PACKAGES} .
COPY ./run_zpc.sh .
COPY ./run_zigpc.sh .
COPY ./run_aoxpc.sh .
COPY ./run_multiprotocol.sh .
COPY ./run_matter_bridge.sh .
COPY ./uic-mb_1.1.0_amd64.deb .
COPY ./chip-tool .

RUN chmod +x run_zigpc.sh run_zpc.sh run_aoxpc.sh run_multiprotocol.sh run_matter_bridge.sh chip-tool

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update \
 && apt install --no-install-recommends -y socat unzip iproute2 \
 && unzip -j *.zip -d deb-packages \
 && echo "deb [trusted=yes] file:///unify/deb-packages ./" > /etc/apt/sources.list.d/uic-packages.list \
 && apt update \
 && apt install -y libreadline-dev libglib2.0-dev \
 && apt install --no-install-recommends -y uic-dev-gui \
                                           uic-gms \
                                           uic-image-provider \
                                           uic-nal \
                                           uic-positioning \
                                           uic-upti-cap \
                                           uic-upvl \
                                           uic-zpc\
                                           uic-zigpc\
                                           uic-aoxpc\
                                           uic-cpcd\
                                           uic-zigbeed\
 && apt install -y ./uic-mb_1.1.0_amd64.deb \
 && rm -rf /var/lib/apt/lists/* \
 && rm -rf /unify/deb-packages
