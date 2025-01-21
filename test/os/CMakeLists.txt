add_executable(test_timer test_timer.cc)
target_link_libraries(test_timer libcore::event GTest::gtest GTest::gtest_main)
# add test_timer to the list of tests
add_test(test_timer test_timer)
