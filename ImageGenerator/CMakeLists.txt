cmake_minimum_required(VERSION 3.15)
project(ImageGenerator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# 查找 OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
endif()

# 查找 FreeType (必需)
find_package(Freetype REQUIRED)
if(FREETYPE_FOUND)
    message(STATUS "FreeType found: ${FREETYPE_VERSION_STRING}")
endif()

# 添加源文件
set(IMAGEGEN_SOURCES
    src/dynachart_renderer.cpp
)

# 创建静态库
add_library(ImageGenerator STATIC ${IMAGEGEN_SOURCES})

# 设置包含目录
target_include_directories(ImageGenerator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接 ChartStore 库（通过 add_dependency 方式）
# 注意：ChartStore 必须在根 CMakeLists.txt 中先于 ImageGenerator 被 add_subdirectory
add_dependencies(ImageGenerator ChartStore)
target_link_libraries(ImageGenerator PUBLIC
    ${OpenCV_LIBS}
    ChartStore
)

# 链接 FreeType
target_link_libraries(ImageGenerator PUBLIC ${FREETYPE_LIBRARIES})
target_include_directories(ImageGenerator PUBLIC ${FREETYPE_INCLUDE_DIRS})

# MSVC 静态编译标志
if(MSVC)
    # 静态链接运行时
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

# 安装规则
install(TARGETS ImageGenerator
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
