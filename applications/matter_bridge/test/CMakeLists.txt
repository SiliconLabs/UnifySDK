include(GoogleTest)

add_executable(hello_gmock_test hello_gmock.cpp)
target_link_libraries(hello_gmock_test gtest gtest_main gmock pthread)
gtest_discover_tests(hello_gmock_test)
