// #include <iostream>
// #include <chrono>
// #include <string>
// #include <fstream>
// #include <filesystem>
// #include "third_party/json.hpp"
// #include "KcgMatch.h"
// #include "PathHelpers.h"

// namespace fs = std::filesystem;
// using namespace std;
// using namespace cv;
// using namespace kcg;
// using json = nlohmann::json;

// void CreateModel(KcgMatch &kcg_match, const json& config, const fs::path& exe_dir)
// {
//     int num_features = 100;
//     float weak_thresh = 30.0f;
//     float strong_thresh = 60.0f;
//     AngleRange angle_range(-180.f, 180.f, 10.f);
//     ScaleRange scale_range(0.7f, 1.3f, 0.05f);
//     fs::path template_path = exe_dir / "template" / "template.png";

//     if (config.contains("TemplateMaking")) {
//         const auto& tm_config = config["TemplateMaking"];
//         num_features = tm_config.value("NumFeatures", 100);
//         weak_thresh = tm_config.value("WeakThreshold", 30.0f);
//         strong_thresh = tm_config.value("StrongThreshold", 60.0f);
//         angle_range.begin = tm_config.value("AngleStart", -180.0f);
//         angle_range.end = tm_config.value("AngleEnd", 180.0f);
//         angle_range.step = tm_config.value("AngleStep", 10.0f);
//         scale_range.begin = tm_config.value("ScaleStart", 0.7f);
//         scale_range.end = tm_config.value("ScaleEnd", 1.3f);
//         scale_range.step = tm_config.value("ScaleStep", 0.05f);
//     }

//     if (config.contains("Paths") && config["Paths"].contains("TemplateImage")) {
//         template_path = exe_dir / fs::path(config["Paths"].value("TemplateImage", "template/template.png"));
//     }

//     Mat model = imread(template_path.string());
//     if (model.empty())
//     {
//         cout << "Error: read model image '" << template_path.string() << "' failed." << endl;
//         return;
//     }
    
//     if (model.channels() > 1) {
// 		cvtColor(model, model, COLOR_BGR2GRAY);
// 	}

//     cout << "Start making templates with following parameters:" << endl;
//     cout << "  - NumFeatures: " << num_features << endl;
//     cout << "  - WeakThreshold: " << weak_thresh << endl;
//     cout << "  - StrongThreshold: " << strong_thresh << endl;
//     cout << "  - AngleRange: " << angle_range.begin << " to " << angle_range.end << " with step " << angle_range.step << endl;
//     cout << "  - ScaleRange: " << scale_range.begin << " to " << scale_range.end << " with step " << scale_range.step << endl;

//     kcg_match.MakingTemplates(model, angle_range, scale_range, num_features, weak_thresh, strong_thresh);
// }

// void runMatching(KcgMatch &kcg_match, const json& config, const fs::path& exe_dir)
// {
//     float score_thresh = 0.9f;
//     float overlap = 0.4f;
//     float mag_thresh = 30.0f;
//     float greediness = 0.8f;
//     PyramidLevel pyrd_level = PyramidLevel_3;
//     int T = 2;
//     int top_k = 0;
//     MatchingStrategy strategy = Strategy_Accurate;
//     string refinement_search_mode = "fixed";
//     float fixed_angle_window = 25.0f;
//     float scale_search_window = 0.1f;
//     fs::path search_path = exe_dir / "template" / "search.jpg";
//     fs::path result_path = exe_dir / "template" / "result.png";
    
