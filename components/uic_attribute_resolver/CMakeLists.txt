# Attribute resolver
add_library(uic_attribute_resolver SHARED
    src/attribute_resolver.cpp
    src/attribute_resolver_rule.cpp
)

target_include_directories(uic_attribute_resolver PUBLIC include PRIVATE src)
target_link_libraries(uic_attribute_resolver PUBLIC
  uic_attribute_store
  uic_multi_invoke
  uic_contiki
  uic_contiki_platform
)

# Attribute resolver rule mock
add_mock(uic_attribute_resolver_rule_mock
  include/attribute_resolver_rule.h
  src/attribute_resolver_rule_internal.h
)
target_include_directories(uic_attribute_resolver_rule_mock PUBLIC include)
target_interface_libraries(uic_attribute_resolver_rule_mock uic_contiki uic_attribute_store )

# Attribute resolver mock
add_mock(uic_attribute_resolver_mock
  include/attribute_resolver.h
)
target_include_directories(uic_attribute_resolver_mock PUBLIC include)
target_interface_libraries(uic_attribute_resolver_mock uic_attribute_store uic_contiki uic_log)

add_subdirectory(test)
