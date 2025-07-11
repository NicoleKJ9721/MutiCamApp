# MutiCamApp C++重构架构设计文档

## 1. 项目概述

### 1.1 重构目标
将现有的Python版本MutiCamApp重构为C++版本，以满足：
- **性能提升**: 利用C++原生性能优势
- **模块化设计**: 解决Python版本代码集中的问题
- **团队协作**: 支持多人并行开发
- **新需求支持**: 集成只有C++接口的第三方库

### 1.2 技术选型
- **C++标准**: C++17
- **GUI框架**: Qt6.8.0
- **图像处理**: OpenCV 3.10 (D:\AppData\opencv)
- **构建系统**: CMake 3.11+
- **编译器**: MSVC 2022

## 2. 架构设计

### 2.1 分层架构
```
┌─────────────────────────────────────────────┐
│  Presentation Layer (表示层)                │
│  ├── MainWindow (主窗口)                    │
│  ├── Dialogs (对话框)                       │
│  ├── Custom Widgets (自定义控件)             │
│  └── View Controllers (视图控制器)           │
├─────────────────────────────────────────────┤
│  Application Layer (应用层)                 │
│  ├── Application Services (应用服务)         │
│  ├── Workflow Orchestration (工作流编排)     │
│  └── Use Case Implementation (用例实现)      │
├─────────────────────────────────────────────┤
│  Domain Layer (领域层)                      │
│  ├── Camera Domain (相机领域)                │
│  ├── Measurement Domain (测量领域)           │
│  ├── Drawing Domain (绘图领域)              │
│  └── Common Domain (公共领域)               │
├─────────────────────────────────────────────┤
│  Infrastructure Layer (基础设施层)           │
│  ├── Camera Hardware (相机硬件)             │
│  ├── Image Processing (图像处理)            │
│  ├── File I/O (文件操作)                   │
│  └── Threading (线程管理)                  │
└─────────────────────────────────────────────┘
```

### 2.2 目录结构说明

#### 2.2.1 Domain Layer (领域层)
```
src/domain/
├── camera/           # 相机领域
│   ├── entities/     # 实体对象
│   │   ├── Camera.h/cpp        # 相机实体
│   │   ├── CameraParams.h/cpp  # 相机参数
│   │   └── Frame.h/cpp         # 帧数据
│   ├── services/     # 领域服务
│   │   └── CameraManager.h/cpp # 相机管理服务
│   └── interfaces/   # 领域接口
│       └── ICameraController.h # 相机控制接口
├── measurement/      # 测量领域
│   ├── entities/     # 几何实体
│   │   ├── Point.h/cpp         # 点
│   │   ├── Line.h/cpp          # 线
│   │   ├── Circle.h/cpp        # 圆
│   │   └── MeasurementResult.h/cpp # 测量结果
│   ├── services/     # 测量服务
│   │   ├── GeometryCalculator.h/cpp    # 几何计算
│   │   └── MeasurementValidator.h/cpp  # 测量验证
│   └── interfaces/   # 测量接口
│       └── IMeasurementCalculator.h    # 测量计算接口
├── drawing/          # 绘图领域
│   ├── entities/     # 绘图实体
│   │   ├── DrawingObject.h/cpp # 绘图对象
│   │   ├── Layer.h/cpp         # 图层
│   │   └── DrawingStyle.h/cpp  # 绘图样式
│   ├── services/     # 绘图服务
│   │   ├── DrawingManager.h/cpp # 绘图管理
│   │   └── LayerManager.h/cpp   # 图层管理
│   └── interfaces/   # 绘图接口
│       └── IDrawingRenderer.h   # 绘图渲染接口
└── common/           # 公共领域
    ├── ValueObjects.h/cpp # 值对象
    ├── Enums.h           # 枚举定义
    └── Constants.h       # 常量定义
```

