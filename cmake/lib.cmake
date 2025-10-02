if(APPLE)
    include(cmake/macos.cmake)                  # macOS平台配置
elseif(UNIX)
    include(cmake/linux.cmake)                 # Linux平台配置
elseif(WIN32)
    include(cmake/windows.cmake)               # Windows平台配置
else()
    message(FATAL_ERROR "Unsupported platform. Please check your platform configuration file.")
endif()

# 定义库查找和链接的通用函数
function(link_all_libraries target)
    # FFmpeg
    link_ffmpeg(${target})
    
    # SDL3
    link_sdl3(${target})
    
    # Boost
    link_boost(${target})
    
    # OpenCV
    link_opencv(${target})
    
    # OpenSSL
    link_openssl(${target})
    
    # Qt
    link_qt(${target})
    
    # 其他库...
endfunction()

# FFmpeg 链接函数
function(link_ffmpeg target)
    # 确保FFMPEG_DIR_PATH已由平台文件设置
    if(NOT FFMPEG_DIR_PATH)
        message(FATAL_ERROR "FFMPEG_DIR_PATH未设置，请检查平台配置文件")
    endif()

    include_directories(${FFMPEG_DIR_PATH}/include)
    link_directories(${FFMPEG_DIR_PATH}/lib)

    set(FFMPEG_LIBS avdevice avcodec avfilter avutil swscale avformat swresample postproc)
    foreach(lib ${FFMPEG_LIBS})
        find_library(${lib}_LIB ${lib} PATHS ${FFMPEG_DIR_PATH}/lib NO_DEFAULT_PATH)
        if(NOT ${lib}_LIB)
            message(FATAL_ERROR "找不到FFmpeg库: ${lib}")
        endif()
        list(APPEND FFMPEG_LIBRARIES ${${lib}_LIB})
    endforeach()
    target_link_libraries(${target} PRIVATE ${FFMPEG_LIBRARIES})
endfunction()

# SDL3 链接函数
function(link_sdl3 target)
    find_package(SDL3 REQUIRED)
    target_link_libraries(${target} PRIVATE SDL3::SDL3)
endfunction()

# Boost 链接函数
function(link_boost target)
    if(WIN32)
        if(NOT BOOST_DIR_PATH)
            message(FATAL_ERROR "BOOST_DIR_PATH未设置，请检查平台配置文件")
        endif()

        include_directories(${BOOST_DIR_PATH}/include)
        file(GLOB BOOST_LIBS "${BOOST_DIR_PATH}/lib/libboost_*.a" "${BOOST_DIR_PATH}/lib/libboost_*.lib")
        target_link_libraries(${target} PRIVATE ${BOOST_LIBS})
    else()
        # 查找 Boost，指定常用的组件
        find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono json)
        if(NOT Boost_FOUND)
            message(FATAL_ERROR "Boost 未找到！请检查路径或安装情况")
        endif()
        
        # 只包含头文件库的目标
        target_include_directories(${target} PRIVATE ${Boost_INCLUDE_DIRS})
        
        # 链接需要编译的库
        target_link_libraries(${target} PRIVATE 
            Boost::system
            Boost::filesystem
            Boost::thread
            Boost::chrono
            Boost::json
        )
    endif()
endfunction()

# OpenCV 链接函数
function(link_opencv target)
    find_package(OpenCV REQUIRED)
    target_link_libraries(${target} PRIVATE ${OpenCV_LIBS})
endfunction()

# OpenSSL 链接函数
function(link_openssl target)
    if(NOT OPENSSL_DIR_PATH)
        message(FATAL_ERROR "OPENSSL_DIR_PATH未设置，请检查平台配置文件")
    endif()

    include_directories(${OPENSSL_DIR_PATH}/include)
    link_directories(${OPENSSL_DIR_PATH}/lib)

    set(OPENSSL_LIBS ssl crypto)
    foreach(lib ${OPENSSL_LIBS})
        find_library(${lib}_LIB ${lib} PATHS ${OPENSSL_DIR_PATH}/lib NO_DEFAULT_PATH)
        if(NOT ${lib}_LIB)
            message(FATAL_ERROR "找不到OpenSSL库: ${lib}")
        endif()
        list(APPEND OPENSSL_LIBRARIES ${${lib}_LIB})
    endforeach()
    target_link_libraries(${target} PRIVATE ${OPENSSL_LIBRARIES})