//     if (config.contains("Matching")) {
//         const auto& m_config = config["Matching"];
//         score_thresh = m_config.value("ScoreThreshold", 0.9f);
//         overlap = m_config.value("Overlap", 0.4f);
//         mag_thresh = m_config.value("MinMagThreshold", 30.0f);
//         greediness = m_config.value("Greediness", 0.8f);
//         pyrd_level = static_cast<PyramidLevel>(m_config.value("PyramidLevel", 3));
//         T = m_config.value("T", 2);
//         top_k = m_config.value("TopK", 0);
//         refinement_search_mode = m_config.value("RefinementSearchMode", "fixed");
//         fixed_angle_window = m_config.value("FixedAngleWindow", 25.0f);
//         scale_search_window = m_config.value("ScaleSearchWindow", 0.1f);
//         strategy = static_cast<MatchingStrategy>(m_config.value("Strategy", 0));
//     }

//     if (config.contains("Paths")) {
//         const auto& p_config = config["Paths"];
//         search_path = exe_dir / fs::path(p_config.value("SearchImage", "template/search.jpg"));
//         result_path = exe_dir / fs::path(p_config.value("ResultImage", "template/result.png"));
//     }

//     Mat source = imread(search_path.string());
//     if (source.empty()) {
//         cerr << "Error: Failed to read source image: " << search_path.string() << endl;
//         return;
//     }

//     Mat source_for_draw;
//     if (source.channels() == 1) {
//         cvtColor(source, source_for_draw, COLOR_GRAY2BGR);
//     } else {
//         source.copyTo(source_for_draw);
//     }
    
//     cvtColor(source, source, COLOR_BGR2GRAY);

//     cout << "Start matching with PyramidLevel=" << static_cast<int>(pyrd_level) 
//          << ", ScoreThreshold=" << score_thresh 
//          << ", Greediness=" << greediness << "..." << endl;

//     auto start = chrono::high_resolution_clock::now();
//     vector<Match> matches = kcg_match.Matching(source, score_thresh, overlap, mag_thresh,
//         greediness, pyrd_level, T, top_k, strategy, refinement_search_mode, fixed_angle_window, scale_search_window);
//     auto end = chrono::high_resolution_clock::now();
//     auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

//     cout << "Matching finished. Found " << matches.size() << " matches." << endl;
//     cout << "Time elapsed: " << duration.count() << " ms" << endl;

//     if (!matches.empty())
//     {
//         kcg_match.DrawMatches(source_for_draw, matches, Scalar(0, 255, 0));
//         string result_text = "Time: " + to_string(duration.count()) + " ms";
//         cv::putText(source_for_draw, result_text, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 0), 2);
        
//         cout << "Top match info: " << "x=" << matches[0].x << ", y=" << matches[0].y
//             << ", similarity=" << matches[0].similarity << endl;
//     }
    
//     imwrite(result_path.string(), source_for_draw);
//     cout << "Result image saved to '" << result_path.string() << "'" << endl;

//     namedWindow("Matching Result", WINDOW_NORMAL);
//     imshow("Matching Result", source_for_draw);
//     waitKey(0);
// }

// // int main(int argc, char *argv[])
// // {
// //     cout << "------------------------------------------" << endl;
// //     cout << "   KCG Template Matching Configuration    " << endl;
// //     cout << "------------------------------------------" << endl;

// //     fs::path exe_dir = get_executable_dir();

// //     json config;
// //     std::ifstream f(exe_dir / "config.jsonc");
// //     if (f.is_open()) {
// //         try {
// //             config = json::parse(f, nullptr, true, true);
// //         } catch (json::parse_error& e) {
// //             cout << "[Error] Failed to parse config.json: " << e.what() << ". Using default parameters." << endl;
// //             config = json::object();
// //         }
// //     } else {
// //         cout << "[Warning] config.json not found or failed to load. Using default parameters." << endl;
// //     }

// //     string mode = "match";
// //     if (argc > 1)
// //     {
// //         mode = argv[1];
// //     }
    
// //     fs::path model_root = exe_dir / "template";
// //     string class_name = "template_model";

// //     if (config.contains("Paths") && config["Paths"].contains("ModelRootPath")) {
// //         model_root = exe_dir / fs::path(config["Paths"].value("ModelRootPath", "template"));
// //     }
// //     if (config.contains("Model") && config["Model"].contains("ClassName")) {
// //         class_name = config["Model"].value("ClassName", class_name);
// //     }
    
