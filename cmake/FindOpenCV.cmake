# FindOpenCV.cmake - 专用于OpenCV 4.10.0的查找模块
# 适配路径: D:\AppData\opencv

# 设置OpenCV根目录
set(OpenCV_ROOT_DIR "D:/AppData/opencv")
set(OpenCV_BUILD_DIR "${OpenCV_ROOT_DIR}/build")

# 检查OpenCV安装是否存在
if(NOT EXISTS ${OpenCV_BUILD_DIR})
    message(FATAL_ERROR "OpenCV build directory not found: ${OpenCV_BUILD_DIR}")
endif()

# 设置版本信息 - 更新为4.10.0
set(OpenCV_VERSION_MAJOR 4)
set(OpenCV_VERSION_MINOR 10)
set(OpenCV_VERSION_PATCH 0)
set(OpenCV_VERSION "${OpenCV_VERSION_MAJOR}.${OpenCV_VERSION_MINOR}.${OpenCV_VERSION_PATCH}")

# 设置包含目录
set(OpenCV_INCLUDE_DIRS
    "${OpenCV_BUILD_DIR}/include"
    "${OpenCV_BUILD_DIR}/include/opencv"
    "${OpenCV_BUILD_DIR}/include/opencv2"
)

# 根据编译器类型设置库目录
if(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64位
        set(OpenCV_LIB_DIR "${OpenCV_BUILD_DIR}/x64/vc16/lib")
        set(OpenCV_BIN_DIR "${OpenCV_BUILD_DIR}/x64/vc16/bin")
    else()
        # 32位
        set(OpenCV_LIB_DIR "${OpenCV_BUILD_DIR}/x86/vc16/lib")
        set(OpenCV_BIN_DIR "${OpenCV_BUILD_DIR}/x86/vc16/bin")
    endif()
else()
    message(WARNING "Only MSVC compiler is tested with this OpenCV configuration")
    set(OpenCV_LIB_DIR "${OpenCV_BUILD_DIR}/lib")
    set(OpenCV_BIN_DIR "${OpenCV_BUILD_DIR}/bin")
endif()

# 检查库目录是否存在
if(NOT EXISTS ${OpenCV_LIB_DIR})
    message(FATAL_ERROR "OpenCV library directory not found: ${OpenCV_LIB_DIR}")
endif()

# 定义OpenCV核心模块
set(OpenCV_MODULES
    core
    imgproc
    imgcodecs
    highgui
    features2d
    calib3d
    objdetect
    video
    videoio
    imgfilters
    photo
)

# 查找库文件
set(OpenCV_LIBS "")
set(OpenCV_LIBS_DEBUG "")

foreach(module ${OpenCV_MODULES})
    # Release库
    find_library(OpenCV_${module}_LIBRARY
        NAMES opencv_${module}4100 opencv_${module}410 opencv_${module}
        PATHS ${OpenCV_LIB_DIR}
        NO_DEFAULT_PATH
    )
    
    # Debug库
    find_library(OpenCV_${module}_LIBRARY_DEBUG
        NAMES opencv_${module}4100d opencv_${module}410d opencv_${module}d
        PATHS ${OpenCV_LIB_DIR}
        NO_DEFAULT_PATH
    )
    
    if(OpenCV_${module}_LIBRARY)
        list(APPEND OpenCV_LIBS ${OpenCV_${module}_LIBRARY})
        message(STATUS "Found OpenCV ${module}: ${OpenCV_${module}_LIBRARY}")
    endif()
    
    if(OpenCV_${module}_LIBRARY_DEBUG)
        list(APPEND OpenCV_LIBS_DEBUG ${OpenCV_${module}_LIBRARY_DEBUG})
    endif()
endforeach()

# 如果找到world库，优先使用（OpenCV 4.x常用world库）
find_library(OpenCV_world_LIBRARY
    NAMES opencv_world4100 opencv_world410 opencv_world
    PATHS ${OpenCV_LIB_DIR}
    NO_DEFAULT_PATH
)

find_library(OpenCV_world_LIBRARY_DEBUG
    NAMES opencv_world4100d opencv_world410d opencv_worldd
    PATHS ${OpenCV_LIB_DIR}
    NO_DEFAULT_PATH
)

if(OpenCV_world_LIBRARY)
    set(OpenCV_LIBS ${OpenCV_world_LIBRARY})
    set(OpenCV_LIBS_DEBUG ${OpenCV_world_LIBRARY_DEBUG})
    message(STATUS "Using OpenCV world library: ${OpenCV_world_LIBRARY}")
endif()

# 验证必要的头文件
find_path(OpenCV_CORE_INCLUDE_DIR
    NAMES opencv2/opencv.hpp
    PATHS ${OpenCV_INCLUDE_DIRS}
    NO_DEFAULT_PATH
)

if(NOT OpenCV_CORE_INCLUDE_DIR)
    message(FATAL_ERROR "OpenCV core headers not found in: ${OpenCV_INCLUDE_DIRS}")
endif()

# 设置找到标志
set(OpenCV_FOUND TRUE)

# 检查是否找到必要的库
if(NOT OpenCV_LIBS)
    set(OpenCV_FOUND FALSE)
    message(FATAL_ERROR "No OpenCV libraries found in: ${OpenCV_LIB_DIR}")
endif()

# 创建导入目标
if(OpenCV_FOUND AND NOT TARGET opencv::opencv)
    add_library(opencv::opencv INTERFACE IMPORTED)
    
    set_target_properties(opencv::opencv PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${OpenCV_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${OpenCV_LIBS}"
    )
    
    # 设置Debug/Release特定的库
    if(OpenCV_LIBS_DEBUG)
        set_target_properties(opencv::opencv PROPERTIES
            INTERFACE_LINK_LIBRARIES_DEBUG "${OpenCV_LIBS_DEBUG}"
            INTERFACE_LINK_LIBRARIES_RELEASE "${OpenCV_LIBS}"
        )
    endif()
endif()

# 输出配置信息
if(OpenCV_FOUND)
    message(STATUS "OpenCV Configuration:")
    message(STATUS "  Version: ${OpenCV_VERSION}")
    message(STATUS "  Include dirs: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "  Library dir: ${OpenCV_LIB_DIR}")
    message(STATUS "  Libraries: ${OpenCV_LIBS}")
    if(OpenCV_LIBS_DEBUG)
        message(STATUS "  Debug Libraries: ${OpenCV_LIBS_DEBUG}")
    endif()
    message(STATUS "  Binary dir: ${OpenCV_BIN_DIR}")
endif()

# 设置标准变量
set(OpenCV_LIBRARIES ${OpenCV_LIBS})
set(OpenCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS})

# 创建兼容的变量名
set(OpenCV_DIR ${OpenCV_BUILD_DIR})

# 标记为高级变量
mark_as_advanced(
    OpenCV_ROOT_DIR
    OpenCV_BUILD_DIR
    OpenCV_LIB_DIR
    OpenCV_BIN_DIR
    OpenCV_CORE_INCLUDE_DIR
) 