# MutiCamApp C++重构开发计划

## 📋 项目概览

### 项目信息
- **项目名称**: MutiCamApp C++重构
- **开发周期**: 6周（预估）
- **团队规模**: 5-6人
- **技术栈**: C++17 + Qt6 + OpenCV 3.10

### 重构目标
- 解决Python版本代码集中问题（单文件4000+行）
- 实现高度模块化设计，便于团队协作
- 提升性能和稳定性
- 支持新的C++接口需求

## 🎯 开发分工

### 模块A: 基础设施层 (Infrastructure Layer) 
**负责人**: 成员A  
**工期**: 2周  
**优先级**: 高 (其他模块依赖)

#### 核心任务
```
src/infrastructure/
├── storage/
│   ├── ConfigManager.h/cpp      # 配置管理 [优先级: 高]
│   ├── LogManager.h/cpp         # 日志管理 [优先级: 高]
│   └── FileHandler.h/cpp        # 文件处理 [优先级: 中]
├── threading/
│   ├── ThreadPool.h/cpp         # 线程池 [优先级: 高]
│   └── CameraWorker.h/cpp       # 相机工作线程 [优先级: 高]
└── src/utils/
    ├── Math.h/cpp               # 数学工具 [优先级: 高]
    ├── StringUtils.h/cpp        # 字符串工具 [优先级: 中]
    ├── ImageUtils.h/cpp         # 图像工具 [优先级: 中]
    └── Timer.h/cpp              # 计时器 [优先级: 低]
```

#### 详细任务
1. **Week 1**: 
   - ConfigManager: JSON配置读写、参数验证
   - LogManager: 多级日志、文件轮转、线程安全
   - Math: 基础几何计算函数

2. **Week 2**: 
   - ThreadPool: 通用线程池实现
   - CameraWorker: 相机专用工作线程
   - 其他工具类实现

#### 交付物
- 完整的基础设施模块
- 单元测试覆盖
- 使用示例和文档

---

### 模块B: 相机控制层 (Camera Control Layer)
**负责人**: 成员B  
**工期**: 3周  
**优先级**: 高

#### 核心任务
```
src/domain/camera/           # 相机领域层
├── entities/
│   ├── Camera.h/cpp         # 相机实体 [优先级: 高]
│   ├── CameraParams.h/cpp   # 相机参数 [优先级: 高]
│   └── Frame.h/cpp          # 帧数据 [优先级: 高]
├── services/
│   └── CameraManager.h/cpp  # 相机管理 [优先级: 高]
└── interfaces/
    └── ICameraController.h  # 相机接口 [优先级: 高]

src/infrastructure/camera/   # 相机基础设施层
├── HikVisionSDK.h/cpp      # 海康SDK封装 [优先级: 高]
└── CameraController.h/cpp   # 相机控制器 [优先级: 高]
```

#### 详细任务
1. **Week 1**: 
   - 设计相机实体类和参数类
   - 实现相机控制接口定义
   - 开始海康SDK封装

2. **Week 2**: 
   - 完成海康SDK封装
   - 实现相机控制器
   - 多相机管理功能

3. **Week 3**: 
   - 相机线程集成
   - 错误处理和异常安全
   - 性能优化和测试

#### 关键接口
```cpp
class ICameraController {
public:
    virtual bool Initialize(const CameraParams& params) = 0;
    virtual bool StartCapture() = 0;
    virtual bool StopCapture() = 0;
    virtual std::shared_ptr<Frame> GetLatestFrame() = 0;
    virtual CameraStatus GetStatus() const = 0;
};
```

#### 交付物
- 完整的相机控制模块
- 支持垂直、左侧、对向三个相机
- 异步采集和状态监控
- 单元测试和集成测试

---

### 模块C: 图像处理层 (Image Processing Layer)
**负责人**: 成员C  
**工期**: 2周  
**优先级**: 中

#### 核心任务
```
src/infrastructure/imageprocessing/
├── OpenCVProcessor.h/cpp    # OpenCV处理器 [优先级: 高]
├── EdgeDetector.h/cpp       # 边缘检测 [优先级: 高]
└── ShapeDetector.h/cpp      # 形状检测 [优先级: 高]

src/application/services/
└── ImageProcessingService.h/cpp  # 图像处理应用服务 [优先级: 中]
```

#### 详细任务
1. **Week 1**: 
   - OpenCV 3.10集成和封装
   - 基础图像处理功能
   - 边缘检测算法实现

2. **Week 2**: 
   - 直线检测 (HoughLinesP)
   - 圆形检测 (HoughCircles)
   - 图像增强和滤波

