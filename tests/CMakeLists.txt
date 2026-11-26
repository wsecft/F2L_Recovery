add_executable(f2l_core_tests test_core.cpp)
target_link_libraries(f2l_core_tests PRIVATE f2l_core)
add_test(NAME f2l_core_tests COMMAND f2l_core_tests)

add_executable(f2l_solver_tests test_solver.cpp)
target_link_libraries(f2l_solver_tests PRIVATE f2l_core)
add_test(NAME f2l_solver_tests COMMAND f2l_solver_tests)

add_executable(f2l_solver_bench bench_solver.cpp)
target_link_libraries(f2l_solver_bench PRIVATE f2l_core)
