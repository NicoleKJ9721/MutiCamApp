# 常用模式和最佳实践

## 架构设计模式
- 相机SDK全局管理：HikvisionSDKManager单例类，引用计数机制，线程安全的资源管理
- 分层渲染模式：VideoDisplayWidget(显示层) + PaintingOverlay(交互层)，职责分离
- 异步处理模式：QtConcurrent::run + QProgressDialog + QFutureWatcher，避免UI阻塞

## 绘制功能最佳实践
- 动态缩放系统：qMax(minValue, baseValue * scaleFactor)模式，避免硬编码
- 字体大小计算：基于图像分辨率(imageHeight/66.67)，不依赖显示窗口大小
- 文本背景padding：fontSize*0.5动态调整，qMax(4.0, fontSize*0.5)保证最小值
- 预览逻辑：虚线预览，实线完成，统一使用calculateExtendedLine()避免绘制异常

## 事件处理经验
- 鼠标移动事件：避免重复条件判断，正确实现实时预览逻辑
