# 工具名称（替换为实际工具名，如 tool_a）
set(TOOL_NAME tool_template)

# 源文件和头文件
file(GLOB TOOL_SOURCES "src/*.cpp" "src/*.c")
file(GLOB TOOL_HEADERS "include/*.h" "src/*.hpp")

# 创建可执行文件
add_executable(${TOOL_NAME} ${TOOL_SOURCES} ${TOOL_HEADERS})


# 链接工具专用库（如 Boost、Qt 等）
target_link_libraries(${TOOL_NAME} PRIVATE Boost::boost)

# 设置编译选项（如优化、调试）
target_compile_options(${TOOL_NAME} PRIVATE -Wall -Wextra)

# 设置包含路径（如公共头文件）
target_include_directories(${TOOL_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    # 其他路径
)