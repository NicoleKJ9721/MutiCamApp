cmake_minimum_required(VERSION 3.10) # CMake install : https://cmake.org/download/
project(MutiCamApp LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17) # 提升到C++17以支持更好的特性
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- HALCON 集成开始 ---
# 查找 HALCON 安装
if(DEFINED ENV{HALCONROOT} AND NOT $ENV{HALCONROOT} STREQUAL "")
  message(STATUS "正在使用环境变量中的 HALCON: " $ENV{HALCONROOT})
  STRING(REGEX REPLACE "\\\\" "/" HALCONROOT $ENV{HALCONROOT}) 
else()
  message(FATAL_ERROR "HALCONROOT 环境变量未设置。请将其设置为您的 HALCON 安装目录。")
endif()

# 设置 HALCON 头文件目录
set(HALCON_INCLUDE_DIRS 
    ${HALCONROOT}/include
    ${HALCONROOT}/include/halconcpp
)

# 设置 HALCON 库 (针对 64 位 Windows)
set(HALCON_LIBS
    ${HALCONROOT}/lib/x64-win64/halconcpp.lib
    ${HALCONROOT}/lib/x64-win64/halcon.lib
)

message(STATUS "HALCON 头文件目录: " ${HALCON_INCLUDE_DIRS})
message(STATUS "HALCON 库: " ${HALCON_LIBS})
# --- HALCON 集成结束 ---

# 配置第三方库路径
set(OPENCV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv)
set(HIKVISION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/hikvision)

# 设置OpenCV路径
set(OpenCV_INCLUDE_DIRS ${OPENCV_DIR}/include)
set(OpenCV_LIB_DIR ${OPENCV_DIR}/lib)
set(OpenCV_BIN_DIR ${OPENCV_DIR}/bin)

# 设置海康SDK路径
set(HIKVISION_INCLUDE_DIRS ${HIKVISION_DIR}/include)
set(HIKVISION_LIB_DIR ${HIKVISION_DIR}/lib)

find_package(Qt6 COMPONENTS Widgets Concurrent Svg REQUIRED) # Qt COMPONENTS
find_package(OpenMP)

# 手动指定源文件，确保所有文件都被包含
set(srcs
    # 核心应用文件
    src/core/main.cpp
    src/core/MutiCamApp.cpp
    src/core/MutiCamApp.h
    
    # UI组件文件
    src/ui/VideoDisplayWidget.cpp
    src/ui/VideoDisplayWidget.h
    src/ui/ZoomPanWidget.cpp
    src/ui/ZoomPanWidget.h
    src/ui/PaintingOverlay.cpp
    src/ui/PaintingOverlay.h
    src/ui/TemplateCreationDialog.h
    src/ui/TemplateCreationDialog.cpp
    src/ui/TemplateSelectionDialog.cpp
    src/ui/TemplateSelectionDialog.h
    src/ui/CalibrationDialog.cpp
    src/ui/CalibrationDialog.h
    src/ui/CheckerboardCalibrationDialog.cpp
    src/ui/CheckerboardCalibrationDialog.h
    src/ui/MutiCamApp.ui
    
    # 工具类文件
    src/utils/SettingsManager.cpp
    src/utils/SettingsManager.h
    src/utils/LogManager.cpp
    src/utils/LogManager.h
    src/utils/TrajectoryRecorder.cpp
    src/utils/TrajectoryRecorder.h
    src/utils/SerialController.cpp
    src/utils/SerialController.h
    src/utils/dependencies_test.cpp
    src/utils/dependencies_test.h
    
    # 核心模块头文件
    src/camera/camera_controller.h
    src/camera/camera_thread.h
    src/camera/hikvision_camera.h
    src/camera/camera_manager.cpp
    src/camera/camera_thread.cpp
    src/camera/hikvision_camera.cpp

    # 图像处理模块
    src/image_processing/edge_detector.h
    src/image_processing/edge_detector.cpp
    src/image_processing/shape_detector.h
    src/image_processing/shape_detector.cpp

    # 模板匹配模块
)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(${PROJECT_NAME}
    # WIN32 # 发布版本时启用此选项以隐藏控制台窗口，提供更好的用户体验
           # 开发调试时注释此行，以便查看std::cout等控制台输出
           # 注意：启用WIN32后，程序将以窗口模式运行，不显示控制台
    ${srcs} 
)

# 添加头文件包含路径
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${HIKVISION_INCLUDE_DIRS}
    ${HALCON_INCLUDE_DIRS} # 添加 HALCON 包含路径
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/matching
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# 处理海康SDK编码警告 - 屏蔽C4828警告（无效字符编码）
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4828)
endif()

# 链接OpenCV库
target_link_libraries(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:${OpenCV_LIB_DIR}/opencv_world4100d.lib>
    $<$<CONFIG:Release>:${OpenCV_LIB_DIR}/opencv_world4100.lib>
    $<$<CONFIG:RelWithDebInfo>:${OpenCV_LIB_DIR}/opencv_world4100.lib>
    $<$<CONFIG:MinSizeRel>:${OpenCV_LIB_DIR}/opencv_world4100.lib>
)

# 链接海康SDK库
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${HIKVISION_LIB_DIR}/MvCameraControl.lib
)

# 链接HALCON库
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${HALCON_LIBS}
)

# 链接Qt6库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Concurrent Qt6::Svg)

# 链接OpenMP库(如果找到)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found and enabled for matching acceleration")
endif()


# 复制OpenCV DLL到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${OpenCV_BIN_DIR}/opencv_world4100.dll"
        "${OpenCV_BIN_DIR}/opencv_world4100d.dll"
        "${OpenCV_BIN_DIR}/opencv_videoio_ffmpeg4100_64.dll"
        "${OpenCV_BIN_DIR}/opencv_videoio_msmf4100_64.dll"
        "${OpenCV_BIN_DIR}/opencv_videoio_msmf4100_64d.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)