#include "TrajectoryRecorder.h"
#include <QMutexLocker>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <cmath>

TrajectoryRecorder::TrajectoryRecorder(QObject* parent)
    : QObject(parent)
    , m_isRecording(false)
    , m_isPaused(false)
{
    qDebug() << "TrajectoryRecorder initialized";
}

TrajectoryRecorder::~TrajectoryRecorder()
{
    if (m_isRecording) {
        stopRecording();
    }
}

void TrajectoryRecorder::startRecording()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isRecording) {
        qWarning() << "Recording is already started";
        return;
    }
    
    m_isRecording = true;
    m_isPaused = false;
    
    // 清空之前的轨迹数据
    m_trajectory.clear();
    m_statistics = TrajectoryStatistics();
    m_statistics.startTime = QDateTime::currentDateTime();
    
    qDebug() << "Trajectory recording started";
    emit recordingStarted();
}

void TrajectoryRecorder::stopRecording()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isRecording) {
        qWarning() << "Recording is not started";
        return;
    }
    
    m_isRecording = false;
    m_isPaused = false;
    m_statistics.endTime = QDateTime::currentDateTime();
    
    calculateStatistics();
    
    qDebug() << "Trajectory recording stopped. Total points:" << m_trajectory.size();
    emit recordingStopped();
    emit statisticsUpdated(m_statistics);
}

void TrajectoryRecorder::pauseRecording()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isRecording || m_isPaused) {
        return;
    }
    
    m_isPaused = true;
    qDebug() << "Trajectory recording paused";
    emit recordingPaused();
}

void TrajectoryRecorder::resumeRecording()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isRecording || !m_isPaused) {
        return;
    }
    
    m_isPaused = false;
    qDebug() << "Trajectory recording resumed";
    emit recordingResumed();
}

bool TrajectoryRecorder::isRecording() const
{
    QMutexLocker locker(&m_mutex);
    return m_isRecording;
}

bool TrajectoryRecorder::isPaused() const
{
    QMutexLocker locker(&m_mutex);
    return m_isPaused;
}

void TrajectoryRecorder::recordPoint(double x, double y, double z, const QString& operation, double stepSize)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isRecording || m_isPaused) {
        return;
    }
    
    TrajectoryPoint point(x, y, z, operation, stepSize);
    m_trajectory.push_back(point);
    
    qDebug() << "Recorded trajectory point:" << operation << "at (" << x << "," << y << "," << z << ")";
    emit pointRecorded(point);
    
    // 实时更新统计信息
    updateStatistics();
}

void TrajectoryRecorder::recordMovement(const QString& direction, double stepSize, double currentX, double currentY, double currentZ)
{
    QString operation = QString("move_%1").arg(direction);
    recordPoint(currentX, currentY, currentZ, operation, stepSize);
}

void TrajectoryRecorder::clearTrajectory()
{
    QMutexLocker locker(&m_mutex);
    
    m_trajectory.clear();
    m_statistics = TrajectoryStatistics();
    
    qDebug() << "Trajectory cleared";
    emit trajectoryCleared();
    emit statisticsUpdated(m_statistics);
}

int TrajectoryRecorder::getPointCount() const
{
    QMutexLocker locker(&m_mutex);
    return static_cast<int>(m_trajectory.size());
}

std::vector<TrajectoryPoint> TrajectoryRecorder::getTrajectory() const
{
    QMutexLocker locker(&m_mutex);
    return m_trajectory;
}

TrajectoryStatistics TrajectoryRecorder::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    return m_statistics;
}

TrajectoryPoint TrajectoryRecorder::getLastPoint() const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_trajectory.empty()) {
        return TrajectoryPoint();
    }
    
    return m_trajectory.back();
}

std::vector<TrajectoryPoint> TrajectoryRecorder::getRecentPoints(int count) const
{
    QMutexLocker locker(&m_mutex);
    
    std::vector<TrajectoryPoint> recent;
    int size = static_cast<int>(m_trajectory.size());
    int start = std::max(0, size - count);
    
    for (int i = start; i < size; ++i) {
        recent.push_back(m_trajectory[i]);
    }
    
    return recent;
}

void TrajectoryRecorder::updateStatistics()
{
    calculateStatistics();
    emit statisticsUpdated(m_statistics);
}

