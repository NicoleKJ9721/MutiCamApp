#include "MatchingController.h"
#include "third_party/json.hpp" // 假设json库在这里
#include <iostream>
#include <fstream>

using json = nlohmann::json;

MatchingController::MatchingController() : 
    kcg_matcher_(nullptr) {
    std::cout << "[INFO] MatchingController created." << std::endl;
}

MatchingController::~MatchingController() {
    delete kcg_matcher_;
}

bool MatchingController::initialize(const std::string& config_path) {
    // 1. 加载配置
    if (!loadConfigFromFile(config_path)) {
        return false;
    }

    // 2. 根据配置加载模型
    std::lock_guard<std::mutex> lock(mtx_);
    if (kcg_matcher_) {
        delete kcg_matcher_;
    }
    try {   
        std::string class_name_to_load = config_data_["Model"].value("ClassName", "default_model");     
        std::cout << "[INFO] Initializing with model: " << class_name_to_load << std::endl;
        kcg_matcher_ = new kcg::KcgMatch(model_save_path_, class_name_to_load); 

        // c. 【最关键的一步】调用 LoadModel()，让对象从文件中加载数据
        std::cout << "[INFO] Calling LoadModel()..." << std::endl;
        kcg_matcher_->LoadModel();       
        std::cout << "[SUCCESS] MatchingController initialized successfully." << std::endl;
    } catch (const std::exception& e) {
        // 如果 new, LoadModel, 或 json 解析的任何一步失败，都会在这里被捕获
        std::cerr << "[FATAL] An exception occurred during model loading: " << e.what() << std::endl;

        // 确保清理掉可能部分创建的资源
        delete kcg_matcher_;
        kcg_matcher_ = nullptr;
        
        return false; // 返回失败
    }
    return true;
}

// In MatchingController.cpp

bool MatchingController::reloadConfiguration() {
    std::string current_config_path;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        current_config_path = this->config_file_path_;
    }

    return initialize(current_config_path);
}

// 这是新的核心私有函数
bool MatchingController::loadConfigFromFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::cout << "[INFO] Loading configuration from: " << path << std::endl;
    
    try {
        std::ifstream f(path);
        if (!f.is_open()) {
             std::cerr << "[ERROR] Cannot open config file: " << path << std::endl;
             return false;
        }
        json config;
        this->config_data_ = json::parse(f, nullptr, true, true);
        this->config_file_path_ = path;
    } catch (json::parse_error& e) {
        std::cerr << "[ERROR] Failed to parse config.jsonc: " << e.what() << std::endl;
        return false;
    }

    std::filesystem::path config_dir = std::filesystem::path(path).parent_path();
    const auto& path_config = this->config_data_.value("Paths", json::object());
    std::string relative_model_path = path_config.value("ModelRootPath", "models");
    this->model_save_path_ = (config_dir / relative_model_path).string();
    std::cout << "[DEBUG] Resolved ModelRootPath to absolute path: " << this->model_save_path_ << std::endl;

    // --- 开始解析JSON并填充所有成员变量 ---
    // 使用 .value("key", default_value) 来安全地获取值
    const auto& matching_config = this->config_data_.value("Matching", json::object());
    this->score_thresh_           = matching_config.value("ScoreThreshold", 0.8f);
    this->overlap_                = matching_config.value("Overlap", 0.4f);
    this->mag_thresh_             = matching_config.value("MinMagThreshold", 20.0f);
    this->greediness_             = matching_config.value("Greediness", 0.8f);
    this->pyrd_level_             = static_cast<kcg::PyramidLevel>(matching_config.value("PyramidLevel", 3));
    this->T_                      = matching_config.value("T", 2);
    this->top_k_                  = matching_config.value("TopK", 10);
    this->strategy_               = static_cast<kcg::MatchingStrategy>(matching_config.value("Strategy", 0));
    this->fixed_angle_window_     = matching_config.value("FixedAngleWindow", 10.0f);
    this->refinement_search_mode_ = matching_config.value("RefinementSearchMode", "pyrd");
    this->scale_search_window_    = matching_config.value("ScaleSearchWindow", 0.1f);

    const auto& template_config = this->config_data_.value("TemplateMaking", json::object());
    this->angle_range_.begin = template_config.value("AngleStart", -180.0f);
    this->angle_range_.end   = template_config.value("AngleEnd", 180.0f);
    this->angle_range_.step  = template_config.value("AngleStep", 10.0f);
    this->scale_range_.begin = template_config.value("ScaleStart", 0.7f);
    this->scale_range_.end   = template_config.value("ScaleEnd", 1.3f);
    this->scale_range_.step  = template_config.value("ScaleStep", 0.05f);

    this->num_features_      = template_config.value("NumFeatures", 0);
    this->weak_thresh_       = template_config.value("WeakThreshold", 30.0f);
    this->strong_thresh_     = template_config.value("StrongThreshold", 60.0f);
    
    std::cout << "[INFO] Configuration loaded successfully." << std::endl;
    return true;
}

