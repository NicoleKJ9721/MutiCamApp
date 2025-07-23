# 开发规范和规则

- 项目重构规则：1. 将OpenCV和海康SDK复制到项目内部，不依赖系统安 2. 实现自包含部署，便于在其他电脑运行
- 项目协作规则更新：1.完全C++重构，不使用Python代码，重写所有功能
- Qt定时器线程安全规则：在多线程环境中操作QTimer时，必须使用QMetaObject::invokeMethod()确保在正确线程中执行。已修复HikvisionCamera中的定时器线程安全问题。
- 绘制文字显示规则：只显示数值，不显示中文标签。距离显示格式为纯数字，角度显示格式为"数字°"，去掉"距离:"和"角度:"等中文标签，保持简洁的数据显示。
- 平行线功能常见错误修复记录：
1. 文本显示错误：使用QString("%.1f").arg()是错误语法，应该使用QString::number(value, 'f', 1)来格式化浮点数
2. 线条粗细不一致：平行线绘制的点半径应该与简单圆功能保持一致
3. 字体大小不一致：平行线文本显示应该使用与简单圆相同的动态字体计算、字体类型（FONT_HERSHEY_DUPLEX）和文本背景绘制方式
4. 中线实现错误：C++版本的中线应该是位于两条平行线中间的平行线，而不是连接两条平行线中点的直线，需要与Python版本保持一致

