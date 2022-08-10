FROM debian:buster

# Input arguments
ARG DEBIAN_PACKAGES=deb-packages.zip

# Copying over artfiacts
WORKDIR /unify
COPY ${DEBIAN_PACKAGES} /tmp/deb-packages.zip
COPY ./run_zpc.sh .

RUN chmod +x run_zpc.sh

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update \
 && apt install --no-install-recommends -y socat unzip \
 && unzip -j /tmp/deb-packages.zip -d /unify/deb-packages \
 && echo "deb [trusted=yes] file:///unify/deb-packages ./" > /tmp/uic-packages.list && mv /tmp/uic-packages.list /etc/apt/sources.list.d/ \
 && apt update \
 && apt install --no-install-recommends -y uic-dev-gui \
                                           uic-gms \
                                           uic-image-provider \
                                           uic-nal \
                                           uic-upti-cap \
                                           uic-upvl \
                                           uic-zpc\
 && rm -rf /var/lib/apt/lists/* \
 && rm -rf /unify/deb-packages \
 && rm -rf /tmp/*
