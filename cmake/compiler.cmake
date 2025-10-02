# 引入常用模块
include(CheckLibraryExists)
include(CheckIncludeFile)
include(CheckLanguage)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckStructHasMember)
include(CMakeDependentOption)
include(CMakeParseArguments)
include(CMakePushCheckState)
include(GNUInstallDirs)

# -----------------------------
# 平台检测与统一变量设置
# -----------------------------

# 统一处理器架构命名
if(MSVC_CXX_ARCHITECTURE_ID)
  string(TOLOWER ${MSVC_CXX_ARCHITECTURE_ID} LOWERCASE_CMAKE_SYSTEM_PROCESSOR)
else()
  string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} LOWERCASE_CMAKE_SYSTEM_PROCESSOR)
endif()

set(ARCH_SIMD_FLAGS "")
set(ARCH_SIMD_DEFINES "")

# -----------------------------
# 1. Windows + MSVC
# -----------------------------
if(WIN32 AND MSVC)
  # UTF-8 源文件编码
  add_compile_options("/utf-8")

  # MSVC 编译选项
  add_compile_options(
    /MP                          # 多进程编译
    /W3                          # 警告级别 3
    /wd4127                      # 禁用 C4127: 条件为常量
    /wd4201                      # 禁用 C4201: 名称未标准化（匿名结构体）
    /wd4251                      # 禁用 C4251: DLL 接口类需要导出
    /wd4456 /wd4457 /wd4458 /wd4459  # 禁用变量遮蔽警告
    /wd4595                      # 禁用 C4595: 内联函数无链接
    "$<$<CONFIG:DEBUG>:/DDEBUG=1;/D_DEBUG=1>"
    "$<$<CONFIG:RELWITHDEBINFO>:/Ob2>"
    /DUNICODE
    /D_UNICODE
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS
  )

  # 链接器选项
  add_link_options(
    "LINKER:/OPT:REF"
    "$<$<NOT:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>>:LINKER\:/SAFESEH\:NO>"
    "$<$<CONFIG:DEBUG>:LINKER\:/INCREMENTAL\:NO>"
    "$<$<CONFIG:RELWITHDEBINFO>:LINKER\:/INCREMENTAL\:NO;/OPT:ICF>"
  )

  # x86/x64 架构 SIMD
  if(LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "(i[3-6]86|x86|x64|x86_64|amd64)")
    set(ARCH_SIMD_FLAGS ${ARCH_SIMD_FLAGS} -mmmx -msse -msse2)
  endif()

# -----------------------------
# 2. Linux (GNU/Clang)
# -----------------------------
elseif(UNIX AND NOT APPLE)
  # 启用 ccache（如果可用）
  find_program(CCACHE_PROGRAM "ccache")
  set(CCACHE_SUPPORT ON CACHE BOOL "Enable ccache support")
  mark_as_advanced(CCACHE_PROGRAM)
  if(CCACHE_PROGRAM AND CCACHE_SUPPORT)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_OBJC_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_OBJCXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  endif()

  # 常用编译选项
  add_compile_options(
    -Wextra
    -Wvla
    -Wno-unused-function
    -Wno-missing-field-initializers
    -fno-strict-aliasing
    "$<$<COMPILE_LANGUAGE:C>:-Werror-implicit-function-declaration;-Wno-missing-braces>"
    "$<$<BOOL:${USE_LIBCXX}>:-stdlib=libc++>"
    "$<$<CONFIG:DEBUG>:-DDEBUG=1;-D_DEBUG=1>"
    "$<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-fcolor-diagnostics>"
    "$<$<COMPILE_LANG_AND_ID:C,AppleClang,Clang>:-fcolor-diagnostics>"
  )

  # Linux 特有：MinGW 以外的架构处理
  if(NOT MINGW)
    if(LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "(i[3-6]86|x86|x64|x86_64|amd64)")
      set(ARCH_SIMD_FLAGS ${ARCH_SIMD_FLAGS} -mmmx -msse -msse2)
    elseif(LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64(le)?")
      list(APPEND ARCH_SIMD_DEFINES -DNO_WARN_X86_INTRINSICS)
      list(APPEND ARCH_SIMD_FLAGS -mvsx)
    else()
      # 其他架构（如 ARM）尝试 OpenMP SIMD
      # check_c_compiler_flag("-fopenmp-simd" C_COMPILER_SUPPORTS_OPENMP_SIMD)
      # check_cxx_compiler_flag("-fopenmp-simd" CXX_COMPILER_SUPPORTS_OPENMP_SIMD)
      set(ARCH_SIMD_FLAGS
        ${ARCH_SIMD_FLAGS}
        -DSIMDE_ENABLE_OPENMP
        "$<$<AND:$<COMPILE_LANGUAGE:C>,$<BOOL:C_COMPILER_SUPPORTS_OPENMP_SIMD>>:-fopenmp-simd>"
        "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:CXX_COMPILER_SUPPORTS_OPENMP_SIMD>>:-fopenmp-simd>"
      )
    endif()
  endif()

# -----------------------------
# 3. macOS (AppleClang)
# -----------------------------
elseif(APPLE)
  # 启用 ccache
  find_program(CCACHE_PROGRAM "ccache")
  set(CCACHE_SUPPORT ON CACHE BOOL "Enable ccache support")
  mark_as_advanced(CCACHE_PROGRAM)
  if(CCACHE_PROGRAM AND CCACHE_SUPPORT)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_OBJC_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_OBJCXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  endif()

  add_compile_definitions(
    _LIBCPP_ENABLE_EXPERIMENTAL
  )

  # macOS 编译选项（类似 Linux）
  add_compile_options(
    -Wall
    -Wextra
    -Wvla
    -Wno-unused-function
    -Wno-missing-field-initializers
    -Wno-unused-private-field
    -fno-strict-aliasing
    "$<$<COMPILE_LANGUAGE:C>:-Werror-implicit-function-declaration;-Wno-missing-braces>"
    "$<$<BOOL:${USE_LIBCXX}>:-stdlib=libc++>"
    "$<$<CONFIG:DEBUG>:-D_DEBUG=1>"
    -fcolor-diagnostics  # AppleClang 默认支持
  )

  if(LOWERCASE_CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|amd64)")
    set(ARCH_SIMD_FLAGS ${ARCH_SIMD_FLAGS} -mmmx -msse -msse2)
  elseif(LOWERCASE_CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    # Apple Silicon：使用 NEON 或 OpenMP SIMD
    check_c_compiler_flag("-fopenmp-simd" C_COMPILER_SUPPORTS_OPENMP_SIMD)
    check_cxx_compiler_flag("-fopenmp-simd" CXX_COMPILER_SUPPORTS_OPENMP_SIMD)
    set(ARCH_SIMD_FLAGS
      ${ARCH_SIMD_FLAGS}
      -DSIMDE_ENABLE_OPENMP
      "$<$<AND:$<COMPILE_LANGUAGE:C>,$<BOOL:C_COMPILER_SUPPORTS_OPENMP_SIMD>>:-fopenmp-simd>"
      "$<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:CXX_COMPILER_SUPPORTS_OPENMP_SIMD>>:-fopenmp-simd>"
    )
  endif()

endif()


# -----------------------------
# 应用 SIMD 标志（如果已设置）
# -----------------------------
if(ARCH_SIMD_FLAGS)
  add_compile_options(${ARCH_SIMD_FLAGS})
endif()
if(ARCH_SIMD_DEFINES)
  add_compile_definitions(${ARCH_SIMD_DEFINES})
endif()