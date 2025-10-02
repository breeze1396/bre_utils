# Helper function to add a Boost.Test target
function(add_boost_test TARGET_NAME SOURCE_FILE)
    message(STATUS "Adding Boost test target: ${TARGET_NAME}")
    add_executable(${TARGET_NAME} ${SOURCE_FILE})

    link_boost(${TARGET_NAME}  
        REQUIRED
        COMPONENTS unit_test_framework
    )

    # Add the test to CTest
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})

    # Set the working directory for the test
    set_tests_properties(${TARGET_NAME} PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Enable C++ standard if needed
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )

endfunction()

# 拷贝Json测试数据到构建目录
add_custom_target(copy_test_data ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/breUtils/json/test
    ${CMAKE_BINARY_DIR}/test
)
message(STATUS "Copying test data to ${CMAKE_BINARY_DIR}/test")
