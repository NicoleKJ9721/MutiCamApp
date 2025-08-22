cmake_minimum_required(VERSION 3.5) # CMake install : https://cmake.org/download/
project(MutiCamApp LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "c:/Qt/6.8.0/msvc2022_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17) # 提升到C++17以支持更好的特性
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

find_package(Qt6 COMPONENTS Widgets Concurrent REQUIRED) # Qt COMPONENTS

# 手动指定源文件，确保所有文件都被包含
set(srcs
    src/main.cpp
    src/MutiCamApp.cpp
    src/MutiCamApp.h
    src/VideoDisplayWidget.cpp
    src/VideoDisplayWidget.h
    src/ZoomPanWidget.cpp
    src/ZoomPanWidget.h
    src/PaintingOverlay.cpp
    src/PaintingOverlay.h
    src/SettingsManager.cpp
    src/SettingsManager.h
    src/CalibrationDialog.cpp
    src/CalibrationDialog.h
    src/LogManager.cpp
    src/LogManager.h
    src/TrajectoryRecorder.cpp
    src/TrajectoryRecorder.h
    src/SerialController.cpp
    src/SerialController.h
    src/dependencies_test.cpp
    src/dependencies_test.h
    src/MutiCamApp.ui
    
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
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

# 链接Qt6库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Concurrent)


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