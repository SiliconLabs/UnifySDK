
add_definitions( -DUNIT_TEST )

#include_directories( ../crypto/aes )
#add_executable( test_ccm test_ccm.c ../crypto/ccm/ccm.c ../crypto/aes/aes.c )
#add_test( test_ccm test_ccm )

# Add test for protocol
add_unity_test(NAME test_protocol FILES test_protocol.c LIBRARIES s2_controller s2crypto aes)

# Add test for AES-CMAC
add_unity_test(NAME test_aes_cmac FILES test_aes_cmac.c LIBRARIES s2crypto aes)

# Add test for Curve25519
include_directories(.)
add_unity_test(NAME test_curve25519 FILES wc_util.c test_curve25519.c LIBRARIES s2crypto aes)

# Add test for CCM
add_unity_test(NAME test_ccm FILES test_ccm.c ../crypto/ccm/ccm.c ../crypto/aes/aes.c)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "C51")
 	add_definitions( -DNEW_TEST_T2 )
 	add_executable(new_test_t2
          new_test_t2.c
          clock_time.c
          ${CMAKE_CURRENT_SOURCE_DIR}/../transport_service/transport_service2.c
          ${CMAKE_CURRENT_SOURCE_DIR}/../transport_service/transport2_fsm.c)
 	include_directories(
          ${CMAKE_CURRENT_SOURCE_DIR}/../transport_service/
          ${CMAKE_CURRENT_SOURCE_DIR}/../include/ )
  if (${CMAKE_PROJECT_NAME} MATCHES "zipgateway")
    target_link_libraries(new_test_t2 zipgateway-lib)
  endif()
	add_test(test_transport_service2 new_test_t2)
endif()

add_definitions( -DRANDLEN=64 )
add_unity_test(NAME test_ctr_dbrg FILES test_ctr_dbrg.c ../crypto/ctr_drbg/ctr_drbg.c ../crypto/aes/aes.c)

add_unity_test(NAME test_kderiv FILES test_kderiv.c ../crypto/kderiv/kderiv.c ../crypto/aes-cmac/aes_cmac.c ../crypto/aes/aes.c)

# Disabling unit test for now. Not sure if it works on C51.
#add_definitions ( -DNO_MEM_FUNCTIONS )
#add_definitions ( -DLINUX_TEST )
#add_executable( test_key_scramble test_key_scramble.c ../crypto/key_scramble/ZW_s2_key_scrambler.c ../crypto/aes/aes.c)
#target_link_libraries(test_key_scramble) 
