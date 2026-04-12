cmake_minimum_required(VERSION 3.15)
project(ChartStore VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置配置类型
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# 添加源文件
set(CHARTSTORE_SOURCES
    src/chart_store.cpp
)

# 创建静态库
add_library(ChartStore STATIC ${CHARTSTORE_SOURCES})

# 设置包含目录
target_include_directories(ChartStore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# MSVC 静态编译标志
if(MSVC)
    set_target_properties(ChartStore PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    
    # 静态链接运行时
    #compile settings: /MT
    # 静态链接运行时
    foreach(CompilerFlag CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE 
            CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG 
            CMAKE_C_FLAGS_RELEASE)
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    string(REPLACE "-MD" "-MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif()

# 安装规则
install(TARGETS ChartStore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
