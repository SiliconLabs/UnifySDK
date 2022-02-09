add_definitions( -DUNIT_TEST )

add_unity_test(NAME test_mock_framework FILES test_mock_framework.c example_test_mock.c LIBRARIES mock)
