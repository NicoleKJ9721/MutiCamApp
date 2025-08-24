# ===================================================================
#           CMakeLists.txt (最终的、强制生成并链接lib版)
# ===================================================================

cmake_minimum_required(VERSION 3.15)
project(MatchingSystem CXX)

# --- 1. 环境与依赖配置 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ENV{MSBUILDENSUREUTF8OUTPUT} 1)
list(APPEND CMAKE_PREFIX_PATH "D:/APPS/Environments/OpenCV4.10.0/opencv/build")
find_package(OpenCV REQUIRED)
find_package(OpenMP)
# ... (if/message 语句) ...
if(OpenCV_FOUND) 
    message(STATUS "OpenCV found: ${OpenCV_VERSION}") 
endif()
if(OpenMP_CXX_FOUND) 
    message(STATUS "OpenMP found") 
endif()

# --- 2. 定义核心库：MatchingController ---
add_library(MatchingController SHARED 
    KcgMatch.h KcgMatch_fixed.cpp 
    MatchingController.h MatchingController.cpp
)
target_include_directories(MatchingController PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(MatchingController PUBLIC ${OpenCV_LIBS})
if(OpenMP_CXX_FOUND)
    target_link_libraries(MatchingController PRIVATE OpenMP::OpenMP_CXX)
endif()
set_target_properties(MatchingController PROPERTIES DEBUG_POSTFIX "d")

# 【【【 核心修正 A: 强制生成导入库 】】】
# 在Windows平台上，确保为共享库（DLL）生成一个导入库（.lib）
if(WIN32)
    set_target_properties(MatchingController PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

# --- 3. 定义测试程序：MatchTemplate ---
add_executable(MatchTemplate main.cpp)

# 【【【 核心修正 B: 使用最直接的链接方式 】】】
# 我们直接链接到目标，让CMake处理所有底层的路径和文件名
target_link_libraries(MatchTemplate PRIVATE MatchingController)


# --- 4. 统一设置编译选项 ---
foreach(TARGET MatchingController MatchTemplate)
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE /W3 /MP /utf-8)
    else()
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -O3)
    endif()
endforeach()


# --- 5. 构建后事件：自动复制运行时所需文件 ---
# 确保测试程序可以直接运行
add_custom_command(TARGET MatchTemplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/config.jsonc"
        "$<TARGET_FILE_DIR:MatchTemplate>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/template"
        "$<TARGET_FILE_DIR:MatchTemplate>/template"

    # 使用硬编码路径复制OpenCV的DLL，这是最可靠的方式
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/APPS/Environments/OpenCV4.10.0/opencv/build/x64/vc16/bin/opencv_world4100.dll"
        "$<TARGET_FILE_DIR:MatchTemplate>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/APPS/Environments/OpenCV4.10.0/opencv/build/x64/vc16/bin/opencv_world4100d.dll"
        "$<TARGET_FILE_DIR:MatchTemplate>"

    # 复制我们自己的DLL，以便可执行文件能找到它
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:MatchingController>"
        "$<TARGET_FILE_DIR:MatchTemplate>"
        
    COMMENT "Copying runtime resources and dependencies..."
)