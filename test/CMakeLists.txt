set(CMAKE_CXX_STANDARD 23)

include_directories(../)



# 启用测试功能
enable_testing()

add_definitions(
    -DBRE_TEST
)

# 添加测试子目录
# add_subdirectory(test_json)
# add_subdirectory(test_signal)
# add_executable(base_test base_test_main.cpp)

# link_boost_header(base_test)
# link_zlib(base_test)

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