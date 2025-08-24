## **`MatchingController` 后端模块交付报告 & API集成指南**

**版本:** 1.0
**日期:** 2025年7月17日
**负责人:** caigou

### **1. 概述 (Overview)**

`MatchingController` 是一个高性能、线程安全的C++后端模块，它封装了所有基于形状的模板匹配算法的复杂细节。该模块旨在提供实时的、可配置的模板匹配与创建功能。

前端应用程序通过与本模块定义的公共API进行交互，可以实现对视频流或静态图像的目标检测，并能以异步方式创建新的识别模板，同时保持UI界面的流畅响应。

### **2. 交付物 (Deliverables)**

为了集成此模块，你需要以下文件：

1.  **头文件 (Header Files):**
    *   `MatchingController.h`: 包含本模块的唯一公共接口。
    *   `KcgMatch.h`: 包含了核心数据结构 `kcg::Match` 的定义 (虽然不直接使用，但作为依赖需要)。
    *   `third_party/json.hpp`: nlohmann::json库，用于参数传递。

2.  **编译库文件 (Compiled Libraries):**
    *   **Windows:**
        *   `MatchingController.lib` (静态链接库)
        *   `MatchingController.dll` (动态链接库)
    *   **Linux:**
        *   `libMatchingController.a` (静态库)
        *   `libMatchingController.so` (共享对象)

### **3. 环境与依赖 (Environment & Dependencies)**

本模块在以下环境中编译和测试：

*   **C++ 标准:** C++17 或更高版本 (因使用了 `std::filesystem`)。
*   **核心依赖:**
    *   **OpenCV:** 4.x.x 版本。
    *   **nlohmann::json:** 3.x.x 版本 (已作为头文件提供)。
*   **编译环境:** (例如) Visual Studio 2019 / g++ 9.4.0

前端项目在集成时，必须确保链接到相同或ABI兼容的OpenCV版本。

### **4. 集成步骤 (CMake 示例)**

在你的QT项目的 `CMakeLists.txt` 中，你需要：

```cmake
# 1. 找到OpenCV
find_package(OpenCV 4 REQUIRED)

# 2. 指定后端模块的头文件和库文件路径
include_directories(/path/to/our/headers)
link_directories(/path/to/our/libs)

# 3. 将头文件目录添加到你的目标
target_include_directories(YourQtApp PRIVATE ${OpenCV_INCLUDE_DIRS} /path/to/our/headers)

# 4. 链接我们的库和OpenCV库到你的目标
target_link_libraries(YourQtApp PRIVATE MatchingController ${OpenCV_LIBS})
```

### **5. API 核心：`MatchingController` 类**

你需要创建并持有一个 `MatchingController` 类的实例来使用所有功能。

#### **5.1. 生命周期管理**

*   **构造函数:** `MatchingController()`
    *   创建一个控制器实例。

*   **初始化:** `bool initialize(const std::string& config_path)`
    *   **【必须调用的第一个函数】**。根据指定的配置文件路径，加载所有参数并初始化内部的模板匹配器。
    *   **返回值**: `true` 代表成功，`false` 代表失败。**前端必须检查此返回值**，如果为`false`，则不应调用任何其他功能。

*   **析构函数:** `~MatchingController()`
    *   在对象被销毁时自动释放所有资源。

#### **5.2. 核心功能**

*   **处理单帧:** `std::vector<EnrichedMatchResult> processSingleFrame(const cv::Mat& frame)`
    *   **【核心工作函数】**。接收一帧图像，执行匹配。
    *   **重要提示**: 这是一个**计算密集型**的操作，**强烈建议在QT的后台工作线程 (如 `QtConcurrent::run` 或 `QThread`) 中调用**，以避免阻塞UI主线程。
    *   **参数**: `frame` - 从相机或文件获取的原始 **BGR 彩色** `cv::Mat` 图像。
    *   **返回值**: 一个包含所有丰富匹配结果的 `std::vector`。

*   **异步创建模板:** `std::future<bool> createTemplateAsync(...)`
    *   在一个后台线程中创建一个新的模板。此函数会**立即返回**，不会阻塞。
    *   **参数**:
        *   `sourceImage`: 用于创建模板的源图像 (`cv::Mat`)。
        *   `templateROI`: 用户在源图像上框选的模板区域 (`cv::Rect`)。
        *   `new_class_name`: 新模板的名称（将作为文件名）。
    *   **返回值**: `std::future<bool>`。前端需要保存这个`future`对象，并用它来查询后台任务的状态和最终结果（`true`为成功）。
    *   **UI处理**: 在调用此函数后，建议禁用“创建模板”按钮，并使用`QTimer`定期检查`future`的状态，任务完成后再恢复UI。