std::vector<MatchResult> MatchingController::formatResults(const std::vector<kcg::Match>& raw_matches) {
    std::vector<MatchResult> enriched_results;
    
    // 这里是所有复杂的数据转换和计算逻辑
    for (const auto& raw_match : raw_matches) {
        MatchResult result;

        // a. 从KcgMatch获取模板和基础框信息 (需要新增接口)
        const kcg::Template& t = kcg_matcher_->getTemplate(8, raw_match.template_id);
        const cv::Rect& base_box = kcg_matcher_->getBaseTemplateBox();

        // 如果获取失败，则跳过
        if (t.is_valid == 0) continue;
        
        // b. 填充基本信息
        result.score = raw_match.similarity;
        result.angle = -t.shape_info.angle; //未知原因，这里需要取反
        result.scale = t.shape_info.scale;
        
        // c. 计算非精确的轴对齐框 (全图坐标)
        result.axis_aligned_box = cv::Rect(raw_match.x, raw_match.y, t.w, t.h);

        // d. 计算精确的旋转框 (核心计算逻辑)
        cv::Size2f rect_size(base_box.width * result.scale, base_box.height * result.scale);
        cv::Point2f rect_center(raw_match.x + t.w / 2.0f, raw_match.y + t.h / 2.0f);
        result.rotated_box = cv::RotatedRect(rect_center, rect_size, result.angle); // 注意：OpenCV中角度通常是负的

        // e. 计算精确的4个角点
        cv::Point2f vertices[4];
        result.rotated_box.points(vertices);
        result.corners.assign(vertices, vertices + 4);

        // f. 将丰富后的结果存入列表
        enriched_results.push_back(result);
    }
    
    return enriched_results;
}


// --- 现在，核心函数的签名变得极其干净！ ---

std::vector<MatchResult> MatchingController::processSingleFrame(const cv::Mat& frame) {
    // 加锁，复制需要的参数，然后立即解锁
    mtx_.lock();
    // 1. 检查匹配器是否存在
    if (!kcg_matcher_) {
        mtx_.unlock();
        return {};
    }
    cv::Rect roi = current_roi_;

    float score                        = this->score_thresh_;
    float overlap                      = this->overlap_;
    float mag_thresh                   = this->mag_thresh_;
    float greediness                   = this->greediness_;
    int T                              = this->T_;
    int top_k                          = this->top_k_;
    kcg::PyramidLevel pyrd_level       = this->pyrd_level_;
    kcg::MatchingStrategy strategy     = this->strategy_;
    float fixed_angle_window           = this->fixed_angle_window_;
    std::string refinement_search_mode = this->refinement_search_mode_;
    float scale_search_window          = this->scale_search_window_;

    mtx_.unlock();

    // 2. 准备图像数据
    cv::Mat gray_image;
    cv::Mat image_to_process;

    // 保证ROI在图像的有效范围内
    roi &= cv::Rect(0, 0, frame.cols, frame.rows); 
    
    if (roi.width <= 0 || roi.height <= 0) {
        image_to_process = frame;
        roi = cv::Rect(0, 0, frame.cols, frame.rows); // 如果ROI无效, 则重置为全图
    } else {
        image_to_process = frame(roi);
    }
    
    if (image_to_process.channels() > 1) {
        cv::cvtColor(image_to_process, gray_image, cv::COLOR_BGR2GRAY);
    } else {
        gray_image = image_to_process;
    }

    // 3. 执行核心匹配算法
    std::vector<kcg::Match> matches = kcg_matcher_->Matching(
        gray_image,
        score,
        overlap,
        mag_thresh,
        greediness,
        pyrd_level,
        T,
        top_k,
        strategy,
        refinement_search_mode,
        fixed_angle_window,
        scale_search_window
    );

    // 4. 坐标转换：将ROI内部坐标转换回全图坐标
    if (roi.x != 0 || roi.y != 0) {
        for (auto& match : matches) {
            match.x += roi.x;
            match.y += roi.y;
        }
    }

    return formatResults(matches);
}

