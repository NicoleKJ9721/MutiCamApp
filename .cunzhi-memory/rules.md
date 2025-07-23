# 开发规范和规则

## 核心项目规则
- 项目重构规则：完全C++重构，自包含部署，OpenCV和海康SDK内置
- 工作流程规则：专注代码实现，不生成文档/测试/编译/运行，用户自己处理
- 寸止交互规则：每个重要步骤完成后必须调用寸止MCP获取用户反馈，根据反馈调整后续行为

## 技术实现规则
- Qt定时器线程安全规则：在多线程环境中操作QTimer时，必须使用QMetaObject::invokeMethod()确保在正确线程中执行
- 绘制文字显示规则：只显示数值，不显示中文标签。距离显示格式为纯数字，角度显示格式为"数字°"，保持简洁的数据显示
- 字符编码兼容性：避免直接使用特殊字符，优先使用图形绘制方式，考虑不同字符编码环境下的兼容性问题
## 调试和功能管理
- qDebug中文乱码修复规则：在Windows环境下，通过自定义Qt消息处理器解决qDebug输出中文乱码问题
- 撤销功能分离规则：将撤销功能分为手动绘制和自动检测两类，通过DrawingAction.Source枚举区分操作来源
- 清空绘画功能分离规则：clearAllDrawings()方法只清空手动绘制的图形，保留自动检测结果
## 保存图像功能
- 保存图像功能实现规则：保存原始图像和可视化图像，目录结构为D:/CamImage/年.月.日/视图名称/，文件命名为时-分-秒_origin.jpg和时-分-秒_visual.jpg
- OpenCV中文路径编码修复规则：在Windows下使用toLocal8Bit()编码处理中文路径，添加QImage备用保存方法
- 段错误修复规则：QImage构造后立即调用.copy()方法创建独立数据副本，避免引用临时cv::Mat数据
- 可视化图像渲染修复规则：在PaintingOverlay中添加renderToImage()方法，正确渲染绘制内容到图像
- 主界面保存overlay选择修复规则：主界面保存应该使用主界面的overlay，而不是标签页的overlay
## 异步保存优化
- 异步保存图像优化规则：使用QtConcurrent::run在后台线程执行保存操作，添加QProgressDialog显示进度，确保界面不卡顿
- QtConcurrent模块配置规则：在CMakeLists.txt中添加Concurrent组件和Qt6::Concurrent链接
- Qt槽函数声明规则：槽函数必须在private slots:或public slots:部分声明
- 图像保存画质优化规则：所有图像保存的JPEG质量设置为100%最高画质
## 字体和显示设置
- 字体大小最终设置规则：使用图像高度的1.5%作为字体大小，计算公式：imageHeight / 66.67，范围限制为14-50像素，在可读性和美观性之间达到平衡
- 文字背景padding最终设置规则：使用fontSize*0.5作为padding，保持与字体大小的比例关系，使用qMax(4.0, fontSize*0.5)确保最小4像素padding
