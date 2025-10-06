set(CMAKE_CXX_STANDARD 23)

include_directories(../)

# 启用测试功能
enable_testing()

add_definitions(
    -DBRE_TEST
)

add_boost_test(test_buffer tests/test_buffer.cpp)

if(PLATFORM_LINUX)
    add_executable(benchmark
        tests/benchmark.cpp
    )
    
    target_include_directories(benchmark PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    target_link_libraries(benchmark PRIVATE
        Threads::Threads
    )
endif()


if(APPLE)
add_compile_definitions(_LIBCPP_ENABLE_EXPERIMENTAL)
target_link_libraries(base_test PRIVATE         
    "-framework AppKit"
    "-framework CoreVideo"
    "-framework IOKit"
    "-framework IOSurface"
    "-framework CoreMedia"
    "-framework CoreGraphics"
    "-framework AVFoundation"
    "-framework CoreFoundation"
    "-framework CoreServices"
    "-framework CoreAudio"
    "-framework SystemConfiguration"
)
endif()