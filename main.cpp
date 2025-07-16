#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <filesystem>
#include "third_party/json.hpp"
#include "KcgMatch.h"
#include "PathHelpers.h"

namespace fs = std::filesystem;
using namespace std;
using namespace cv;
using namespace kcg;
using json = nlohmann::json;

void CreateModel(KcgMatch &kcg_match, const json& config, const fs::path& exe_dir)
{
    int num_features = 100;
    float weak_thresh = 30.0f;
    float strong_thresh = 60.0f;
    AngleRange angle_range(-180.f, 180.f, 10.f);
    ScaleRange scale_range(0.7f, 1.3f, 0.05f);
    fs::path template_path = exe_dir / "template" / "template.png";

    if (config.contains("TemplateMaking")) {
        const auto& tm_config = config["TemplateMaking"];
        num_features = tm_config.value("NumFeatures", 100);
        weak_thresh = tm_config.value("WeakThreshold", 30.0f);
        strong_thresh = tm_config.value("StrongThreshold", 60.0f);
        angle_range.begin = tm_config.value("AngleStart", -180.0f);
        angle_range.end = tm_config.value("AngleEnd", 180.0f);
        angle_range.step = tm_config.value("AngleStep", 10.0f);
        scale_range.begin = tm_config.value("ScaleStart", 0.7f);
        scale_range.end = tm_config.value("ScaleEnd", 1.3f);
        scale_range.step = tm_config.value("ScaleStep", 0.05f);
    }

    if (config.contains("Paths") && config["Paths"].contains("TemplateImage")) {
        template_path = exe_dir / fs::path(config["Paths"].value("TemplateImage", "template/template.png"));
    }

    Mat model = imread(template_path.string());
    if (model.empty())
    {
        cout << "Error: read model image '" << template_path.string() << "' failed." << endl;
        return;
    }
    
    if (model.channels() > 1) {
		cvtColor(model, model, COLOR_BGR2GRAY);
	}

    cout << "Start making templates with following parameters:" << endl;
    cout << "  - NumFeatures: " << num_features << endl;
    cout << "  - WeakThreshold: " << weak_thresh << endl;
    cout << "  - StrongThreshold: " << strong_thresh << endl;
    cout << "  - AngleRange: " << angle_range.begin << " to " << angle_range.end << " with step " << angle_range.step << endl;
    cout << "  - ScaleRange: " << scale_range.begin << " to " << scale_range.end << " with step " << scale_range.step << endl;

    kcg_match.MakingTemplates(model, angle_range, scale_range, num_features, weak_thresh, strong_thresh);
}

void runMatching(KcgMatch &kcg_match, const json& config, const fs::path& exe_dir)
{
    float score_thresh = 0.9f;
    float overlap = 0.4f;
    float mag_thresh = 30.0f;
    float greediness = 0.8f;
    PyramidLevel pyrd_level = PyramidLevel_3;
    int T = 2;
    int top_k = 0;
    MatchingStrategy strategy = Strategy_Accurate;
    fs::path search_path = exe_dir / "template" / "search.jpg";
    fs::path result_path = exe_dir / "template" / "result.png";
    
    if (config.contains("Matching")) {
        const auto& m_config = config["Matching"];
        score_thresh = m_config.value("ScoreThreshold", 0.9f);
        overlap = m_config.value("Overlap", 0.4f);
        mag_thresh = m_config.value("MinMagThreshold", 30.0f);
        greediness = m_config.value("Greediness", 0.8f);
        pyrd_level = static_cast<PyramidLevel>(m_config.value("PyramidLevel", 3));
        T = m_config.value("T", 2);
        top_k = m_config.value("TopK", 0);
        strategy = static_cast<MatchingStrategy>(m_config.value("Strategy", 0));
    }

    if (config.contains("Paths")) {
        const auto& p_config = config["Paths"];
        search_path = exe_dir / fs::path(p_config.value("SearchImage", "template/search.jpg"));
        result_path = exe_dir / fs::path(p_config.value("ResultImage", "template/result.png"));
    }

    Mat source = imread(search_path.string());
    if (source.empty()) {
        cerr << "Error: Failed to read source image: " << search_path.string() << endl;
        return;
    }

    Mat source_for_draw;
    if (source.channels() == 1) {
        cvtColor(source, source_for_draw, COLOR_GRAY2BGR);
    } else {
        source.copyTo(source_for_draw);
    }
    
    cvtColor(source, source, COLOR_BGR2GRAY);

    cout << "Start matching with PyramidLevel=" << static_cast<int>(pyrd_level) 
         << ", ScoreThreshold=" << score_thresh 
         << ", Greediness=" << greediness << "..." << endl;

    auto start = chrono::high_resolution_clock::now();
    vector<Match> matches = kcg_match.Matching(source, score_thresh, overlap, mag_thresh,
        greediness, pyrd_level, T, top_k, strategy);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Matching finished. Found " << matches.size() << " matches." << endl;
    cout << "Time elapsed: " << duration.count() << " ms" << endl;

    if (!matches.empty())
    {
        kcg_match.DrawMatches(source_for_draw, matches, Scalar(0, 255, 0));
        string result_text = "Time: " + to_string(duration.count()) + " ms";
        cv::putText(source_for_draw, result_text, Point(5, 20), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 0), 2);
        
        cout << "Top match info: " << "x=" << matches[0].x << ", y=" << matches[0].y
            << ", similarity=" << matches[0].similarity << endl;
    }
    
    imwrite(result_path.string(), source_for_draw);
    cout << "Result image saved to '" << result_path.string() << "'" << endl;

    namedWindow("Matching Result", WINDOW_NORMAL);
    imshow("Matching Result", source_for_draw);
    waitKey(0);
}

int main(int argc, char *argv[])
{
    cout << "------------------------------------------" << endl;
    cout << "   KCG Template Matching Configuration    " << endl;
    cout << "------------------------------------------" << endl;

    fs::path exe_dir = get_executable_dir();

    json config;
    std::ifstream f(exe_dir / "config.json");
    if (f.is_open()) {
        try {
            config = json::parse(f, nullptr, true, true);
        } catch (json::parse_error& e) {
            cout << "[Error] Failed to parse config.json: " << e.what() << ". Using default parameters." << endl;
            config = json::object();
        }
    } else {
        cout << "[Warning] config.json not found or failed to load. Using default parameters." << endl;
    }

    string mode = "match";
    if (argc > 1)
    {
        mode = argv[1];
    }
    
    fs::path model_root = exe_dir / "template";
    string class_name = "template_model";

    if (config.contains("Paths") && config["Paths"].contains("ModelRootPath")) {
        model_root = exe_dir / fs::path(config["Paths"].value("ModelRootPath", "template"));
    }
    if (config.contains("Model") && config["Model"].contains("ClassName")) {
        class_name = config["Model"].value("ClassName", class_name);
    }
    
    KcgMatch kcg_match(model_root.string(), class_name);

    if (mode == "create")
    {
        cout << "\n[Mode] Create Model" << endl;
        CreateModel(kcg_match, config, exe_dir);
        cout << "Model creation finished." << endl;
    }
    else
    {
        cout << "\n[Mode] Matching" << endl;
        try {
             kcg_match.LoadModel();
        } catch (const std::exception& e) {
            cout << "Error: Failed to load model. Please run with 'create' mode first." << endl;
            cerr << "Details: " << e.what() << endl;
            return -1;
        }
        cout << "Model loaded successfully." << endl;
        runMatching(kcg_match, config, exe_dir);
        cout << "Matching process finished." << endl;
    }

    return 0;
}