endfunction()
function(link_qt target Components)
    # 启用 AUTOMOC 和 AUTOUIC（处理 .ui 或 .qrc 文件）
    set_target_properties(${target} PROPERTIES
        AUTOMOC TRUE
        AUTOUIC TRUE
        AUTORCC TRUE
    )

    message(STATUS "link_qt(${target}) with components: ${Components}")

    if(NOT QTDIR)
        message(FATAL_ERROR "QTDIR未设置，请检查平台配置文件")
    endif()

    # 直接设置 Qt5_DIR 路径（比 CMAKE_PREFIX_PATH 更明确）
    set(Qt5_DIR "${QTDIR}/lib/cmake/Qt5" CACHE PATH "Qt5 directory" FORCE)

    # 解析传入的组件参数（将空格分隔的字符串转为列表）
    separate_arguments(Components_LIST UNIX_COMMAND "${Components}")

    # 确保包含 Core（Qt 必须的组件）
    list(APPEND Components_LIST Core)

    # 转换组件名称（如将 "GUI" 转为 "Gui"）
    foreach(component IN LISTS Components_LIST)
        string(TOUPPER ${component} component_upper)
        if(component_upper STREQUAL "GUI")
            list(APPEND Qt5_COMPONENTS Gui)
        else()
            list(APPEND Qt5_COMPONENTS ${component})
        endif()
    endforeach()

    # 查找 Qt5，并指定需要的组件
    find_package(Qt5 REQUIRED COMPONENTS ${Qt5_COMPONENTS})

    if(NOT Qt5_FOUND)
        message(FATAL_ERROR "Qt5 未找到！请检查路径或组件名称是否正确")
    endif()

    # 生成需要链接的 Qt 库列表
    set(Qt5_LIBRARIES "")
    foreach(component IN LISTS Qt5_COMPONENTS)
        list(APPEND Qt5_LIBRARIES Qt5::${component})
    endforeach()

    # 链接库（使用导入目标）
    target_link_libraries(${target} PRIVATE ${Qt5_LIBRARIES})

    # 输出调试信息
    message(STATUS "Qt5_DIR: ${Qt5_DIR}")
    message(STATUS "Qt5_COMPONENTS: ${Qt5_COMPONENTS}")
    message(STATUS "Qt5_LIBRARIES: ${Qt5_LIBRARIES}")
    message(STATUS "Qt5_VERSION: ${Qt5_VERSION}")
endfunction()

# spdlog 链接函数
set(SPDLOG_DIR_PATH "${CMAKE_SOURCE_DIR}/thirdLib/spdlog")
function(link_spdlog target)
    # 设置 spdlog 的包含目录
    include_directories(${SPDLOG_DIR_PATH}/include)
endfunction()


function(link_benchmark target)
    find_package(benchmark)
    if(NOT benchmark_FOUND)
        message(WARNING "benchmark 未找到！请检查路径或安装情况")
    endif()

    target_include_directories(${target} PRIVATE benchmark::benchmark)
    target_link_libraries(${target} PRIVATE ${benchmark_LIBRARIES} benchmark::benchmark)
endfunction()



# zlib
function(link_zlib target)
    # 先使用 CMake 的 ZLIB 包查找，没找到再使用自定义路径
    find_package(ZLIB QUIET)
    if(NOT ZLIB_FOUND)
        if(NOT ZLIB_DIR_PATH)
            message(WARNING "ZLIB_DIR_PATH未设置，请检查平台配置文件")
        endif()

        message(STATUS "ZLIB 未找到，使用自定义路径: ${ZLIB_DIR_PATH}")
        set(ZLIB_INCLUDE_DIRS ${ZLIB_DIR_PATH}/include)
        set(ZLIB_LIBRARIES ${ZLIB_DIR_PATH}/lib/libz.a)
        set(ZLIB_FOUND TRUE)
    endif()
    if(NOT ZLIB_FOUND)
        message(FATAL_ERROR "Zlib 未找到，请检查路径或安装情况")
    endif()
    target_include_directories(${target} PRIVATE ${ZLIB_INCLUDE_DIRS})

    target_link_libraries(${target} PRIVATE ${ZLIB_LIBRARIES})

endfunction(link_zlib target)




# ========== 仅头文件库 ==========
function(link_breutils target)
    if(NOT BREUTILS_DIR_PATH)
        message(FATAL_ERROR "BRE_DIR_PATH未设置，请检查平台配置文件")
    endif()

    target_include_directories(${target} PRIVATE ${BREUTILS_DIR_PATH})
endfunction()

function(link_boost_header target)
    message(STATUS "link_boost_header(${target}) with BOOST_DIR_PATH: ${BOOST_DIR_PATH}/include")
    target_include_directories(${target} PRIVATE ${BOOST_DIR_PATH}/include)
    
endfunction(link_boost_header target)

