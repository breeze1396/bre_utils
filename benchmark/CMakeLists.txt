project(benchmark_base)

file(GLOB BENCHMARK_SRC "*.cpp" "*.h")

add_executable(${PROJECT_NAME} ${BENCHMARK_SRC})

# -o2 optimization
target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
    $<$<CONFIG:RELEASE>:NDEBUG>
)
target_compile_options(
    ${PROJECT_NAME} PRIVATE
    $<$<CONFIG:RELEASE>:-O2>
)

link_benchmark(${PROJECT_NAME})


