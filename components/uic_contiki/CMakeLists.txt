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

add_library(uic_contiki_platform SHARED platform/${COMPATIBLE_PLATFORM}/clock.c)
target_include_directories(uic_contiki_platform
                           PUBLIC core platform/${COMPATIBLE_PLATFORM})

set(CONTIKI_INC core core/dev core/lib core/sys cpu/native
                platform/${COMPATIBLE_PLATFORM})
add_library(uic_contiki SHARED ${CONTIKI_SRC})

target_include_directories(uic_contiki PUBLIC ${CONTIKI_INC})
target_compile_definitions(uic_contiki PUBLIC)

if(NOT APPLE)
  target_link_libraries(uic_contiki PRIVATE rt uic_contiki_platform uic_log)
else()
  target_link_libraries(uic_contiki PRIVATE uic_contiki_platform uic_log)
endif()

add_subdirectory(test)
