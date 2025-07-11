cmake_minimum_required(VERSION 3.11) # CMake install : https://cmake.org/download/
project(MutiCamApp LANGUAGES CXX)

# 基本配置
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "c:/Qt/6.8.0/msvc2022_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)  # 升级到C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC UTF-8 编码支持
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 添加自定义CMake模块路径
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# OpenCV 3.10 配置 - 使用自定义查找模块
find_package(OpenCV REQUIRED)

# Qt6 组件
find_package(Qt6 COMPONENTS 
    Core 
    Widgets 
    Gui
    Concurrent  # 用于多线程处理
    REQUIRED)

# 线程支持
find_package(Threads REQUIRED)

# 海康威视SDK配置
set(HIK_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SDK)
set(HIK_INCLUDE_DIR ${HIK_SDK_DIR}/include)
set(HIK_LIB_DIR ${HIK_SDK_DIR}/Lib)

# 验证SDK目录存在
if(NOT EXISTS ${HIK_INCLUDE_DIR}/MvCameraControl.h)
    message(FATAL_ERROR "海康SDK头文件未找到: ${HIK_INCLUDE_DIR}/MvCameraControl.h")
endif()

if(NOT EXISTS ${HIK_LIB_DIR}/MvCameraControl.lib)
    message(FATAL_ERROR "海康SDK库文件未找到: ${HIK_LIB_DIR}/MvCameraControl.lib")
endif()

# 包含目录
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${HIK_INCLUDE_DIR})  # 添加海康SDK头文件路径
include_directories(src)

# 定义源文件组织
set(DOMAIN_SOURCES
    # 相机领域
    src/domain/camera/entities/Camera.cpp
    src/domain/camera/entities/CameraParams.cpp
    src/domain/camera/entities/Frame.cpp
    src/domain/camera/services/CameraManager.cpp
    
    # 测量领域
    src/domain/measurement/entities/Point.cpp
    src/domain/measurement/entities/Line.cpp
    src/domain/measurement/entities/Circle.cpp
    src/domain/measurement/entities/MeasurementResult.cpp
    src/domain/measurement/services/GeometryCalculator.cpp
    src/domain/measurement/services/MeasurementValidator.cpp
    
    # 绘图领域
    src/domain/drawing/services/DrawingManager.cpp
    src/domain/drawing/services/LayerManager.cpp
    
    # 公共领域
    src/domain/common/ValueObjects.cpp
)

set(INFRASTRUCTURE_SOURCES
    # 相机基础设施
    src/infrastructure/camera/HikVisionCamera.cpp
    
    # 线程管理
    src/infrastructure/threading/CameraThread.cpp
)

set(APPLICATION_SOURCES
    # 应用服务
    src/application/services/ApplicationService.cpp
    src/application/services/MeasurementService.cpp
    src/application/services/ImageProcessingService.cpp
)

set(PRESENTATION_SOURCES
    # 主窗口
    src/presentation/mainwindow/MainWindow.cpp
    src/presentation/mainwindow/MainWindowController.cpp
    
    # 对话框
    src/presentation/dialogs/ParameterDialog.cpp
    src/presentation/dialogs/CalibrationDialog.cpp
    src/presentation/dialogs/AboutDialog.cpp
    
    # 自定义控件
    src/presentation/widgets/ImageView.cpp
    src/presentation/widgets/MeasurementPanel.cpp
    src/presentation/widgets/ToolBar.cpp
    src/presentation/widgets/StatusDisplay.cpp
    
    # 表示层公共
    src/presentation/common/ViewModels.cpp
    src/presentation/common/UIUtilities.cpp
)

set(UTILS_SOURCES
    src/utils/DebugUtils.cpp
)

# 主程序源文件
set(MAIN_SOURCES
    src/main.cpp
    src/MutiCamApp.cpp
    src/MutiCamApp.h
    src/MutiCamApp.ui
    ${INFRASTRUCTURE_SOURCES}
    ${UTILS_SOURCES}
    # 添加绘图功能
    src/domain/drawing/services/DrawingManager.cpp
    src/domain/drawing/services/LayerManager.cpp
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    WIN32 # GUI应用程序
    ${MAIN_SOURCES}
)

# 海康SDK库文件
set(HIK_LIBRARIES ${HIK_LIB_DIR}/MvCameraControl.lib)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Concurrent
    ${OpenCV_LIBS}
    ${HIK_LIBRARIES}  # 添加海康SDK库
    Threads::Threads
)

# 编译器特定配置
if(MSVC)
    # MSVC特定设置
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
endif()

# Debug/Release配置
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# 复制OpenCV DLL到输出目录（Release模式）
if(WIN32)
    # 获取OpenCV DLL路径
    get_filename_component(OpenCV_BIN_DIR ${OpenCV_DIR} DIRECTORY)
    set(OpenCV_BIN_DIR "${OpenCV_BIN_DIR}/x64/vc15/bin")
    
    # 复制必要的DLL文件
    if(EXISTS ${OpenCV_BIN_DIR})
        file(GLOB OpenCV_DLLS "${OpenCV_BIN_DIR}/opencv_world*.dll")
        foreach(dll ${OpenCV_DLLS})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${dll} $<TARGET_FILE_DIR:${PROJECT_NAME}>
            )
        endforeach()
    endif()
endif()

# 安装配置
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装资源文件
install(DIRECTORY resources/
    DESTINATION bin/resources
)

# 开发者选项
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_DOCS "Build documentation" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()

# 打印配置信息
message(STATUS "=== MutiCamApp 构建配置 ===")
message(STATUS "Qt6 版本: ${Qt6_VERSION}")
message(STATUS "OpenCV 版本: ${OpenCV_VERSION}")
message(STATUS "OpenCV 路径: ${OpenCV_DIR}")
message(STATUS "海康SDK 路径: ${HIK_SDK_DIR}")
message(STATUS "海康SDK 头文件: ${HIK_INCLUDE_DIR}")
message(STATUS "海康SDK 库文件: ${HIK_LIBRARIES}")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "输出目录: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "=============================")