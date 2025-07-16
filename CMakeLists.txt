cmake_minimum_required(VERSION 3.10)
set(ENV{MSBUILDENSUREUTF8OUTPUT} 1) # Force MSBuild to output in UTF-8 to prevent garbled text
project(MatchTemplate)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置OpenCV路径
list(APPEND CMAKE_PREFIX_PATH "D:/APPS/Environments/OpenCV4.10.0/opencv/build")

# 查找OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
else()
    message(FATAL_ERROR "OpenCV not found!")
endif()

# 查找OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
else()
    message(WARNING "OpenMP not found, performance may be reduced")
endif()

# 包含目录
include_directories(${OpenCV_INCLUDE_DIRS})

# 添加可执行文件
add_executable(${PROJECT_NAME} 
    main.cpp
    KcgMatch_fixed.cpp # 使用修复后的文件
    KcgMatch.h
    PathHelpers.cpp
    PathHelpers.h
)

# 链接库
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# 如果找到OpenMP，链接OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
endif()

# 设置编译选项
if(MSVC)
    # Visual Studio编译器选项
    target_compile_options(${PROJECT_NAME} PRIVATE /W3)
    # 启用多处理器编译
    target_compile_options(${PROJECT_NAME} PRIVATE /MP)
    # 设置源文件和执行字符集为UTF-8
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
else()
    # GCC/Clang编译器选项
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O3)
endif()

# 复制资源文件到Debug和Release目录
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    file(COPY ${CMAKE_SOURCE_DIR}/template DESTINATION ${CMAKE_BINARY_DIR}/${CONFIG})
    file(COPY ${CMAKE_SOURCE_DIR}/config.jsonc DESTINATION ${CMAKE_BINARY_DIR}/${CONFIG})
endforeach()

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 打印构建信息
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")