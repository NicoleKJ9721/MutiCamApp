# 开发规范和规则

- 项目重构规则：1. 将OpenCV和海康SDK复制到项目内部，不依赖系统安装 2. 不生成总结性Markdown文档 3. 不生成测试脚本 4. 不编译运行，用户自己处理 5. 实现自包含部署，便于在其他电脑运行
- 项目协作规则更新：1. 帮助生成总结性Markdown文档 2. 不生成测试脚本 3. 不编译项目，用户自己编译 4. 不运行程序，用户自己运行
- 项目协作规则更新：1. 生成总结性Markdown文档 2. 不生成测试脚本 3. 不编译项目，用户自己编译 4. 不运行程序，用户自己运行
- 项目协作规则更新：1.生成总结性Markdown文档 2.不生成测试脚本 3.不编译项目(用户自己编译) 4.不运行程序(用户自己运行) 5.完全C++重构，不使用Python代码，重写所有功能
- Qt定时器线程安全规则：在多线程环境中操作QTimer时，必须使用QMetaObject::invokeMethod()确保在正确线程中执行。已修复HikvisionCamera中的定时器线程安全问题。
- Qt高DPI显示修复：在main.cpp中添加QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true)和QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true)，在图像转换函数中设置pixmap.setDevicePixelRatio(qApp->devicePixelRatio())以确保高DPI屏幕下图像清晰显示。这些设置必须在QApplication创建之前调用。
- Qt高DPI显示问题重新修复方案：1. 用户反馈原方案只是缩小图像未真正修复；2. 新方案参考Python版本成功策略，在OpenCV层面进行精确缩放而非Qt层面；3. 修改displayImageOnLabel函数使用cv::resize进行预缩放；4. 修改matToQPixmap函数添加setDevicePixelRatio参数控制；5. 在图像显示时不设置devicePixelRatio避免过度缩放；6. 保持应用程序级别的Qt高DPI支持配置；7. 生成详细的修复总结文档06_Qt高DPI显示问题重新修复总结.md
- 平行线功能文字显示规则：只显示数值，不显示中文标签。距离显示格式为纯数字，角度显示格式为"数字°"，去掉"距离:"和"角度:"等中文标签，保持简洁的数据显示。
- 平行线功能常见错误修复记录：
1. 文本显示错误：使用QString("%.1f").arg()是错误语法，应该使用QString::number(value, 'f', 1)来格式化浮点数
2. 线条粗细不一致：平行线绘制的点半径应该与简单圆功能保持一致（8和10像素）
3. 字体大小不一致：平行线文本显示应该使用与简单圆相同的动态字体计算、字体类型（FONT_HERSHEY_DUPLEX）和文本背景绘制方式
4. 中线实现错误：C++版本的中线应该是位于两条平行线中间的平行线，而不是连接两条平行线中点的直线，需要与Python版本保持一致
- 平行线功能度数符号显示修复：

**问题描述：**
- 平行线功能中角度显示使用了"°"字符，在某些环境下会显示为问号
- 与项目中其他功能（如角度线）的度数符号实现方式不一致

**解决方案：**
- 将 `QString::number(parallel.angle, 'f', 1) + "°"` 修改为只显示数字部分
- 使用 `cv::circle()` 绘制空心圆环代替度数符号，与角度线功能保持一致
- 调整文本背景大小以容纳数字和圆环

**修复位置：**
- 文件：src/MutiCamApp.cpp
- 函数：drawSingleParallel
- 行数：约268行附近

**关键代码：**
```cpp
// 修复前
QString angleText = QString::number(parallel.angle, 'f', 1) + "°";

// 修复后
QString angleNumberText = QString::number(parallel.angle, 'f', 1);
// 使用cv::circle绘制空心圆环代替度数符号
cv::circle(image, circleCenter, circleRadius, cv::Scalar(0, 0, 0), circleThickness, cv::LINE_AA);
```

**注意事项：**
- 确保与其他功能（角度线、圆形测量等）的度数符号实现方式保持一致
- 避免直接使用特殊字符，优先使用图形绘制方式
- 考虑不同字符编码环境下的兼容性问题
- 当你感觉上下文不够用时，提醒用户新建对话