// //     KcgMatch kcg_match(model_root.string(), class_name);

// //     if (mode == "create")
// //     {
// //         cout << "\n[Mode] Create Model" << endl;
// //         CreateModel(kcg_match, config, exe_dir);
// //         cout << "Model creation finished." << endl;
// //     }
// //     else
// //     {
// //         cout << "\n[Mode] Matching" << endl;
// //         try {
// //              kcg_match.LoadModel();
// //         } catch (const std::exception& e) {
// //             cout << "Error: Failed to load model. Please run with 'create' mode first." << endl;
// //             cerr << "Details: " << e.what() << endl;
// //             return -1;
// //         }
// //         cout << "Model loaded successfully." << endl;
// //         runMatching(kcg_match, config, exe_dir);
// //         cout << "Matching process finished." << endl;
// //     }

// //     return 0;
// // }

// // ==========================================================
// //                 main.cpp (Test Driver)

// //      用于测试 MatchingController 核心功能的驱动程序
// // ==========================================================

// #include "MatchingController.h"

// int main(int argc, char* argv[]) {
//     std::cout << "--- MatchingController Test Program ---" << std::endl;

//     // --- 1. 初始化阶段 ---
//     std::string exe_dir = get_executable_dir();
//     fs::path config_path = fs::path(exe_dir) / "config.jsonc"; // 指向配置文件

//     MatchingController controller;
//     bool init_ok = controller.initialize(config_path.string());

//     if (!init_ok) {
//         std::cerr << "[FATAL] Controller initialization failed. Exiting." << std::endl;
//         return -1;
//     }
    
//     // --- 2. 模拟模板匹配 ---
//     std::cout << "\n--- [PASS 1] Testing with initial configuration ---" << std::endl;
    
//     fs::path search_image_path = fs::path(exe_dir) / "template" / "search.jpg";
//     cv::Mat search_image = cv::imread(search_image_path.string());

//     if (search_image.empty()) {
//         std::cerr << "[FATAL] Failed to load search image at: " << search_image_path << std::endl;
//         return -1;
//     }
    
//     // 模拟用户设置ROI (可以注释掉这一行来测试全图匹配)
//     cv::Rect test_roi(300, 150, 900, 700);
//     controller.setROI(test_roi);


//     // 计时并执行匹配
//     auto start_time = std::chrono::high_resolution_clock::now();
//     std::vector<kcg::Match> results_pass1 = controller.processSingleFrame(search_image);
//     auto end_time = std::chrono::high_resolution_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
//     std::cout << "Found " << results_pass1.size() << " matches with initial config." << std::endl;

//     std::cout << "\n--- [PASS 2] Testing with reloaded, modified configuration ---" << std::endl;

//     // a. 读取当前的JSON文件
//     json config;
//     std::ifstream f_in(config_path.string());
//     std::cout << "a. 读取当前的JSON文件" << config << std::endl;
//     f_in >> config;
//     std::cout << "a. 读取当前的JSON文件" << config << std::endl;
//     f_in.close();
//     std::cout << "a. 读取当前的JSON文件" << config << std::endl;

//     // b. 修改JSON对象中的值
//     std::cout << "Simulating UI change: Setting ScoreThreshold to 0.45" << std::endl;
//     config["Matching"]["ScoreThreshold"] = 0.45;

//     std::cout << "b. 修改JSON对象中的值" << config << std::endl;
//     // c. 将修改后的JSON对象写回文件
//     std::ofstream f_out(config_path.string());
//     f_out << std::setw(4) << config << std::endl; // setw(4) 是为了格式化输出，方便人看
//     f_out.close();
//     std::cout << "c. 将修改后的JSON对象写回文件" << config << std::endl;
//     // d. 通知控制器重新加载配置文件
//     std::cout << "Reloading configuration..." << std::endl;
//     bool reload_ok = controller.reloadConfiguration(config_path.string());
//     if (!reload_ok) {
//         std::cerr << "[FATAL] Failed to reload configuration." << std::endl;
//         return -1;
//     }
//     std::cout << "d. 通知控制器重新加载配置文件" << config << std::endl;

