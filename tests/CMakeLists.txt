add_executable(f2l_core_tests test_core.cpp)
target_link_libraries(f2l_core_tests PRIVATE f2l_core)
add_test(NAME f2l_core_tests COMMAND f2l_core_tests)
