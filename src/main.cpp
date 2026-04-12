/**
 * @brief DynaChartView - 谱面可视化主程序
 * 
 * 基于原 Dynachart 项目的渲染逻辑
 * 读取谱面 XML 文件并生成完全克隆效果的可视化图片
 * 
 * 用法：DynaChartView <input.xml> <output.png> [options]
 * 
 * 选项:
 *   -s, --scale <float>       图像缩放比例 (默认：0.5)
 *   -S, --speed <float>       显示速度 (默认：0.5)
 *   -l, --page-limit <int>    每页 bar 数 (默认：32)
 *   -b, --bar-span <int>      bar 线间距 (默认：2)
 *   -f, --font <path>         字体路径 (默认：Fonts/arial.ttf)
 *   -h, --help                显示帮助
 */

#include "chart_store.h"
#include "dynachart_renderer.h"
#include "defs.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#endif

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " <input.xml> <output.png> [options]\n"
              << "\nOptions:\n"
              << "  -s, --scale <float>       Image scale factor (default: 0.5)\n"
              << "  -S, --speed <float>       Display speed (default: 0.5)\n"
              << "  -l, --page-limit <int>    Max bars per page (default: 32)\n"
              << "  -b, --bar-span <int>      Bars between bar lines (default: 2)\n"
              << "  -f, --font <path>         Custom font path\n"
              << "  --use-system-font         Use system Arial font (default on Windows)\n"
              << "  -h, --help                Show this help message\n";
}

int main(int argc, char* argv[]) {
    // 设置 UTF-8 运行时环境
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
#endif

    // 默认参数
    DynachartRenderer::Options options;
    std::string inputFile;
    std::string outputFile;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "-s" || arg == "--scale") {
            if (i + 1 < argc) {
                options.scale = std::stod(argv[++i]);
            }
        }
        else if (arg == "-S" || arg == "--speed") {
            if (i + 1 < argc) {
                options.speed = std::stod(argv[++i]);
            }
        }
        else if (arg == "-l" || arg == "--page-limit") {
            if (i + 1 < argc) {
                options.timeLimit = std::stoi(argv[++i]);
            }
        }
        else if (arg == "-b" || arg == "--bar-span") {
            if (i + 1 < argc) {
                options.barSpan = std::stoi(argv[++i]);
            }
        }
        else if (arg == "-f" || arg == "--font") {
            if (i + 1 < argc) {
                options.fontPath = argv[++i];
            }
        }
        else if (arg == "--use-system-font") {
            // 使用系统字体目录的 Arial
#ifdef _WIN32
            options.fontPath = "C:\\Windows\\Fonts\\arial.ttf";
#else
            options.fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
        }
        else if (arg[0] != '-') {
            if (inputFile.empty()) {
                inputFile = arg;
            } else if (outputFile.empty()) {
                outputFile = arg;
            }
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    if (inputFile.empty()) {
        std::cerr << "Error: Input file required." << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    // 如果未指定输出文件名，使用输入文件名，后缀改为.png
    if (outputFile.empty()) {
        // 找到最后一个路径分隔符
        size_t lastSlash = inputFile.find_last_of("/\\");
        std::string basePath = (lastSlash != std::string::npos) ? 
                               inputFile.substr(0, lastSlash + 1) : "";
        std::string fileName = (lastSlash != std::string::npos) ? 
                               inputFile.substr(lastSlash + 1) : inputFile;
        
        // 移除原有后缀
        size_t lastDot = fileName.find_last_of('.');
        if (lastDot != std::string::npos) {
            fileName = fileName.substr(0, lastDot);
        }
        
        outputFile = basePath + fileName + ".png";
    }
    
    // 检查文件是否存在
#ifdef _WIN32
    if (_access(inputFile.c_str(), 0) == -1) {
#else
    if (access(inputFile.c_str(), 0) == -1) {
#endif
        std::cerr << "Error: File not found: " << inputFile << std::endl;
        return 1;
    }
    
    try {
        // 创建谱面存储对象
        chart_store chart;
        
        // 读取谱面文件
        std::cout << "Reading chart from: " << inputFile << std::endl;
        int result = chart.readfile(inputFile);
        
        if (result != 0) {
            std::cerr << "Warning: Chart read flags = " << result << std::endl;
            if (result & BARPM_MISSING) {
                std::cerr << "  - BarPerMin is missing" << std::endl;
            }
            if (result & LEFT_SIDE_MISSING) {
                std::cerr << "  - Left side type is missing" << std::endl;
            }
            if (result & RIGHT_SIDE_MISSING) {
                std::cerr << "  - Right side type is missing" << std::endl;
            }
            if (result & HOLD_SUB_MISMATCH) {
                std::cerr << "  - Hold-Sub mismatch detected" << std::endl;
            }
        }
        
        // 显示谱面信息
        std::cout << "\nChart Information:" << std::endl;
        std::cout << "  Notes (Center): " << chart.get_mid_count() << std::endl;
        std::cout << "  Notes (Left): " << chart.get_left_count() << std::endl;
        std::cout << "  Notes (Right): " << chart.get_right_count() << std::endl;
        std::cout << "  Tap Count: " << chart.get_tap_count() << std::endl;
        std::cout << "  Chain Count: " << chart.get_chain_count() << std::endl;
        std::cout << "  Hold Count: " << chart.get_hold_count() << std::endl;
        std::cout << "  BPM Changes: " << chart.bpm_list.size() << std::endl;
        
        // 创建渲染器
        DynachartRenderer renderer;
        
        // 设置字体路径
        std::string fontPath = options.fontPath;
        if (fontPath.empty()) {
            // 默认字体路径：Windows 系统字体目录
#ifdef _WIN32
            fontPath = "C:\\Windows\\Fonts\\arial.ttf";
#else
            fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
        }
        
        // 检查字体文件是否存在
#ifdef _WIN32
        bool fontExists = (_access(fontPath.c_str(), 0) == 0);
#else
        bool fontExists = (access(fontPath.c_str(), 0) == 0);
#endif
        
        if (fontExists) {
            renderer.setFontPath(fontPath);
            std::cout << "Font loaded: " << fontPath << std::endl;
        } else {
            std::cerr << "Warning: Font not found at " << fontPath 
                     << ", text rendering may be disabled." << std::endl;
        }
        
        // 生成图片
        std::cout << "\nGenerating image to: " << outputFile << std::endl;
        std::cout << "  Scale: " << options.scale << std::endl;
        std::cout << "  Speed: " << options.speed << std::endl;
        std::cout << "  Time Limit: " << options.timeLimit << " bars/page" << std::endl;
        std::cout << "  Bar Span: " << options.barSpan << std::endl;
        
        // 设置进度回调
        auto startTime = std::chrono::steady_clock::now();
        options.progressCallback = [](int current, int total) {
            double progress = (current * 100.0) / total;
            int barWidth = 30;
            int pos = static_cast<int>(progress * barWidth / 100.0);
            
            std::cout << "\r[";
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << std::fixed << std::setprecision(1) << progress << "% "
                      << "(" << current << "/" << total << ") " << std::flush;
        };
        
        if (renderer.generate(chart, outputFile, options)) {
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            std::cout << "\r" << std::string(80, ' ') << "\r"; // 清除进度条
            std::cout << "\nSuccess! Image generated in " 
                      << duration.count() / 1000.0 << " seconds." << std::endl;
        } else {
            std::cerr << "Error: Failed to generate image." << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
