# this one is important
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm64)
set(CONFIGURE_HOST arm64-linux)
# this one not so much
set(CMAKE_SYSTEM_VERSION 1)

set(triple aarch64-linux-gnu)

# specify the cross compiler If CMAKE_CLANG is set, use clang, otherwise use gcc
if(CMAKE_CLANG)
  set(CMAKE_C_COMPILER clang)
  set(CMAKE_C_COMPILER_TARGET ${triple})
  set(CMAKE_CXX_COMPILER clang++)
  set(CMAKE_CXX_COMPILER_TARGET ${triple})
else()
  set(CMAKE_C_COMPILER ${triple}-gcc)
  set(CMAKE_CXX_COMPILER ${triple}-g++)
endif()

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE arm64)
set(CPACK_FILE_NAME_EXTRA bullseye)

# Configure pkg-config to only find target libs
set(PKG_CONFIG_EXECUTABLE
    "/usr/bin/${triple}-pkg-config"
    CACHE PATH "pkg-config")
set(ENV{PKG_CONFIG_PATH} "/usr/lib/${triple}/pkgconfig")
