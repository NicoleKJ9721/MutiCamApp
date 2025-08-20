cmake_minimum_required(VERSION 3.10)
project(SerialTest)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加可执行文件
add_executable(serial_test serial_test.cpp)

# Windows平台特定设置
if(WIN32)
    # 链接Windows API库
    target_link_libraries(serial_test kernel32)
    
    # 设置Windows子系统
    set_target_properties(serial_test PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# Linux平台特定设置
if(UNIX AND NOT APPLE)
    # 链接pthread库
    find_package(Threads REQUIRED)
    target_link_libraries(serial_test Threads::Threads)
endif()

# 编译选项
if(MSVC)
    target_compile_options(serial_test PRIVATE /W4)
else()
    target_compile_options(serial_test PRIVATE -Wall -Wextra -O2)
endif()

# 打印构建信息
message(STATUS "构建串口测试程序")
message(STATUS "目标文件: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/serial_test") 