#### 2.2.2 Infrastructure Layer (基础设施层)
```
src/infrastructure/
├── camera/           # 相机基础设施
│   ├── HikVisionSDK.h/cpp     # 海康SDK封装
│   └── CameraController.h/cpp # 相机控制器
├── imageprocessing/  # 图像处理
│   ├── OpenCVProcessor.h/cpp  # OpenCV处理器
│   ├── EdgeDetector.h/cpp     # 边缘检测
│   └── ShapeDetector.h/cpp    # 形状检测
├── storage/          # 存储
│   ├── ConfigManager.h/cpp    # 配置管理
│   ├── LogManager.h/cpp       # 日志管理
│   └── FileHandler.h/cpp      # 文件处理
└── threading/        # 线程管理
    ├── ThreadPool.h/cpp       # 线程池
    └── CameraWorker.h/cpp     # 相机工作线程
```

#### 2.2.3 Application Layer (应用层)
```
src/application/
├── services/         # 应用服务
│   ├── ApplicationService.h/cpp      # 主应用服务
│   ├── MeasurementService.h/cpp      # 测量应用服务
│   └── ImageProcessingService.h/cpp  # 图像处理应用服务
└── interfaces/       # 应用接口
    ├── ICameraService.h        # 相机服务接口
    ├── IMeasurementService.h   # 测量服务接口
    └── IImageProcessor.h       # 图像处理接口
```

#### 2.2.4 Presentation Layer (表示层)
```
src/presentation/
├── mainwindow/       # 主窗口
│   ├── MainWindow.h/cpp           # 主窗口实现
│   ├── MainWindow.ui              # 主窗口界面
│   └── MainWindowController.h/cpp # 主窗口控制器
├── dialogs/          # 对话框
│   ├── ParameterDialog.h/cpp      # 参数设置对话框
│   ├── CalibrationDialog.h/cpp    # 标定对话框
│   └── AboutDialog.h/cpp          # 关于对话框
├── widgets/          # 自定义控件
│   ├── ImageView.h/cpp            # 图像显示控件
│   ├── MeasurementPanel.h/cpp     # 测量面板
│   ├── ToolBar.h/cpp              # 工具栏
│   └── StatusDisplay.h/cpp        # 状态显示
└── common/           # 表示层公共
    ├── ViewModels.h/cpp           # 视图模型
    └── UIUtilities.h/cpp          # UI工具类
```

## 3. 设计模式应用

### 3.1 依赖注入 (Dependency Injection)
```cpp
class MeasurementService {
private:
    std::shared_ptr<IMeasurementCalculator> calculator_;
    std::shared_ptr<IImageProcessor> processor_;
    
public:
    MeasurementService(
        std::shared_ptr<IMeasurementCalculator> calculator,
        std::shared_ptr<IImageProcessor> processor)
        : calculator_(calculator), processor_(processor) {}
};
```

### 3.2 工厂模式 (Factory Pattern)
```cpp
class CameraFactory {
public:
    static std::unique_ptr<Camera> CreateCamera(
        const std::string& serialNumber, 
        CameraType type);
};
```

### 3.3 观察者模式 (Observer Pattern)
```cpp
class ICameraObserver {
public:
    virtual ~ICameraObserver() = default;
    virtual void OnFrameReceived(std::shared_ptr<Frame> frame) = 0;
    virtual void OnCameraStatusChanged(CameraStatus status) = 0;
};
```

### 3.4 策略模式 (Strategy Pattern)
```cpp
class IMeasurementStrategy {
public:
    virtual ~IMeasurementStrategy() = default;
    virtual MeasurementResult Calculate(const std::vector<Point>& points) = 0;
};

class LineMeasurementStrategy : public IMeasurementStrategy {
    // 直线测量策略实现
};

class CircleMeasurementStrategy : public IMeasurementStrategy {
    // 圆形测量策略实现
};
```

### 3.5 命令模式 (Command Pattern)
```cpp
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
};

class DrawLineCommand : public ICommand {
    // 绘制直线命令实现
};
```

## 4. 核心接口设计

### 4.1 相机控制接口
```cpp
class ICameraController {
public:
    virtual ~ICameraController() = default;
    virtual bool Initialize(const CameraParams& params) = 0;
    virtual bool StartCapture() = 0;
    virtual bool StopCapture() = 0;
    virtual std::shared_ptr<Frame> GetLatestFrame() = 0;
    virtual CameraStatus GetStatus() const = 0;
    virtual void RegisterObserver(std::shared_ptr<ICameraObserver> observer) = 0;
    virtual void UnregisterObserver(std::shared_ptr<ICameraObserver> observer) = 0;
};
```

