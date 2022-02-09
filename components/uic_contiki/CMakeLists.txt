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
    core
    core/dev
    core/lib
    core/sys
    cpu/native
    include
    platform/${COMPATIBLE_PLATFORM})

if(ZWAVE_BUILD_SYSTEM EQUAL 1)
  add_library(uic_contiki ${CONTIKI_SRC})
else()
  add_library(uic_contiki SHARED ${CONTIKI_SRC} src/rust_process.c)
endif()

target_include_directories(uic_contiki PUBLIC ${CONTIKI_INC})

if(ZWAVE_BUILD_SYSTEM EQUAL 1)
  add_library(uic_contiki_platform platform/${COMPATIBLE_PLATFORM}/clock.c)
else()
  add_library(uic_contiki_platform SHARED
              platform/${COMPATIBLE_PLATFORM}/clock.c)

  if(ZWAVE_BUILD_SYSTEM EQUAL 1)
    set(PRIVATE_LIB_LIST "")
  else()
    set(PRIVATE_LIB_LIST "rt")
  endif()
  if(NOT APPLE)
    target_link_libraries(uic_contiki_platform PRIVATE ${PRIVATE_LIB_LIST} )
  endif()
endif()

target_include_directories(
  uic_contiki_platform PUBLIC core platform/${COMPATIBLE_PLATFORM} core/sys
                              cpu/native)

target_link_libraries(uic_contiki PRIVATE uic_contiki_platform uic_log)

add_subdirectory(test)
