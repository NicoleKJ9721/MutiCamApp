# 常用模式和最佳实践

- 相机SDK全局管理和引用计数解决方案：通过HikvisionSDKManager单例类实现SDK的全局管理，使用引用计数机制确保SDK在所有相机释放后才Finalize，解决了相机资源未释放导致下次启动失败的问题。关键技术：单例模式、引用计数、线程安全、强制释放机制。已创建hikvision_sdk_manager.h/.cpp文件，修改了HikvisionCamera类集成SDK管理器，在MutiCamApp析构函数中添加强制释放调用。
- 平行线功能文字显示优化：参考简单圆功能的行距设置，使用 angleSize.height + 10 作为行距，避免文字上下两行重叠问题。同时调整背景框高度计算以匹配新的行距。
- 直线功能修复经验：在鼠标移动事件处理中，避免重复的条件判断。正确的实时预览逻辑应该是：第一次移动时添加第二个点，后续移动时更新第二个点位置。重复条件判断会导致else分支永远不执行，造成功能异常。
- drawSingleTwoLines函数重构最佳实践：1.动态缩放系统-使用qMax(minValue, baseValue * scaleFactor)模式替代所有硬编码尺寸；2.统一视觉风格-点序号样式与drawSingleParallel一致，线条粗细使用thickness * 2.0 * scaleFactor逻辑；3.文本布局优化-使用drawTextWithBackground辅助函数，正确处理锚点和偏移，实现角度和坐标文本的上下紧挨布局；4.字符串格式化-统一使用QString::asprintf()替代有Bug的%.1f格式；5.预览逻辑-第一条线和第二条线都支持虚线预览状态；6.代码结构-与其他重构函数保持一致的绘制流程和注释风格
- 1.直线预览虚线问题通过统一使用calculateExtendedLine()解决，避免过度延伸导致Qt绘制异常 2.预览状态使用虚线(isCurrentDrawing=true)完成状态使用实线