**注意事项：**
- 确保与其他功能（角度线、圆形测量等）的度数符号实现方式保持一致
- 避免直接使用特殊字符，优先使用图形绘制方式
- 考虑不同字符编码环境下的兼容性问题
- 当你感觉上下文不够用时，提醒用户新建对话
- 每对话7次时提醒用户；编译前必须询问用户；不要生成总结性Markdown文档；不要生成测试脚本；不要编译，用户自己编译；不要运行，用户自己运行
- qDebug中文乱码修复规则：在Windows环境下，通过自定义Qt消息处理器解决qDebug输出中文乱码问题。在main.cpp中添加customMessageOutput函数，使用Windows API直接输出UTF-8编码文本，并通过qInstallMessageHandler安装处理器。此方案无需修改现有qDebug代码，统一处理所有Qt调试输出。
- 调试输出清理规则：删除频繁调用函数中的不必要qDebug输出，包括getPaintingOverlay、getCurrentFrame、updateVideoDisplayWidget、invalidateCache等函数。保留重要的错误和警告信息，但移除正常操作流程中的冗余日志，以减少控制台输出噪音，提高程序运行时的可读性。
- 撤销功能分离规则：将撤销功能分为两类：1) undoLastDrawing() 只撤销手动绘制操作；2) undoLastDetection() 只撤销自动检测操作。通过DrawingAction.Source枚举区分操作来源（ManualDrawing/AutoDetection）。UI中btnCan1StepDraw系列按钮撤销手动绘制，btnCan1StepDet系列按钮撤销自动检测。这样用户可以分别控制手动绘制和自动检测的撤销操作。
- 清空绘画功能分离规则：修改clearAllDrawings()方法只清空手动绘制的图形，保留自动检测结果。通过遍历历史记录识别手动绘制图形（source==ManualDrawing），使用removeItemsByIndices模板方法从后往前删除避免索引变化。同时清理历史记录中的手动绘制操作，保持选择状态一致性。这样"清空绘画"按钮不会意外删除用户的自动检测结果。
- 保存图像功能实现规则：实现完整的保存图像功能，包括主界面和各视图的保存按钮。保存原始图像（直接从getCurrentFrame获取）和可视化图像（通过renderVisualizedImage渲染绘制图形）。目录结构为D:/CamImage/年.月.日/视图名称/，文件命名为时-分-秒_origin.jpg和时-分-秒_visual.jpg。主界面保存按钮同时保存所有三个视图，各视图按钮只保存对应视图。
- 保存图像问题修复规则：解决cv::imwrite保存失败问题的方法：1)使用英文路径避免中文编码问题；2)检查图像数据连续性，非连续时使用clone()创建连续副本；3)添加QImage备用保存方法，当OpenCV保存失败时自动尝试QImage::save()；4)添加详细的调试信息包括数据连续性、步长等参数检查。这样可以解决相机图像保存的兼容性问题。
- 可视化图像渲染修复规则：1)恢复中文路径设置，保持原有的目录结构；2)在PaintingOverlay中添加renderToImage()公共方法，提取paintEvent中的绘制逻辑；3)renderToImage方法临时设置1:1映射变换参数，绘制所有图形元素后恢复原始参数；4)在MutiCamApp::renderVisualizedImage中调用overlay->renderToImage()来正确渲染绘制内容到图像上。这样保存的visual图像就包含了所有绘画和测量痕迹。
- 段错误修复规则：在QImage构造时避免引用临时cv::Mat数据导致的段错误。解决方法：1)在所有QImage构造后立即调用.copy()方法创建独立数据副本；2)在cv::Mat转QImage再转回cv::Mat时确保使用.clone()创建独立副本；3)添加try-catch捕获所有异常类型包括未知异常；4)检查QImage格式确保支持的格式才进行转换。这样避免了内存访问错误和程序崩溃。
- OpenCV保存失败诊断规则：当cv::imwrite保存失败时的诊断方法：1)检查图像类型、深度、通道数和数据指针有效性；2)使用英文路径测试OpenCV功能是否正常，确定是否为中文路径问题；3)输出OpenCV版本信息和JPEG编码器支持状态；4)创建测试图像验证编码器功能；5)提供QImage作为备用保存方法确保功能可用。这样可以快速定位OpenCV保存失败的具体原因。
- Python vs C++ OpenCV中文路径差异分析：Python版本能保存中文路径而C++版本不行的原因：1)Python的OpenCV通常使用预编译wheel包，包含更完整编码器支持；2)Python字符串默认Unicode，OpenCV-Python可能有额外路径编码转换层；3)C++中QString::toStdString()在Windows下可能产生错误编码；4)需要测试toUtf8()和toLocal8Bit()等不同编码方式；5)Python绑定可能有专门的Windows中文路径workaround。解决方案是使用QImage作为备用保存方法。
- OpenCV中文路径编码修复规则：在Windows下OpenCV处理中文路径的正确方法是使用toLocal8Bit()编码。将所有cv::imwrite调用中的QString路径从toStdString()改为toLocal8Bit().toStdString()。测试证明：toStdString()和toUtf8()都失败，只有toLocal8Bit()成功。这是因为Windows系统的本地编码页(ANSI)与OpenCV期望的编码方式匹配。修复后OpenCV和QImage都能正常保存中文路径。
- 主界面保存图像overlay选择修复规则：主界面保存图像时文字大小不正常的根本原因是使用了错误的overlay。主界面保存应该使用主界面的overlay(m_verticalPaintingOverlay等)，而不是标签页的overlay(m_verticalPaintingOverlay2等)。标签页overlay可能有不同的缩放状态，导致字体大小计算错误。修复：将saveImages方法中的overlay选择从标签页overlay改为主界面overlay。这样确保主界面保存的可视化图像字体大小与主界面显示一致。
- 异步保存图像优化规则：解决保存图像时界面卡顿问题的方法：1)使用QtConcurrent::run在后台线程执行保存操作；2)添加QProgressDialog显示保存进度和取消功能；3)使用QFutureWatcher监控异步任务完成；4)在保存期间禁用保存按钮防止重复点击；5)通过QMetaObject::invokeMethod线程安全地更新UI进度；6)保存完成后重新启用按钮并显示完成提示。这样确保大图像保存不会阻塞主线程，用户界面保持响应。
- QtConcurrent模块配置规则：使用QtConcurrent进行异步操作时需要在CMakeLists.txt中正确配置：1)在find_package中添加Concurrent组件：find_package(Qt6 COMPONENTS Widgets Concurrent REQUIRED)；2)在target_link_libraries中添加Qt6::Concurrent链接：target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Concurrent)。这样才能正确编译使用QtConcurrent::run等异步功能的代码。
- Qt槽函数声明规则：在Qt中使用信号槽机制时，槽函数必须在头文件的private slots:或public slots:部分声明，不能在普通的private:部分声明。例如onAsyncSaveFinished()这样的回调函数需要连接到QFutureWatcher的finished信号，必须声明为槽函数。正确的做法是在private slots:部分添加void onAsyncSaveFinished();声明。
- 选项卡异步保存优化规则：将选项卡的单个视图保存按钮也改为异步保存，保持与主界面一致的用户体验。优化内容：1)修改onSaveImageVerticalClicked等方法使用saveImagesAsync；2)优化saveImagesAsync支持单个视图保存，显示对应的进度文本；3)根据保存视图类型禁用对应按钮，避免重复点击；4)保存完成后重新启用所有保存按钮。这样确保所有保存操作都是异步的，界面不会卡顿。
- 图像保存画质优化规则：将所有图像保存的JPEG质量设置为100%最高画质。修改内容：1)OpenCV保存使用compression_params设置IMWRITE_JPEG_QUALITY为100；2)QImage备用保存的质量参数从90改为100；3)包括原始图像、可视化图像和测试图像的所有保存操作；4)添加调试信息显示使用的画质设置。这样确保保存的图像具有最高的画质，无损失地保留原始图像的细节。
