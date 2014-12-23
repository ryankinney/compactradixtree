cmake_minimum_required(VERSION 2.8)
find_package(GTest REQUIRED)
add_executable(Test UniqueNumberCounter.cpp Test.cpp)
target_link_libraries(Test ${GTEST_BOTH_LIBRARIES} pthread)
