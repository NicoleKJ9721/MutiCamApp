cmake_minimum_required(VERSION 3.10) # CMake install : https://cmake.org/download/
project(MutiCamApp LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17) # 提升到C++17以支持更好的特性
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置默认构建类型为RelWithDebInfo（优化版本+调试信息）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
    message(STATUS "构建类型设置为: RelWithDebInfo (优化+调试信息)")
endif()



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
set(MCC6_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mcc6/lib)

# 设置OpenCV路径
set(OpenCV_INCLUDE_DIRS ${OPENCV_DIR}/include)
set(OpenCV_LIB_DIR ${OPENCV_DIR}/lib)
set(OpenCV_BIN_DIR ${OPENCV_DIR}/bin)

# 设置海康SDK路径
set(HIKVISION_INCLUDE_DIRS ${HIKVISION_DIR}/include)
set(HIKVISION_LIB_DIR ${HIKVISION_DIR}/lib)

# 设置MCC6轴控制库路径
set(MCC6_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mcc6/include)
set(MCC6_LIB_DIR ${MCC6_DIR})

find_package(Qt6 COMPONENTS Widgets Concurrent Svg SerialPort REQUIRED) # Qt COMPONENTS
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
    src/ui/StageAssistedCalibrationDialog.cpp
    src/ui/StageAssistedCalibrationDialog.h
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
    src/utils/SerialPortDetector.cpp
    src/utils/SerialPortDetector.h
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

    # 配置管理模块
    src/config/TemplateMatchingConfig.h
    src/config/TemplateMatchingConfig.cpp
    
    # 轴控制模块
    src/controllers/AxisControllerEnums.h
    src/controllers/AxisController.h
    src/controllers/AxisController.cpp
)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(${PROJECT_NAME}
    WIN32 # 发布版本时启用此选项以隐藏控制台窗口，提供更好的用户体验
           # 开发调试时注释此行，以便查看std::cout等控制台输出
           # 注意：启用WIN32后，程序将以窗口模式运行，不显示控制台
    ${srcs} 
)

# 添加头文件包含路径
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${HIKVISION_INCLUDE_DIRS}
    ${HALCON_INCLUDE_DIRS} # 添加 HALCON 包含路径
    ${MCC6_INCLUDE_DIRS} # 添加 MCC6 轴控制库包含路径
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/matching
    ${CMAKE_CURRENT_SOURCE_DIR}/src/axis
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# 处理海康SDK编码警告 - 屏蔽C4828警告（无效字符编码）
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4828)
    
    # 多相机应用性能优化编译选项
    target_compile_options(${PROJECT_NAME} PRIVATE 
        # Release模式：最大性能优化
        $<$<CONFIG:Release>:/O2 /Ob2 /DNDEBUG /favor:INTEL64>
        # RelWithDebInfo模式：优化+调试信息（推荐用于开发）
        $<$<CONFIG:RelWithDebInfo>:/O2 /Ob1 /Zi /DEBUG:FULL /favor:INTEL64>
        # Debug模式：保持调试友好性
        $<$<CONFIG:Debug>:/Od /Zi /DEBUG:FULL /RTC1>
    )
    message(STATUS "已启用多相机应用性能优化编译选项")
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

# 链接MCC6轴控制库
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${MCC6_LIB_DIR}/MCC6DLL_x64_Release.lib
)

# 链接HALCON库
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${HALCON_LIBS}
)

# 链接Qt6库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Concurrent Qt6::Svg Qt6::SerialPort)

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

# 复制MCC6 DLL到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MCC6_LIB_DIR}/MCC6DLL_x64_Release.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
