#pragma once

#include "KcgMatch.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <mutex>
#include <future>
#include "third_party/json.hpp"

/**
 * @struct EnrichedMatchResult
 * @brief  一个包含了所有前端可能需要的、丰富的匹配结果信息。
 *         这是对外暴露的【标准结果格式】。
 */
struct MatchResult {
    // 核心匹配信息
    float score;
    float angle;
    float scale;

    // 边界框信息
    cv::Rect axis_aligned_box;          // 轴对齐的、非精确的外接矩形
    cv::RotatedRect rotated_box;        // 精确的旋转矩形 (包含中心点、尺寸、角度)
    std::vector<cv::Point2f> corners;   // 精确的4个角点坐标
};


class MatchingController {
public:
    MatchingController();
    ~MatchingController();

    /**
     * @brief 初始化控制器，加载配置文件和指定的模型。
     * @param config_path 指向 config.jsonc 文件的完整路径。
     * @return true 如果初始化成功, false 如果失败。
     */
    bool initialize(const std::string& config_path);
    
    /**
     * @brief 将当前内存中的配置参数保存回它最初加载的那个JSON文件。
     * @return true 如果保存成功, false 如果失败。
     */
    bool saveConfiguration();

    /**
     * @brief 重新加载配置文件，并根据新配置重建内部匹配器。
     * @return true 如果加载和重建成功, false 如果失败。
     */
    bool reloadConfiguration();

    /**
     * @brief 对输入的单帧图像进行模板匹配。函数会使用内部加载的最新参数。
     * @param frame 从相机获取的原始视频帧 (BGR彩色格式)。
     * @return 返回一个包含所有匹配结果的向量。
     */
    std::vector<MatchResult> processSingleFrame(const cv::Mat& frame);

    /**
     * @brief 异步地创建一个新的模板文件。它会使用配置文件中 "TemplateMaking" 部分的参数。
     * @param sourceImage 用于截取模板的原始大图。
     * @param templateROI 用户在UI上绘制的、定义了模板区域的矩形。
     * @param new_class_name 要创建的新模板的名称。
     * @return std::future<bool> - 用于查询任务状态和结果。
     */
    std::future<bool> createTemplateAsync(const cv::Mat& sourceImage, const cv::Rect& templateROI, const std::string& new_class_name);

    /**
     * @brief 设置或更新用于匹配的感兴趣区域 (ROI)。
     * @param roi 一个cv::Rect对象，坐标基于全尺寸视频帧。
     */
    void setROI(const cv::Rect& roi);

    /**
     * @brief 动态修改一个内存中的浮点型匹配参数。
     * @param param_name 参数名 (如 "ScoreThreshold", "Overlap", "Greediness")。
     * @param value 新的参数值。
     */
    void setParameter(const std::string& param_name, const nlohmann::json& value);

private:
    // 私有辅助函数，用于加载和解析JSON配置
    bool loadConfigFromFile(const std::string& path);
    std::vector<MatchResult> formatResults(const std::vector<kcg::Match>& raw_matches);

    // 核心算法对象指针
    kcg::KcgMatch* kcg_matcher_;
    std::string config_file_path_;
    nlohmann::json config_data_;             // 在内存中完整地持有整个JSON配置

    // 线程安全保护
    std::mutex mtx_;

    // --- 所有参数现在都是内部成员变量 ---
    cv::Rect current_roi_;

    // --- 匹配参数 ---
    float score_thresh_;
    float overlap_;
    float mag_thresh_;
    float greediness_;
    int T_;
    int top_k_;
    kcg::PyramidLevel pyrd_level_;
    kcg::MatchingStrategy strategy_;
    float fixed_angle_window_;
    std::string refinement_search_mode_;
    float scale_search_window_;

    // 模板制作参数
    std::string model_save_path_;
    kcg::AngleRange angle_range_;
    kcg::ScaleRange scale_range_;
    int num_features_;
    float weak_thresh_;
    float strong_thresh_;
};