cmake_minimum_required(VERSION 3.10)
project(zlib_cpp VERSION 1.2.11 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录
include_directories(include)

# 源文件
file(GLOB SOURCES 
    src/*.cpp
)

# 头文件
file(GLOB HEADERS
    include/*.hpp
)

# 创建库
add_library(zlib_cpp STATIC ${SOURCES} ${HEADERS})

# 安装规则
install(TARGETS zlib_cpp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(FILES ${HEADERS} DESTINATION include)

# 示例程序
option(BUILD_EXAMPLES "Build example programs" OFF)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 测试
option(BUILD_TESTS "Build test programs" OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