//     // e. 再次执行匹配，此时应该使用新的参数了
//     auto start_time1 = std::chrono::high_resolution_clock::now();
//     std::vector<kcg::Match> results_pass2 = controller.processSingleFrame(search_image);
//     auto end_time1 = std::chrono::high_resolution_clock::now();
//     auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end_time1 - start_time1);

//     // 打印第二次的结果
//     std::cout << "Matching finished in " << duration1.count() << " ms." << std::endl;
//     std::cout << "Found " << results_pass2.size() << " matches with reloaded config." << std::endl;

//     // 绘制并保存结果图像
//     if (!results_pass2.empty()) {
//         // 创建一个副本用于绘制
//         cv::Mat result_image = search_image.clone();
        
//         // 在图上绘制ROI框
//         cv::rectangle(result_image, test_roi, cv::Scalar(255, 0, 0), 2); // 蓝色ROI框

//         // 绘制匹配结果
//         // 假设 KcgMatch 类中有一个 DrawMatches 静态方法或你有独立的绘制函数
//         // 如果没有，可以简单地在这里实现
//         for (const auto& match : results_pass2) {
//             // 这里需要知道匹配到的模板的宽高，我们假设一个固定值或从KcgMatch获取
//             cv::rectangle(result_image, cv::Point(match.x, match.y), 
//                           cv::Point(match.x + 100, match.y + 100), // 假设宽高为100x100
//                           cv::Scalar(0, 255, 0), 2);
//         }

//         fs::path result_image_path = fs::path(exe_dir) / "template" / "test_result.jpg";
//         cv::imwrite(result_image_path.string(), result_image);
//         std::cout << "Result image saved to: " << result_image_path << std::endl;
//     }

//     // --- 3. 模拟模板创建 ---
//     std::cout << "\n--- Testing createTemplateAsync ---" << std::endl;
//     fs::path template_source_path = fs::path(exe_dir) / "template" / "template.jpg";
//     cv::Mat template_source_image = cv::imread(template_source_path.string());

//     if(template_source_image.empty()) {
//         std::cerr << "[WARNING] Failed to load image for template creation at: " << template_source_path << std::endl;
//     } else {
//         // 模拟用户在 "template.jpg" 上画了一个框来创建新模板
//         cv::Rect new_template_roi(20, 20, 200, 150);
//         std::string new_model_name = "test_created_model";
        
//         std::cout << "Starting async template creation for '" << new_model_name << "'..." << std::endl;
//         std::future<bool> creation_future = controller.createTemplateAsync(template_source_image, new_template_roi, new_model_name);

//         // 等待后台任务完成
//         bool creation_ok = creation_future.get(); // get()会阻塞直到任务完成

//         if (creation_ok) {
//             std::cout << "Async template creation completed successfully." << std::endl;
//         } else {
//             std::cerr << "Async template creation failed." << std::endl;
//         }
//     }


//     std::cout << "\n--- Test Program Finished ---" << std::endl;
//     return 0;
// }

// // int main() {
// //     std::cout << "--- Starting Minimal FileStorage Test ---" << std::endl;

// //     // 1. 构造出模型文件的绝对路径
// //     std::string exe_dir = get_executable_dir();
// //     fs::path model_path = fs::path(exe_dir) / "template" / "template_model.yaml";
// //     std::string model_path_str = model_path.string();
    
// //     std::cout << "Attempting to open file: " << model_path_str << std::endl;

