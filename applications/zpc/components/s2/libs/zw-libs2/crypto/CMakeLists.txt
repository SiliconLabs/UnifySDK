
CROSS_SYSTEM_DIR("curve25519" "" CURVE_INCLUDE_DIR)
include_directories( ${CURVE_INCLUDE_DIR} )

add_definitions(-DCCM_USE_PREDEFINED_VALUES)

if(${CMAKE_SYSTEM_NAME} MATCHES "C51")
  SET(CURVE_SRC curve25519/c51/curve25519.c curve25519/c51/fe25519.c curve25519/c51/bigint.a51 )
  SET(SCRAMBLER_SRC key_scrambler/ZW_s2_key_scrambler.c)
else()
  SET(CURVE_SRC curve25519/generic/smult.c curve25519/generic/base.c curve25519/generic/bigint.c)
endif()

SET(CRYPTO_SRC  ccm/ccm.c aes-cmac/aes_cmac.c ctr_drbg/ctr_drbg.c 
  nextnonce/nextnonce.c kderiv/kderiv.c ${CURVE_SRC} ${SCRAMBLER_SRC})

if(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "SDK")
  add_library(aes aes/aes.c)
  add_library(s2crypto ${CRYPTO_SRC})
  target_compile_definitions(s2crypto PUBLIC "DllExport=extern")
  if(WIN32)
    add_library(s2cryptoShared SHARED ${CRYPTO_SRC} aes/aes.c)
  endif()
# zwave-protocol support
else(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "SDK")
  if(NOT (${CMAKE_SYSTEM_NAME} MATCHES "C51"))
    add_library(aes OBJECT aes/aes.c)
    add_library(s2crypto OBJECT ${CRYPTO_SRC})
  endif(NOT (${CMAKE_SYSTEM_NAME} MATCHES "C51"))
endif(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "SDK")

target_include_directories(s2crypto PUBLIC "${CURVE_INCLUDE_DIR}" ../include )
