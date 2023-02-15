set(CONTIKI_SRC
    core/lib/assert.c
    core/lib/crc16.c
    core/lib/list.c
    core/lib/memb.c
    core/lib/random.c
    core/lib/ringbuf.c
    core/sys/autostart.c
    core/sys/ctimer.c
    core/sys/etimer.c
    core/sys/process.c
    core/sys/procinit.c
    core/sys/timer.c)

set(CONTIKI_INC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/core/dev>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/core/lib>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/core/sys>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/cpu/native>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/platform/${COMPATIBLE_PLATFORM}>
)

if(ZWAVE_BUILD_SYSTEM EQUAL 1)
  add_library(uic_contiki ${CONTIKI_SRC})
  target_include_directories(uic_contiki PUBLIC ${CONTIKI_INC} ${ZW_ROOT}/ZWave)
else()
  add_library(uic_contiki OBJECT ${CONTIKI_SRC} src/rust_process.c)
  target_include_directories(uic_contiki PUBLIC ${CONTIKI_INC})
endif()

if(ZWAVE_BUILD_SYSTEM EQUAL 1)
  add_library(uic_contiki_platform platform/${COMPATIBLE_PLATFORM}/clock.c)
else()
  add_library(uic_contiki_platform OBJECT
              platform/${COMPATIBLE_PLATFORM}/clock.c)

  if(ZWAVE_BUILD_SYSTEM EQUAL 1)
    set(PRIVATE_LIB_LIST "")
  else()
    set(PRIVATE_LIB_LIST "rt")
  endif()
  if(NOT APPLE)
    target_link_libraries(uic_contiki_platform PRIVATE ${PRIVATE_LIB_LIST})
  endif()
endif()

target_include_directories(
  uic_contiki_platform
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/platform/${COMPATIBLE_PLATFORM}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/core/sys>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/cpu/native>)

target_link_libraries(uic_contiki PRIVATE uic_contiki_platform uic_log)

# Install headers under uic/include, uic/include/lib uic/include/sys
install(
  FILES platform/posix/contiki-conf.h include/request_poller.h
  DESTINATION include/uic
  COMPONENT libuic-dev)

install(
  DIRECTORY include
  DESTINATION include/uic
  COMPONENT libuic-dev)

file(GLOB_RECURSE PUBLIC_CONTIKI1 core/lib/*.h)
install(
  FILES ${PUBLIC_CONTIKI1}
  DESTINATION include/uic/lib
  COMPONENT libuic-dev)

file(GLOB_RECURSE PUBLIC_CONTIKI2 core/sys/*.h)
install(
  FILES ${PUBLIC_CONTIKI2}
  DESTINATION include/uic/sys
  COMPONENT libuic-dev)

if(BUILD_TESTING)
  add_library(uic_contiki_stub $<TARGET_PROPERTY:uic_contiki,SOURCES>
                               test/contiki_test_helper.c)
  target_interface_libraries(uic_contiki_stub)

  target_include_directories(
    uic_contiki_stub PUBLIC test
                            $<TARGET_PROPERTY:uic_contiki,INCLUDE_DIRECTORIES>)

  add_subdirectory(test)
endif()