#### **5.3. 参数配置**

*   **设置ROI:** `void setROI(const cv::Rect& roi)`
    *   设置一个感兴趣区域来加速匹配。此函数是线程安全的。
    *   **坐标系**: `roi`的坐标必须是基于**全尺寸视频帧**的。

*   **设置单个参数:** `void setParameter(const std::string& name, const json& value)`
    *   动态修改一个内存中的参数，使其**立即生效**。此函数是线程安全的。
    *   **示例**:
        ```cpp
        controller.setParameter("ScoreThreshold", 0.5f);
        controller.setParameter("PyramidLevel", 4);
        controller.setParameter("RefinementSearchMode", "fixed");
        ```

*   **保存配置:** `bool saveConfiguration()`
    *   将当前内存中（可能已被`setParameter`修改过）的所有配置参数，写回到最初加载的`.jsonc`文件中。此函数是线程安全的。

*   **重载配置:** `bool reloadConfiguration()`
    *   从硬盘重新加载配置文件，并用文件中的参数**完全重置**所有内部状态（包括重建模板匹配器）。用于实现“从文件加载配置”或“撤销所有未保存的更改”功能。

### **6. 核心数据结构：`EnrichedMatchResult`**

这是`processSingleFrame`返回的、你需要使用的核心数据结构。

```cpp
struct EnrichedMatchResult {
    float score;                  // 匹配得分 (0.0 ~ 1.0)
    float angle;                  // 匹配到的角度 (度)
    float scale;                  // 匹配到的缩放比例
    
    cv::Rect axis_aligned_box;    // 轴对齐的、不精确的外接矩形
    cv::RotatedRect rotated_box;  // 精确的旋转矩形 (含中心点、尺寸、角度)
    std::vector<cv::Point2f> corners; // 【最常用】精确的4个角点坐标 (全图坐标系)
};```

### **7. 配置文件说明 (`config.jsonc`)**

所有参数均由一个JSON文件管理，支持`//`和`/* ... */`风格的注释。前端可以根据此结构来构建一个完整的参数设置界面。

*   **`Paths`**: 定义所有文件路径。`ModelRootPath`是存放模板`.yaml`文件的文件夹。
*   **`Model`**: 定义默认加载的模型名称。
*   **`Matching`**: 定义所有**匹配时**的参数。
*   **`TemplateMaking`**: 定义所有**创建模板时**的参数。

### **8. 完整使用示例**

```cpp
#include "MatchingController.h"
#include <iostream>

int main() {
    // 1. 初始化
    MatchingController controller;
    if (!controller.initialize("path/to/your/config.jsonc")) {
        std::cerr << "Initialization failed!" << std::endl;
        return -1;
    }

    // 2. 加载图像并匹配
    cv::Mat image = cv::imread("path/to/your/search_image.jpg");
    if (image.empty()) return -1;
    
    // 3. 设置参数并执行
    controller.setROI({100, 100, 800, 800}); // 设置ROI
    controller.setParameter("ScoreThreshold", 0.6f); // 动态修改阈值

    // 4. 获取丰富结果 (在真实应用中，这一步应在后台线程)
    std::vector<EnrichedMatchResult> results = controller.processSingleFrame(image);

    // 5. 使用结果
    std::cout << "Found " << results.size() << " matches." << std::endl;
    if (!results.empty()) {
        std::cout << "Top match score: " << results[0].score << std::endl;
        std::cout << "Top match angle: " << results[0].angle << std::endl;
        // 前端可以直接使用 results[0].corners 在QT界面上绘制精确的四边形
    }

    // 6. 保存修改后的配置
    controller.saveConfiguration();

    return 0;
}
```

### **9. 错误处理与最佳实践**

*   **检查返回值**: 务必检查 `initialize()` 和 `reloadConfiguration()` 的 `bool` 返回值。
*   **线程安全**: `processSingleFrame` 和 `createTemplateAsync` 是耗时操作，必须在工作线程中调用。所有公共接口**本身都是线程安全的**，你可以在UI主线程中随时调用`setROI`或`setParameter`，无需担心与后台的匹配任务冲突。
*   **UI更新**: 从后台线程获取到匹配结果后，需要通过QT的信号-槽机制(Signal-Slot)将结果安全地传递给UI主线程进行界面更新。

---
