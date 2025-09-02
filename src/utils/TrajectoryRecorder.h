#pragma once

#include <QObject>
#include <QDateTime>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <vector>
#include <memory>

/**
 * @brief 载物台轨迹点数据结构
 */
struct TrajectoryPoint {
    QDateTime timestamp;        // 时间戳
    double x;                  // X坐标 (mm)
    double y;                  // Y坐标 (mm)
    double z;                  // Z坐标 (mm)
    QString operation;         // 操作类型 ("move_x+", "move_y-", "move_z+", "home", "stop")
    double stepSize;           // 移动步长 (mm)
    
    TrajectoryPoint() : x(0), y(0), z(0), stepSize(0) {
        timestamp = QDateTime::currentDateTime();
    }
    
    TrajectoryPoint(double x_, double y_, double z_, const QString& op, double step = 0)
        : x(x_), y(y_), z(z_), operation(op), stepSize(step) {
        timestamp = QDateTime::currentDateTime();
    }
    
    // 转换为JSON对象
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["timestamp"] = timestamp.toString(Qt::ISODate);
        obj["x"] = x;
        obj["y"] = y;
        obj["z"] = z;
        obj["operation"] = operation;
        obj["stepSize"] = stepSize;
        return obj;
    }
    
    // 从JSON对象创建
    static TrajectoryPoint fromJson(const QJsonObject& obj) {
        TrajectoryPoint point;
        point.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
        point.x = obj["x"].toDouble();
        point.y = obj["y"].toDouble();
        point.z = obj["z"].toDouble();
        point.operation = obj["operation"].toString();
        point.stepSize = obj["stepSize"].toDouble();
        return point;
    }
};

/**
 * @brief 轨迹统计信息
 */
struct TrajectoryStatistics {
    int totalPoints;           // 总点数
    double totalDistance;      // 总移动距离
    QDateTime startTime;       // 开始时间
    QDateTime endTime;         // 结束时间
    double minX, maxX;         // X轴范围
    double minY, maxY;         // Y轴范围
    double minZ, maxZ;         // Z轴范围
    
    TrajectoryStatistics() : totalPoints(0), totalDistance(0.0),
        minX(0), maxX(0), minY(0), maxY(0), minZ(0), maxZ(0) {}
};

/**
 * @brief 载物台轨迹记录器
 * 负责记录、管理和导出载物台的运动轨迹
 */
class TrajectoryRecorder : public QObject {
    Q_OBJECT
    
public:
    explicit TrajectoryRecorder(QObject* parent = nullptr);
    ~TrajectoryRecorder();
    
    // 记录控制
    void startRecording();
    void stopRecording();
    void pauseRecording();
    void resumeRecording();
    bool isRecording() const;
    bool isPaused() const;
    
    // 轨迹记录
    void recordPoint(double x, double y, double z, const QString& operation, double stepSize = 0);
    void recordMovement(const QString& direction, double stepSize, double currentX, double currentY, double currentZ);
    
    // 数据管理
    void clearTrajectory();
    int getPointCount() const;
    std::vector<TrajectoryPoint> getTrajectory() const;
    TrajectoryStatistics getStatistics() const;
    
    // 文件操作
    bool saveToFile(const QString& filePath) const;
    bool loadFromFile(const QString& filePath);
    bool exportToCsv(const QString& filePath) const;
    
    // 数据访问
    TrajectoryPoint getLastPoint() const;
    std::vector<TrajectoryPoint> getRecentPoints(int count) const;

signals:
    void recordingStarted();
    void recordingStopped();
    void recordingPaused();
    void recordingResumed();
    void pointRecorded(const TrajectoryPoint& point);
    void trajectoryCleared();
    void statisticsUpdated(const TrajectoryStatistics& stats);

private slots:
    void updateStatistics();

private:
    mutable QMutex m_mutex;                    // 线程安全保护
    std::vector<TrajectoryPoint> m_trajectory; // 轨迹数据
    bool m_isRecording;                        // 是否正在记录
    bool m_isPaused;                          // 是否暂停
    TrajectoryStatistics m_statistics;        // 统计信息
    
    // 内部辅助函数
    void calculateStatistics();
    double calculateDistance(const TrajectoryPoint& p1, const TrajectoryPoint& p2) const;
};
