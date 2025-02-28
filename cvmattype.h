#ifndef CVMATTYPE_H
#define CVMATTYPE_H

#include <QMetaType>
#include <opencv2/opencv.hpp>

// 注册cv::Mat类型到Qt元对象系统
Q_DECLARE_METATYPE(cv::Mat)

#endif // CVMATTYPE_H 