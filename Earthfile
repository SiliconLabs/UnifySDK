VERSION 0.6
IMPORT ./earthly_builds/base AS base

ARG BUILD_TYPE="Debug"

build-all-applications:
    BUILD +build-zpc
    BUILD +build-zigpc
    BUILD +build-dev-gui
    BUILD +build-upvl
    BUILD +build-gms
    BUILD +build-image-provider
    BUILD +build-aoxpc
    BUILD +build-positioning
    BUILD +build-upti-writer
    BUILD +build-cpcd
    # BUILD +build-zigbeed
    BUILD +build-matter-bridge
    BUILD +build-epc


base-pull:
    ARG FROM_IMAGE
    FROM $FROM_IMAGE

base-dockerfile:
    ARG ARCH
    FROM DOCKERFILE docker --build-arg ARCH=$ARCH

unify-source-files:
    ARG FROM_IMAGE="null"
    ARG ARCH
    FROM busybox

    # Check arch value is set
    IF [ -z "$ARCH" ]
        RUN echo "ARCH is not set" && exit 1
    END

    IF [ "$FROM_IMAGE" = "null" ]
        FROM +base-dockerfile --ARCH=$ARCH
    ELSE
        FROM +base-pull --FROM_IMAGE=$FROM_IMAGE
    END

    WORKDIR /unify

    # Git folder
    COPY .git .git

    # CMake configuraiton files
    COPY cmake cmake
    RUN sed -i 's/ ON)/ OFF)/g' ./cmake/include/build_permutations.cmake
    COPY CMakeLists.txt CMakeLists.txt
    COPY scripts scripts

    # Source code
    COPY components components
    COPY applications applications
    COPY include include

    # Rust applications configuration
    COPY Cargo.toml Cargo.toml
    COPY Cargo.lock Cargo.lock
    COPY .cargo .cargo

unify-lib:
    ARG ARCH
    FROM +unify-source-files
    RUN mkdir build
    IF [ "$ARCH" = "armhf" ]
        RUN cmake -GNinja -B./build -DFETCH_GSDK=ON -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf_debian.cmake -DBUILD_TYPE=$BUILD_TYPE .
    ELSE
        RUN cmake -GNinja -B./build -DFETCH_GSDK=ON -DBUILD_TYPE=$BUILD_TYPE .
    END
    RUN ninja -C build

build-zpc:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_ZPC=ON -B./build .
    RUN ninja -C build

build-zigpc:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_ZIGPC=ON -B./build .
    RUN ninja -C build

build-dev-gui:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_DEV_GUI=ON -B./build .
    RUN ninja -C build

build-upvl:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_UPVL=ON -B./build .
    RUN ninja -C build

build-gms:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_GMS=ON -B./build .
    RUN ninja -C build

build-image-provider:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_IMAGE_PROVIDER=ON -B./build .
    RUN ninja -C build

build-aoxpc:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_AOXPC=ON -B./build .
    RUN ninja -C build

build-positioning:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_POSITIONING=ON -B./build .
    RUN ninja -C build

build-upti-writer:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_UPTI_WRITER=ON -B./build .
    RUN ninja -C build

build-cpcd:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_CPCD=ON -B./build .
    RUN ninja -C build

build-zigbeed:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_ZIGBEED=ON -B./build .
    RUN ninja -C build

build-matter-bridge:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_MATTER_BRIDGE=ON -B./build .
    RUN ninja -C build

build-epc:
    FROM +unify-lib
    RUN cmake -GNinja -DBUILD_EPC=ON -B./build .
    RUN ninja -C build


