option(BUILD_TESTS "Build the unit tests" On)
option(BUILD_TOOLS "Build the unit tools" OFF)

if(LINUX)
    option(BUILD_BENCHMARK "Build the unit benchmark" ON)
endif()




message(STATUS "BUILD_TESTS: ${BUILD_TESTS}")
message(STATUS "BUILD_TOOLS: ${BUILD_TOOLS}")
message(STATUS "BUILD_BENCHMARK: ${BUILD_BENCHMARK}")

# 添加子目录
if(BUILD_TOOLS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tools)
endif()

# 启用测试
if(BUILD_TESTS)
# 添加宏 BRE_TEST
    message("BRE_TEST ON")
    add_definitions(-DBRE_TEST)
    enable_testing()
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif()

if(BUILD_BENCHMARK)
    message("BUILD_BENCHMARK ON")
    add_subdirectory(${CMAKE_SOURCE_DIR}/benchmark)
endif()