#### 核心算法
- **边缘检测**: Canny算法，可调参数
- **直线检测**: HoughLinesP，支持最小长度、最大间隙
- **圆形检测**: HoughCircles，支持累加器阈值
- **图像增强**: 对比度、亮度、锐化

#### 交付物
- 完整的图像处理模块
- 与Python版本算法一致的检测效果
- 性能优化的处理流水线

---

### 模块D: 测量计算层 (Measurement Layer)
**负责人**: 成员D  
**工期**: 2周  
**优先级**: 高

#### 核心任务
```
src/domain/measurement/
├── entities/
│   ├── Point.h/cpp              # 点实体 [优先级: 高]
│   ├── Line.h/cpp               # 线实体 [优先级: 高]
│   ├── Circle.h/cpp             # 圆实体 [优先级: 高]
│   └── MeasurementResult.h/cpp  # 测量结果 [优先级: 高]
├── services/
│   ├── GeometryCalculator.h/cpp    # 几何计算 [优先级: 高]
│   └── MeasurementValidator.h/cpp  # 测量验证 [优先级: 中]
└── interfaces/
    └── IMeasurementCalculator.h    # 测量接口 [优先级: 高]
```

#### 详细任务
1. **Week 1**: 
   - 几何实体类设计和实现
   - 基础测量算法 (距离、角度)
   - 高级几何计算 (拟合、交点)

2. **Week 2**: 
   - 像素距离标定功能
   - 测量精度验证
   - 复合测量功能

#### 核心功能
```cpp
class GeometryCalculator {
public:
    // 基础测量
    double CalculateDistance(const Point& p1, const Point& p2);
    double CalculateAngle(const Line& l1, const Line& l2);
    
    // 几何拟合
    Circle FitCircle(const std::vector<Point>& points);
    Line FitLine(const std::vector<Point>& points);
    
    // 高级计算
    Point CalculateIntersection(const Line& l1, const Line& l2);
    double CalculatePointToLineDistance(const Point& p, const Line& l);
    double CalculateLineToCircleDistance(const Line& l, const Circle& c);
};
```

#### 交付物
- 完整的测量计算模块
- 支持所有Python版本的测量功能
- 像素标定和单位转换
- 高精度数值计算

---

### 模块E: 绘图管理层 (Drawing Management Layer)
**负责人**: 成员E  
**工期**: 2周  
**优先级**: 中

#### 核心任务
```
src/domain/drawing/
├── entities/
│   ├── DrawingObject.h/cpp      # 绘图对象 [优先级: 高]
│   ├── Layer.h/cpp              # 图层 [优先级: 高]
│   └── DrawingStyle.h/cpp       # 绘图样式 [优先级: 中]
├── services/
│   ├── DrawingManager.h/cpp     # 绘图管理 [优先级: 高]
│   └── LayerManager.h/cpp       # 图层管理 [优先级: 高]
└── interfaces/
    └── IDrawingRenderer.h       # 绘图渲染接口 [优先级: 高]
```

#### 详细任务
1. **Week 1**: 
   - 绘图对象实体设计
   - 图层管理系统
   - 基础绘制功能

2. **Week 2**: 
   - 交互式绘制功能
   - 撤销重做机制
   - 绘图性能优化

#### 核心功能
- **多图层管理**: 手动绘制层、自动检测层、网格层
- **绘图对象**: 点、线、圆、复合图形
- **交互功能**: 选择、拖拽、删除、编辑
- **渲染优化**: 脏区域更新、缓存机制

#### 交付物
- 完整的绘图管理模块
- 支持复杂图形的高效渲染
- 用户友好的交互体验

---

### 模块F: 界面表示层 (UI Presentation Layer)
**负责人**: 成员F  
**工期**: 3周  
**优先级**: 中

#### 核心任务
```
src/presentation/
├── mainwindow/
│   ├── MainWindow.h/cpp         # 主窗口 [优先级: 高]
│   ├── MainWindow.ui            # 主窗口UI [优先级: 高]
│   └── MainWindowController.h/cpp # 主窗口控制器 [优先级: 高]
├── dialogs/
│   ├── ParameterDialog.h/cpp    # 参数对话框 [优先级: 中]
│   ├── CalibrationDialog.h/cpp  # 标定对话框 [优先级: 中]
│   └── AboutDialog.h/cpp        # 关于对话框 [优先级: 低]
├── widgets/
│   ├── ImageView.h/cpp          # 图像显示控件 [优先级: 高]
│   ├── MeasurementPanel.h/cpp   # 测量面板 [优先级: 中]
│   ├── ToolBar.h/cpp            # 工具栏 [优先级: 中]
│   └── StatusDisplay.h/cpp      # 状态显示 [优先级: 低]
└── common/
    ├── ViewModels.h/cpp         # 视图模型 [优先级: 中]
    └── UIUtilities.h/cpp        # UI工具类 [优先级: 低]
```