### 4.2 测量计算接口
```cpp
class IMeasurementCalculator {
public:
    virtual ~IMeasurementCalculator() = default;
    virtual double CalculateDistance(const Point& p1, const Point& p2) = 0;
    virtual double CalculateAngle(const Line& l1, const Line& l2) = 0;
    virtual Circle FitCircle(const std::vector<Point>& points) = 0;
    virtual Line FitLine(const std::vector<Point>& points) = 0;
    virtual Point CalculateIntersection(const Line& l1, const Line& l2) = 0;
};
```

### 4.3 图像处理接口
```cpp
class IImageProcessor {
public:
    virtual ~IImageProcessor() = default;
    virtual void SetFrame(const cv::Mat& frame) = 0;
    virtual std::vector<Line> DetectLines(const EdgeDetectionParams& params) = 0;
    virtual std::vector<Circle> DetectCircles(const CircleDetectionParams& params) = 0;
    virtual cv::Mat EnhanceImage(const EnhancementParams& params) = 0;
    virtual cv::Mat ApplyFilter(FilterType type, const FilterParams& params) = 0;
};
```

## 5. 数据流设计

### 5.1 相机数据流
```
相机硬件 → CameraController → CameraWorker → ApplicationService → MainWindow
                                    ↓
                              ImageProcessor → MeasurementService → DrawingManager
```

### 5.2 测量数据流
```
用户输入 → MainWindow → MeasurementService → GeometryCalculator → MeasurementResult
                                                      ↓
                         DrawingManager → LayerManager → 渲染更新
```

### 5.3 配置数据流
```
配置文件 → ConfigManager → ApplicationService → 各模块初始化
                              ↓
                         UI参数更新 → 界面刷新
```

## 6. 错误处理策略

### 6.1 异常层次结构
```cpp
class MutiCamException : public std::exception {
public:
    explicit MutiCamException(const std::string& message);
    const char* what() const noexcept override;
};

class CameraException : public MutiCamException {
    // 相机相关异常
};

class MeasurementException : public MutiCamException {
    // 测量相关异常
};

class ImageProcessingException : public MutiCamException {
    // 图像处理相关异常
};
```

### 6.2 错误处理原则
- **异常安全**: 保证资源不泄漏
- **错误传播**: 适当的异常传播机制
- **用户友好**: 向用户提供有意义的错误信息
- **日志记录**: 详细的错误日志记录

## 7. 性能优化策略

### 7.1 内存管理
- 使用智能指针管理资源
- 对象池复用频繁创建的对象
- 及时释放大内存对象

### 7.2 多线程优化
- 相机采集线程与UI线程分离
- 图像处理使用线程池
- 避免线程间频繁数据拷贝

### 7.3 算法优化
- 使用OpenCV优化的算法
- ROI区域处理减少计算量
- 缓存计算结果

## 8. 测试策略

### 8.1 单元测试
- 每个类都有对应的单元测试
- 使用Google Test框架
- 模拟对象测试接口

### 8.2 集成测试
- 模块间接口测试
- 端到端功能测试
- 性能测试

### 8.3 UI测试
- 自动化UI测试
- 用户交互测试
- 界面响应性测试

## 9. 部署和打包

### 9.1 依赖管理
- 自动检测和配置OpenCV
- 自动复制必要的DLL文件
- 打包脚本生成可分发版本

### 9.2 安装程序
- 使用NSIS或Inno Setup
- 自动安装依赖库
- 配置环境变量

## 10. 开发规范

### 10.1 编码规范
- 使用一致的命名约定
- 充分的代码注释
- 遵循SOLID原则
- 使用现代C++特性

### 10.2 版本控制
- 功能分支开发
- 代码审查机制
- 持续集成

### 10.3 文档维护
- API文档自动生成
- 架构图更新
- 用户手册维护

---

这个架构设计确保了：
✅ **高度模块化**: 清晰的职责分离，便于团队协作
✅ **可测试性**: 接口导向设计，便于单元测试
✅ **可扩展性**: 插件化架构，支持功能扩展
✅ **高性能**: C++原生性能 + 多线程优化
✅ **可维护性**: 标准化设计模式和编码规范 