std::future<bool> MatchingController::createTemplateAsync(
    const cv::Mat& sourceImage, 
    const cv::Rect& templateROI, 
    const std::string& new_class_name) 
{
    // 加锁，复制所有模板创建参数
    mtx_.lock();

    std::string model_save_path  = this->model_save_path_;
    kcg::AngleRange angle_range  = this->angle_range_;
    kcg::ScaleRange scale_range  = this->scale_range_;
    int num_features             = this->num_features_;
    float weak_thresh            = this->weak_thresh_;
    float strong_thresh          = this->strong_thresh_;

    mtx_.unlock();

    // 异步逻辑和之前一样，只是把复制出来的params传给lambda
    return std::async(std::launch::async, [=]() -> bool {
        try {
            kcg::KcgMatch template_creator(
                model_save_path, 
                new_class_name
            );
            cv::Mat template_image = sourceImage(templateROI).clone();
            template_creator.MakingTemplates(
                template_image,
                angle_range,
                scale_range,
                num_features,
                weak_thresh,
                strong_thresh
            );
            return true;
        } catch (const cv::Exception& e) { // 优先捕获OpenCV的异常
            std::cerr << "\n\n[FATAL-THREAD] OpenCV Exception during template creation: \n" 
                      << e.what() << std::endl;
            fflush(stderr); // 强制刷新标准错误输出流
            return false;
        } catch (const std::exception& e) { // 再捕获标准异常
            std::cerr << "\n\n[FATAL-THREAD] Standard Exception during template creation: \n" 
                      << e.what() << std::endl;
            fflush(stderr);
            return false;
        } catch (...) { // 最后捕获所有未知异常
            std::cerr << "\n\n[FATAL-THREAD] An unknown error occurred during template creation!" << std::endl;
            fflush(stderr);
            return false;
        }
    });
}

// setROI 函数不变
void MatchingController::setROI(const cv::Rect& roi) {
    std::lock_guard<std::mutex> lock(mtx_);
    current_roi_ = roi;
}


// ==========================================================
//      MatchingController.cpp -> setParameter (最终版)
// ==========================================================