// //     // 2. 检查文件是否真的存在于该路径
// //     if (!fs::exists(model_path)) {
// //         std::cerr << "[FATAL] Filesystem check failed: The file does not exist at the specified path." << std::endl;
// //         system("pause");
// //         return -1;
// //     }
// //     std::cout << "[INFO] Filesystem check passed: File exists." << std::endl;

// //     // 3. 直接尝试用 cv::FileStorage 打开它
// //     cv::FileStorage fs;
// //     try {
// //         bool opened = fs.open(model_path_str, cv::FileStorage::READ);
        
// //         // 4. 检查 isOpened() 的结果
// //         if (opened && fs.isOpened()) {
// //             std::cout << "[SUCCESS] cv::FileStorage successfully opened and verified the file!" << std::endl;
// //             fs.release();
// //         } else {
// //             // 如果 fs.open 返回 true 但 isOpened 是 false，说明文件内容可能有问题
// //             std::cerr << "[FATAL] cv::FileStorage failed. fs.open() returned " << (opened ? "true" : "false")
// //                       << ", but fs.isOpened() is " << (fs.isOpened() ? "true" : "false") << "." << std::endl;
// //             std::cerr << "This strongly suggests the YAML file content is invalid or has an encoding issue." << std::endl;
// //         }

// //     } catch (const cv::Exception& e) {
// //         std::cerr << "[FATAL] An OpenCV exception occurred: " << e.what() << std::endl;
// //     } catch (const std::exception& e) {
// //         std::cerr << "[FATAL] A standard exception occurred: " << e.what() << std::endl;
// //     }

// //     system("pause"); // 暂停控制台，方便查看输出
// //     return 0;
// // }

#include "MatchingController.h"
#include <iostream>
#include <chrono>
#include <filesystem>
#include <thread> // 仅用于演示时的休眠
#include "third_party/json.hpp"
#include "KcgMatch.h"
// #include "PathHelpers.h"

namespace fs = std::filesystem;
using namespace std;
using namespace cv;
using namespace kcg;
using json = nlohmann::json;


