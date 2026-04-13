cmake_minimum_required(VERSION 3.15)
project(DynaChartView VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# ==================== 静态编译配置 ====================
# 启用静态链接
set(BUILD_SHARED_LIBS OFF)
set(OpenCV_STATIC ON)

# 智能设置 MSVC 运行时库（/MT 或 /MTd）
set(CompilerFlags
    CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE 
    CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG 
    CMAKE_C_FLAGS_RELEASE
)
foreach(flag ${CompilerFlags})
    set(current "${${flag}}")
    
    if(current MATCHES "/MDd")
        string(REPLACE "/MDd" "/MTd" ${flag} "${current}")
    elseif(current MATCHES "/MD")
        string(REPLACE "/MD" "/MT" ${flag} "${current}")
    elseif(flag MATCHES "DEBUG")
        set(${flag} "/MTd ${current}")
    else()
        set(${flag} "/MT ${current}")
    endif()
endforeach()
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
message(STATUS "Chartstore Static Configuration")
message(STATUS "========================================")
message(STATUS "BUILD_SHARED_LIBS:   ${BUILD_SHARED_LIBS}")
message(STATUS "OpenCV_STATIC:       ${OpenCV_STATIC}")
message(STATUS "CMAKE_CXX_FLAGS:     ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "========================================")