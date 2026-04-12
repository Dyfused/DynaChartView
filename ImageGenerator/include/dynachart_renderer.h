#pragma once

#include "chart_store.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <functional>

// FreeType 头文件
#ifdef _WIN32
#include <ft2build.h>
#include FT_FREETYPE_H
#else
#include <freetype/freetype.h>
#endif

// 常量定义 (参考 Python 项目 lib/chart.py Board 类)
#define SIDE_BORDER -0.4
#define SIDE_CAP 6.2
#define SIDE_LIMIT -1.1
#define SIDE_VISIBLE_LIMIT -1.3
#define SIDE_VISIBLE_CAP 6.5

#define FRONT_LEFT_BORDER -0.4
#define FRONT_RIGHT_BORDER 5.4
#define FRONT_LEFT_LIMIT -0.7
#define FRONT_RIGHT_LIMIT 5.7
#define FRONT_VISIBLE_LEFT_LIMIT -0.7
#define FRONT_VISIBLE_RIGHT_LIMIT 5.7

#define FRONT_BOARD_RATE 2.0
#define FRONT_NOTE_RATE 2.0

#define NOTE_SIZE 129
#define BOARD_SIZE 149
#define TIME_SIZE 2880

#define SIDE_LINE_WIDTH 10
#define BOTTOM_LINE_WIDTH 15
#define BAR_LINE_WIDTH 7
#define SEMI_BAR_LINE_WIDTH 4
#define SPLIT_LINE_WIDTH 30

#define FONT_SIZE 96

/**
 * @brief Dynachart 风格渲染器
 * 
 * 完全克隆原 Dynachart 项目的渲染效果
 * 基于 ChartStore 解析的谱面数据生成可视化图片
 */
class DynachartRenderer {
public:
    // 渲染选项
    struct Options {
        double scale = 0.5;           // 图像缩放比例 (默认 0.5)
        int timeLimit = 32;           // 每页显示的 bar 数 (默认 32)
        double speed = 0.5;           // 显示速度 (默认 0.5)
        int barSpan = 2;              // 两条 bar 线之间的 bar 数 (默认 2)
        double semiBarSpan = 1.0/16.0; // 半 bar 间距 (默认 1/16)
        std::string fontPath; // 字体路径
        std::function<void(int current, int total)> progressCallback; // 进度回调函数
        
        Options() = default;
    };

    DynachartRenderer();
    ~DynachartRenderer();

    /**
     * @brief 从 ChartStore 生成谱面图片
     * @param chart 谱面数据 (ChartStore 对象)
     * @param outputPath 输出文件路径
     * @param options 渲染选项
     * @return true 表示成功
     */
    bool generate(const chart_store& chart, 
                  const std::string& outputPath,
                  const Options& options = Options());

    /**
     * @brief 设置字体路径
     */
    void setFontPath(const std::string& path);

private:

    // 音符尺寸
    static constexpr double NOTE_WIDTH_NORMAL = 32;
    static constexpr double NOTE_WIDTH_CHAIN = 14;
    static constexpr double NOTE_WIDTH_HOLD = 32;
    static constexpr double NOTE_WIDTH_MIN = 14;

    // 颜色定义 (BGRA 顺序 - OpenCV 使用 BGR)
    static const cv::Scalar COLOR_BACKGROUND;
    static const cv::Scalar COLOR_LINE;
    static const cv::Scalar COLOR_BAR_LINE;
    static const cv::Scalar COLOR_SEMI_BAR_LINE;
    static const cv::Scalar COLOR_SPLIT_LINE;
    static const cv::Scalar COLOR_FONT;
    
    // 音符颜色 (BGR 顺序)
    static const cv::Scalar NOTE_COLOR_NORMAL;      // RGB(0,255,255) -> BGR(255,255,0) 青色
    static const cv::Scalar NOTE_COLOR_CHAIN;       // RGB(255,51,51) -> BGR(51,51,255) 红色
    static const cv::Scalar NOTE_COLOR_HOLD_BOARD;  // RGB(255,255,128) -> BGR(128,255,255) 黄色
    static const cv::Scalar NOTE_COLOR_HOLD_FILL;   // RGB(70,134,0) -> BGR(0,134,70) 绿色

    // 内部音符结构
    struct RenderNote {
        int id;
        int type;  // 0=normal, 1=chain, 2=hold
        int side;  // -1=left, 0=front, 1=right
        double pos;    // 中心位置
        double width;
        double start;
        double end;
        cv::Scalar color;
        bool hasSub;   // HOLD 音符是否有 SUB 尾部标记
        int subId;     // SUB 音符 ID（如果有）
    };

    // 页面布局参数
    struct PageLayout {
        double pageWidth;
        double pageHeight;
        double barHeight;
        double bottomLineY;
        double sideLineLeftSideX;
        double sideLineLeftX;
        double sideLineRightX;
        double sideLineRightSideX;
    };

    // 渲染方法
    cv::Mat render(const chart_store& chart, const Options& options);
    cv::Mat drawBoard(const chart_store& chart, const Options& options, PageLayout& layout);
    cv::Mat drawPage(const Options& options, PageLayout& layout);
    void drawNotes(cv::Mat& board, const PageLayout& layout, 
                   const std::vector<RenderNote>& notes, const Options& options,
                   const std::function<void(int, int)>& progressCallback = nullptr);
    void drawTimeMarkers(cv::Mat& img, const PageLayout& layout, 
                        const std::vector<bpmchange>& bpmList, 
                        double barPerMin, double timeOffset,
                        const Options& options);
    
    void drawTimeMarker(cv::Mat& img, int pageX, int y, int barIndex,
                       const chart_store& chart, const Options& options,
                       const PageLayout& layout);
    
    // 音符渲染
    cv::Mat generateNoteImage(const RenderNote& note, 
                             double widthPerUnit, 
                             double barHeight,
                             const Options& options);
    
    // 辅助函数
    double getMaxTime(const chart_store& chart);
    std::vector<RenderNote> convertNotes(const chart_store& chart);
    void cleanup();

    // FreeType 资源

    FT_Library ftLibrary_;
    FT_Face ftFace_;

    std::string fontPath_;
    bool freeTypeAvailable_;
};