void MatchingController::setParameter(const std::string& name, const json& value) {
    // 加锁以保护所有成员变量的读写，保证线程安全
    std::lock_guard<std::mutex> lock(mtx_);

    try {
        // --- 匹配参数 (Matching) ---
        if (name == "ScoreThreshold") {
            score_thresh_ = value.get<float>();
            config_data_["Matching"]["ScoreThreshold"] = score_thresh_;
        } 
        else if (name == "Overlap") {
            overlap_ = value.get<float>();
            config_data_["Matching"]["Overlap"] = overlap_;
        }
        else if (name == "MinMagThreshold") {
            mag_thresh_ = value.get<float>();
            config_data_["Matching"]["MinMagThreshold"] = mag_thresh_;
        }
        else if (name == "Greediness") {
            greediness_ = value.get<float>();
            config_data_["Matching"]["Greediness"] = greediness_;
        }
        else if (name == "T") {
            T_ = value.get<int>();
            config_data_["Matching"]["T"] = T_;
        }
        else if (name == "TopK") {
            top_k_ = value.get<int>();
            config_data_["Matching"]["TopK"] = top_k_;
        }
        else if (name == "PyramidLevel") {
            pyrd_level_ = static_cast<kcg::PyramidLevel>(value.get<int>());
            config_data_["Matching"]["PyramidLevel"] = value.get<int>();
        }
        else if (name == "Strategy") {
            strategy_ = static_cast<kcg::MatchingStrategy>(value.get<int>());
            config_data_["Matching"]["Strategy"] = value.get<int>();
        }
        else if (name == "RefinementSearchMode") {
            refinement_search_mode_ = value.get<std::string>();
            config_data_["Matching"]["RefinementSearchMode"] = refinement_search_mode_;
        }
        else if (name == "FixedAngleWindow") {
            fixed_angle_window_ = value.get<float>();
            config_data_["Matching"]["FixedAngleWindow"] = fixed_angle_window_;
        }
        else if (name == "ScaleSearchWindow") {
            scale_search_window_ = value.get<float>();
            config_data_["Matching"]["ScaleSearchWindow"] = scale_search_window_;
        }
        
        // --- 模板制作参数 (TemplateMaking) ---
        else if (name == "AngleStart") {
            angle_range_.begin = value.get<float>();
            config_data_["TemplateMaking"]["AngleStart"] = angle_range_.begin;
        }
        else if (name == "AngleEnd") {
            angle_range_.end = value.get<float>();
            config_data_["TemplateMaking"]["AngleEnd"] = angle_range_.end;
        }
        else if (name == "AngleStep") {
            angle_range_.step = value.get<float>();
            config_data_["TemplateMaking"]["AngleStep"] = angle_range_.step;
        }
        else if (name == "ScaleStart") {
            scale_range_.begin = value.get<float>();
            config_data_["TemplateMaking"]["ScaleStart"] = scale_range_.begin;
        }
        else if (name == "ScaleEnd") {
            scale_range_.end = value.get<float>();
            config_data_["TemplateMaking"]["ScaleEnd"] = scale_range_.end;
        }
        else if (name == "ScaleStep") {
            scale_range_.step = value.get<float>();
            config_data_["TemplateMaking"]["ScaleStep"] = scale_range_.step;
        }
        else if (name == "NumFeatures") {
            num_features_ = value.get<int>();
            config_data_["TemplateMaking"]["NumFeatures"] = num_features_;
        }
        else if (name == "WeakThreshold") {
            weak_thresh_ = value.get<float>();
            config_data_["TemplateMaking"]["WeakThreshold"] = weak_thresh_;
        }
        else if (name == "StrongThreshold") {
            strong_thresh_ = value.get<float>();
            config_data_["TemplateMaking"]["StrongThreshold"] = strong_thresh_;
        }
        
        // --- 如果没有匹配到任何已知参数 ---
        else {
            std::cerr << "[WARNING] setParameter: Unknown or unhandled parameter name '" << name << "'" << std::endl;
            return; // 直接返回，不打印成功信息
        }

        std::cout << "[INFO] Parameter '" << name << "' updated to: " << value.dump() << std::endl;

    } catch (const json::exception& e) {
        // 捕获json在 .get<T>() 时可能发生的类型转换错误
        std::cerr << "[ERROR] Failed to set parameter '" << name << "' with value " << value.dump() 
                  << ". Reason: " << e.what() << std::endl;
    }
}

// ==========================================================
//    MatchingController.cpp -> saveConfiguration (最终版)
// ==========================================================

bool MatchingController::saveConfiguration() {
    std::lock_guard<std::mutex> lock(mtx_);

    if (config_file_path_.empty()) {
        std::cerr << "[ERROR] Cannot save configuration: Configuration file path is not set. "
                  << "Please initialize the controller first." << std::endl;
        return false;
    }

    std::cout << "[INFO] Saving current configuration to: " << config_file_path_ << std::endl;

    // 3. 使用 ofstream 将内存中的JSON对象写入文件。
    try {
        std::ofstream f_out(config_file_path_);
        
        // 检查文件是否成功打开以供写入
        if (!f_out.is_open()) {
            std::cerr << "[ERROR] Failed to open file for writing: " << config_file_path_ << std::endl;
            return false;
        }
        
        f_out << std::setw(4) << config_data_ << std::endl;
        
        f_out.close();

    } catch (const std::exception& e) {
        // 捕获在文件写入过程中可能发生的任何标准库异常
        std::cerr << "[ERROR] An exception occurred during file saving: " << e.what() << std::endl;
        return false;
    }

    std::cout << "[SUCCESS] Configuration saved successfully." << std::endl;
    return true;
}


