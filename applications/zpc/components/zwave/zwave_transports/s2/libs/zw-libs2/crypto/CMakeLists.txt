cross_system_dir("curve25519" "" CURVE_INCLUDE_DIR)
include_directories(${CURVE_INCLUDE_DIR})

add_definitions(-DCCM_USE_PREDEFINED_VALUES)

set(CURVE_SRC curve25519/generic/smult.c curve25519/generic/base.c
              curve25519/generic/bigint.c)

set_source_files_properties(kderiv/kderiv.c PROPERTIES COMPILE_FLAGS
                                                       -Wno-unused-parameter)
set(CRYPTO_SRC
    ccm/ccm.c
    aes-cmac/aes_cmac.c
    ctr_drbg/ctr_drbg.c
    nextnonce/nextnonce.c
    kderiv/kderiv.c
    ${CURVE_SRC}
    ${SCRAMBLER_SRC})

if(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "SDK")
  add_library(aes aes/aes.c)
  add_library(s2crypto ${CRYPTO_SRC})
  target_compile_definitions(s2crypto PUBLIC "DllExport=extern")
  if(WIN32)
    add_library(s2cryptoShared SHARED ${CRYPTO_SRC} aes/aes.c)
  endif()
  # zwave-protocol support
else(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "SDK")
  add_library(aes OBJECT aes/aes.c)
  add_library(s2crypto OBJECT ${CRYPTO_SRC})
endif(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "SDK")

target_include_directories(s2crypto PUBLIC "${CURVE_INCLUDE_DIR}" ../include)
