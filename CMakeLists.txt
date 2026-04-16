cmake_minimum_required(VERSION 3.15)
project(DynaChartView VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# ==================== 静态编译配置 ====================
# 启用静态链接
set(BUILD_SHARED_LIBS OFF)
if(MSVC)
    option (OpenCV_STATIC "Use OpenCV as a static library" OFF)
endif()

# 设置版本号
set(VERSION_H_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in")
set(VERSION_H_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h")
configure_file(${VERSION_H_INPUT} ${VERSION_H_OUTPUT} @ONLY)

# /MT manual replacing doesn't work. Use this for static compiling.
if(MSVC AND OpenCV_STATIC)
   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ====================================================
if(WIN32)
   add_definitions(-DUNICODE -D_UNICODE)
   add_compile_definitions(_UTF8_SOURCE)
endif()

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加子项目
add_subdirectory(ChartStore)
add_subdirectory(ImageGenerator)

# 主程序
add_executable(DynaChartView src/main.cpp "include/version.h")
target_link_libraries(DynaChartView PRIVATE ImageGenerator ChartStore)
target_include_directories(DynaChartView PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ChartStore/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ImageGenerator/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# UTF-8 编码设置
if (MSVC)
    target_compile_options(DynaChartView PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
    target_compile_options(ChartStore PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
    target_compile_options(ImageGenerator PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
    message(STATUS "UTF-8: MSVC source/execution charset enabled for all targets")
else()
    target_compile_options(DynaChartView PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
    target_compile_options(ChartStore PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
    target_compile_options(ImageGenerator PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
    message(STATUS "UTF-8: GCC/Clang input/exec charset enabled for all targets")
endif()

# 打印配置信息
message(STATUS "========================================")
message(STATUS "DynaChartView CMake Configuration")
message(STATUS "========================================")
message(STATUS "VERSION:             ${PROJECT_VERSION}")
message(STATUS "BUILD_SHARED_LIBS:   ${BUILD_SHARED_LIBS}")
if(MSVC)
# Windows exclusive settings
message(STATUS "OpenCV_STATIC:       ${OpenCV_STATIC}")
endif()
message(STATUS "CMAKE_CXX_FLAGS:     ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CURRENT PLATFORM:    ${CMAKE_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION}-${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "========================================")