#### 详细任务
1. **Week 1**: 
   - 主窗口界面设计和实现
   - 图像显示控件开发
   - 基础UI交互

2. **Week 2**: 
   - 测量工具面板
   - 参数配置对话框
   - 键盘快捷键支持

3. **Week 3**: 
   - UI美化和用户体验优化
   - 界面响应性调优
   - 辅助功能实现

#### 关键特性
- **多视图显示**: 主界面三视图 + 独立选项卡
- **实时交互**: 鼠标绘制、缩放、拖拽
- **状态同步**: 相机状态、测量结果实时更新
- **用户友好**: 直观的操作界面、丰富的提示信息

#### 交付物
- 完整的用户界面
- 与Python版本一致的操作体验
- 响应式设计和性能优化

---

## ⏱️ 开发时间线

### 第1周: 基础设施 + 架构搭建
```
成员A: ConfigManager, LogManager基础实现
成员B: 相机实体类设计, 接口定义
成员C: OpenCV集成配置
成员D: 几何实体类设计
成员E: 绘图对象实体设计
成员F: 主窗口界面设计
```

### 第2周: 核心模块开发
```
成员A: ThreadPool, 工具类实现
成员B: 海康SDK封装开始
成员C: 边缘检测, 基础图像处理
成员D: 基础测量算法实现
成员E: 图层管理, 基础绘制
成员F: 图像显示控件, 基础交互
```

### 第3周: 功能集成
```
成员A: 模块集成支持, 工具类完善
成员B: 相机控制器完成, 多相机管理
成员C: 形状检测算法, 处理服务层
成员D: 像素标定, 复合测量
成员E: 交互式绘制, 撤销重做
成员F: 测量面板, 参数对话框
```

### 第4周: 系统集成测试
```
全员: 模块集成, 接口对接, 功能测试
重点: 数据流验证, 性能测试, Bug修复
```

### 第5周: 性能优化 + 完善功能
```
全员: 性能优化, 内存管理, 异常处理
重点: 多线程优化, 算法调优, 稳定性测试
```

### 第6周: 测试 + 文档 + 部署
```
全员: 全面测试, 文档完善, 打包部署
重点: 用户测试, 性能基准, 发布准备
```

## 🔄 协作机制

### 接口优先原则
1. **第1周末**: 所有模块接口定义完成并评审
2. **接口冻结**: 第2周开始接口锁定,只能扩展不能修改
3. **Mock实现**: 使用Mock对象进行并行开发

### 集成策略
1. **每日构建**: 自动化构建和基础测试
2. **周集成**: 每周五进行模块集成测试
3. **持续集成**: 使用Git分支管理,Pull Request审查

### 沟通机制
1. **每日站会**: 15分钟进度同步
2. **技术评审**: 重要设计决策集体讨论
3. **代码审查**: 所有代码必须经过审查

## 📊 质量控制

### 代码规范
- **命名约定**: 统一的类名、函数名、变量名规范
- **注释要求**: 所有公共接口必须有详细注释
- **格式化**: 使用clang-format统一代码格式

### 测试策略
- **单元测试**: 每个类都要有单元测试,覆盖率>80%
- **集成测试**: 模块间接口测试
- **性能测试**: 关键路径性能基准测试

### 文档要求
- **API文档**: 自动生成的API参考文档
- **设计文档**: 模块设计和接口说明
- **用户文档**: 使用指南和故障排除

## 🚀 成功标准

### 功能完整性
- ✅ 支持Python版本的所有核心功能
- ✅ 三相机同步采集和显示
- ✅ 完整的测量工具集
- ✅ 自动检测算法
- ✅ 用户友好的界面

### 性能目标
- ✅ 图像采集帧率: ≥20fps
- ✅ 界面响应时间: <100ms
- ✅ 内存使用: <2GB
- ✅ 启动时间: <5秒

### 可维护性
- ✅ 模块化设计,单一职责
- ✅ 清晰的接口定义
- ✅ 完善的错误处理
- ✅ 充分的测试覆盖

---

这个开发计划确保了：
🎯 **明确分工**: 每个成员有清晰的职责和时间安排  
🔄 **有序协作**: 接口优先,并行开发,定期集成  
📈 **质量保证**: 代码规范,测试覆盖,性能监控  
⚡ **高效交付**: 6周内完成高质量的模块化C++重构 