void TrajectoryRecorder::calculateStatistics()
{
    if (m_trajectory.empty()) {
        m_statistics = TrajectoryStatistics();
        return;
    }
    
    m_statistics.totalPoints = static_cast<int>(m_trajectory.size());
    m_statistics.startTime = m_trajectory.front().timestamp;
    m_statistics.endTime = m_trajectory.back().timestamp;
    
    // 计算坐标范围
    m_statistics.minX = m_statistics.maxX = m_trajectory[0].x;
    m_statistics.minY = m_statistics.maxY = m_trajectory[0].y;
    m_statistics.minZ = m_statistics.maxZ = m_trajectory[0].z;
    
    m_statistics.totalDistance = 0.0;
    
    for (size_t i = 0; i < m_trajectory.size(); ++i) {
        const auto& point = m_trajectory[i];
        
        // 更新坐标范围
        m_statistics.minX = std::min(m_statistics.minX, point.x);
        m_statistics.maxX = std::max(m_statistics.maxX, point.x);
        m_statistics.minY = std::min(m_statistics.minY, point.y);
        m_statistics.maxY = std::max(m_statistics.maxY, point.y);
        m_statistics.minZ = std::min(m_statistics.minZ, point.z);
        m_statistics.maxZ = std::max(m_statistics.maxZ, point.z);
        
        // 计算累计距离
        if (i > 0) {
            m_statistics.totalDistance += calculateDistance(m_trajectory[i-1], point);
        }
    }
}

double TrajectoryRecorder::calculateDistance(const TrajectoryPoint& p1, const TrajectoryPoint& p2) const
{
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

bool TrajectoryRecorder::saveToFile(const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);

    QJsonObject rootObj;
    rootObj["version"] = "1.0";
    rootObj["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    rootObj["pointCount"] = static_cast<int>(m_trajectory.size());

    // 保存统计信息
    QJsonObject statsObj;
    statsObj["totalPoints"] = m_statistics.totalPoints;
    statsObj["totalDistance"] = m_statistics.totalDistance;
    statsObj["startTime"] = m_statistics.startTime.toString(Qt::ISODate);
    statsObj["endTime"] = m_statistics.endTime.toString(Qt::ISODate);
    statsObj["minX"] = m_statistics.minX;
    statsObj["maxX"] = m_statistics.maxX;
    statsObj["minY"] = m_statistics.minY;
    statsObj["maxY"] = m_statistics.maxY;
    statsObj["minZ"] = m_statistics.minZ;
    statsObj["maxZ"] = m_statistics.maxZ;
    rootObj["statistics"] = statsObj;

    // 保存轨迹点
    QJsonArray pointsArray;
    for (const auto& point : m_trajectory) {
        pointsArray.append(point.toJson());
    }
    rootObj["trajectory"] = pointsArray;

    QJsonDocument doc(rootObj);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Trajectory saved to:" << filePath;
    return true;
}

bool TrajectoryRecorder::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return false;
    }

    QMutexLocker locker(&m_mutex);

    QJsonObject rootObj = doc.object();

    // 加载轨迹点
    m_trajectory.clear();
    QJsonArray pointsArray = rootObj["trajectory"].toArray();

    for (const auto& value : pointsArray) {
        QJsonObject pointObj = value.toObject();
        m_trajectory.push_back(TrajectoryPoint::fromJson(pointObj));
    }

    // 重新计算统计信息
    calculateStatistics();

    qDebug() << "Trajectory loaded from:" << filePath << "Points:" << m_trajectory.size();
    emit statisticsUpdated(m_statistics);
    return true;
}

bool TrajectoryRecorder::exportToCsv(const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV file for writing:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // 写入CSV头部
    stream << "Timestamp,X(mm),Y(mm),Z(mm),Operation,StepSize(mm)\n";

    // 写入数据
    for (const auto& point : m_trajectory) {
        stream << point.timestamp.toString(Qt::ISODate) << ","
               << point.x << ","
               << point.y << ","
               << point.z << ","
               << point.operation << ","
               << point.stepSize << "\n";
    }

    file.close();

    qDebug() << "Trajectory exported to CSV:" << filePath;
    return true;
}
