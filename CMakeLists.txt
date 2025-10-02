# using breUtils
# just include(your/path/to/breUtils)

cmake_minimum_required(VERSION 3.20)
project(YourProgramName CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/compiler.cmake)
include(cmake/lib.cmake)                      # 引入库管理逻辑
include(cmake/test.cmake)                     # 引入测试逻辑



include(cmake/option.cmake)