target_add_unittest(uic_contiki 
SOURCES test_contiki.c  "$<TARGET_OBJECTS:uic_contiki_stub>"
EXCLUDE clock.c)