void drawResults(cv::Mat& image, const std::vector<MatchResult>& results, const cv::Rect& roi) {
    // 绘制ROI框
    if (roi.width < image.cols || roi.height < image.rows) {
        cv::rectangle(image, roi, cv::Scalar(255, 0, 0), 2); // 蓝色ROI框
    }

    // 绘制每一个丰富匹配结果
    for (const auto& result : results) {
        // --- 1. 绘制精确的旋转矩形框 ---
        for (size_t i = 0; i < result.corners.size(); ++i) {
            cv::line(image, result.corners[i], result.corners[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
        }

        // --- 2. 在左上角角点附近，显示详细信息 ---
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) // 控制浮点数显示精度
           << "S:" << result.score
           << " A:" << result.angle
           << " Z:" << result.scale;
        
        cv::putText(image, ss.str(), result.corners[0] + cv::Point2f(0, -10), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);
    }
}

// --- 用这个函数替换掉你原来的 getExecutableDir ---
std::string getExecutableDir(const char* argv0) {
    // C++17 标准方式，argv[0] 是最可靠的获取程序路径的方式
    return std::filesystem::path(argv0).parent_path().string();
}


int main(int argc, char* argv[]) {
    std::cout << "--- MatchingController Test Program (Final Version) ---" << std::endl;

    // --- 1. 初始化 ---
    fs::path exe_dir = fs::path(getExecutableDir(argv[0]));
    fs::path config_path = exe_dir / "config.jsonc";
    
    MatchingController controller;
    if (!controller.initialize(config_path.string())) {
        std::cerr << "[FATAL] Controller initialization failed. Exiting." << std::endl;
        system("pause");
        return -1;
    }

    // --- 2. 加载测试图像 ---
    fs::path search_image_path = exe_dir / "template" / "search.jpg";
    cv::Mat search_image = cv::imread(search_image_path.string());
    if (search_image.empty()) {
        std::cerr << "[FATAL] Failed to load search image: " << search_image_path << std::endl;
        system("pause");
        return -1;
    }

    // --- 3. 第一次匹配 (使用初始配置) ---
    std::cout << "\n--- [PASS 1] Testing with initial configuration ---" << std::endl;
    cv::Rect test_roi(0, 0, 640, 480);
    controller.setROI(test_roi); // 模拟设置ROI
    
    auto results1 = controller.processSingleFrame(search_image);
    std::cout << "Found " << results1.size() << " matches with initial config." << std::endl;

    // --- 4. 模拟参数修改、保存和重载 ---
    std::cout << "\n--- [PASS 2] Simulating parameter change, save, and reload ---" << std::endl;
    controller.setParameter("ScoreThreshold", 0.45f); // 模拟UI调整参数
    controller.saveConfiguration();                   // 模拟UI点击“保存”
    controller.reloadConfiguration();                 // 模拟UI点击“重载”
    
    auto results2 = controller.processSingleFrame(search_image);
    
    std::cout << "Found " << results2.size() << " matches with reloaded config." << std::endl;

    // ===================================================================
    //      ↓↓↓  修改的核心在这里：打印和绘制新的丰富信息  ↓↓↓
    // ===================================================================

    // --- 4. 打印详细的、格式化后的结果信息 ---
    // std::cout << "\nMatching finished in " << duration.count() << " ms." << std::endl;
    std::cout << "Found " << results2.size() << " matches." << std::endl;
    
    // 设置cout的浮点数输出格式
    std::cout << std::fixed << std::setprecision(3);

    for (size_t i = 0; i < results2.size(); ++i) {
        const auto& res = results2[i];
        std::cout << "--- Match " << i << " ---" << std::endl;
        std::cout << "  - Score: " << res.score << std::endl;
        std::cout << "  - Angle: " << res.angle << std::endl;
        std::cout << "  - Scale: " << res.scale << std::endl;
        std::cout << "  - Center: (" << res.rotated_box.center.x << ", " << res.rotated_box.center.y << ")" << std::endl;
        std::cout << "  - Corners: [(" << res.corners[0].x << ", " << res.corners[0].y << "), ...]" << std::endl;
    }

    // --- 5. 绘制并保存最终结果 ---
    cv::Mat result_image_to_save = search_image.clone();
    // 调用我们新版的 drawResults
    drawResults(result_image_to_save, results2, test_roi); 
    
    fs::path result_image_path = exe_dir / "test_result_final.jpg"; // 用一个新名字保存
    cv::imwrite(result_image_path.string(), result_image_to_save);
    std::cout << "\nResult image with precise boxes saved to: " << result_image_path << std::endl;

    // --- 6. 模拟异步模板创建 ---
    std::cout << "\n--- [PASS 3] Simulating async template creation ---" << std::endl;
    fs::path template_source_path = exe_dir / "template" / "template.jpg";
    cv::Mat template_source_image = cv::imread(template_source_path.string());
    if(template_source_image.empty()) {
        std::cerr << "[WARNING] Could not load image for template creation." << std::endl;
    } else {
        std::string new_model_name = "model_created_from_test";
        cv::Rect new_template_roi(0, 0, 162, 71); // 模拟用户画的框
        
        auto creation_future = controller.createTemplateAsync(template_source_image, new_template_roi, new_model_name);
        std::cout << "Template creation started in background. Waiting for it to finish..." << std::endl;
        
        if (creation_future.get()) { // .get() 会阻塞当前线程直到后台任务完成
            std::cout << "[SUCCESS] Template '" << new_model_name << "' created successfully." << std::endl;
        } else {
            std::cerr << "[FAILURE] Template creation failed." << std::endl;
        }
    }

    std::cout << "\n--- Test Program Finished ---" << std::endl;
    system("pause"); // 在程序退出前暂停，方便查看控制台输出
    return 0;
}