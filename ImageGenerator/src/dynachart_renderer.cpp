#include "dynachart_renderer.h"
#include "defs.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

// FreeType 头文件
#ifdef _WIN32
#include <ft2build.h>
#include FT_FREETYPE_H
#else
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>
#endif

// 颜色定义 (BGRA 顺序)
const cv::Scalar DynachartRenderer::COLOR_BACKGROUND(0, 0, 0, 255);
const cv::Scalar DynachartRenderer::COLOR_LINE(255, 255, 255, 255);
const cv::Scalar DynachartRenderer::COLOR_BAR_LINE(255, 255, 255, 255);
const cv::Scalar DynachartRenderer::COLOR_SEMI_BAR_LINE(127, 127, 127, 255);
const cv::Scalar DynachartRenderer::COLOR_SPLIT_LINE(255, 255, 255, 255);
const cv::Scalar DynachartRenderer::COLOR_FONT(255, 255, 255, 255);

// 音符颜色 (BGR 顺序)
const cv::Scalar DynachartRenderer::NOTE_COLOR_NORMAL(255, 255, 0, 255);      // 青色
const cv::Scalar DynachartRenderer::NOTE_COLOR_CHAIN(51, 51, 255, 255);       // 红色
const cv::Scalar DynachartRenderer::NOTE_COLOR_HOLD_BOARD(128, 255, 255, 200); // 黄色（半透明）
const cv::Scalar DynachartRenderer::NOTE_COLOR_HOLD_FILL(0, 134, 70, 180);    // 绿色（半透明）

// 侧边背景颜色 (BGR 顺序)
const cv::Scalar DynachartRenderer::COLOR_LEFT_SIDE_BG(20, 60, 20, 255);      // 暗绿色 (左侧背景)
const cv::Scalar DynachartRenderer::COLOR_RIGHT_SIDE_BG(60, 20, 60, 255);     // 暗紫色 (右侧背景)

DynachartRenderer::DynachartRenderer() : ftLibrary_(nullptr), ftFace_(nullptr), freeTypeAvailable_(false) {}

DynachartRenderer::~DynachartRenderer() {
    cleanup();
}

void DynachartRenderer::cleanup() {
    if (ftFace_) {
        FT_Done_Face(ftFace_);
        ftFace_ = nullptr;
    }
    if (ftLibrary_) {
        FT_Done_FreeType(ftLibrary_);
        ftLibrary_ = nullptr;
    }
}

void DynachartRenderer::setFontPath(const std::string& path) {
    fontPath_ = path;
    
    // 加载字体
    if (FT_Init_FreeType(&ftLibrary_) != 0) {
        std::cerr << "[ERROR] Failed to initialize FreeType library" << std::endl;
        freeTypeAvailable_ = false;
        return;
    }
    
    if (FT_New_Face(ftLibrary_, fontPath_.c_str(), 0, &ftFace_) != 0) {
        std::cerr << "[ERROR] Failed to load font face from: " << fontPath_ << std::endl;
        std::cerr << "[ERROR] Error code: " << FT_New_Face(ftLibrary_, fontPath_.c_str(), 0, &ftFace_) << std::endl;
        FT_Done_FreeType(ftLibrary_);
        ftLibrary_ = nullptr;
        freeTypeAvailable_ = false;
        return;
    }
    
    freeTypeAvailable_ = true;
    std::cout << "[SUCCESS] FreeType initialized successfully, font: " << fontPath_ << std::endl;
}

double DynachartRenderer::getMaxTime(const chart_store& chart) {
    double maxTime = 0.0;
    
    // 检查所有侧的最大时间
    for (const auto& pair : chart.m_notes) {
        double noteEnd = (pair.second.notetype == HOLD) ? 
                         (pair.second.time + pair.second.width) : pair.second.time;
        if (noteEnd > maxTime) maxTime = noteEnd;
    }
    for (const auto& pair : chart.m_left) {
        double noteEnd = (pair.second.notetype == HOLD) ? 
                         (pair.second.time + pair.second.width) : pair.second.time;
        if (noteEnd > maxTime) maxTime = noteEnd;
    }
    for (const auto& pair : chart.m_right) {
        double noteEnd = (pair.second.notetype == HOLD) ? 
                         (pair.second.time + pair.second.width) : pair.second.time;
        if (noteEnd > maxTime) maxTime = noteEnd;
    }
    
    return maxTime > 0 ? maxTime : 100.0;
}

std::vector<DynachartRenderer::RenderNote> DynachartRenderer::convertNotes(const chart_store& chart) {
    std::vector<RenderNote> notes;
    
    // 为每个面分别收集 SUB 音符的位置，用于计算 HOLD 长度
    // 避免不同面的相同 ID 的 note 互相覆盖
    std::map<int, double> subPositionsLeft;   // LEFT 面 SUB 音符 ID -> time 位置
    std::map<int, double> subPositionsFront;  // FRONT 面 SUB 音符 ID -> time 位置
    std::map<int, double> subPositionsRight;  // RIGHT 面 SUB 音符 ID -> time 位置
    
    auto collectSubPositions = [&](const std::map<int, note>& noteMap, std::map<int, double>& subPositions) {
        for (const auto& pair : noteMap) {
            const note& n = pair.second;
            if (n.notetype == SUB) {
                subPositions[n.id] = n.time;
            }
        }
    };
    
    collectSubPositions(chart.m_left, subPositionsLeft);
    collectSubPositions(chart.m_notes, subPositionsFront);
    collectSubPositions(chart.m_right, subPositionsRight);
    
    // process all the 3 sides
    auto processSide = [&](const std::map<int, note>& noteMap, int side,
                          const std::map<int, int>& holdMap,
                          const std::map<int, double>& subPositions) {
        for (const auto& pair : noteMap) {
            const note& n = pair.second;
            if (n.notetype == NULLTP) continue;
            if (n.notetype == SUB) continue;  // SUB 不单独渲染，作为 HOLD 的尾部标记
            
            RenderNote rn;
            rn.id = n.id; //note id
			rn.side = side; // -1=left, 0=front, 1=right
            rn.width = n.width;
            rn.start = n.time;
            rn.pos = n.position+n.width/2.0;  // ChartStore 中的 position 已经是中心位置 (并非，position+width/2才是)
            rn.hasSub = false;
            rn.subId = -1;
            
            switch (n.notetype) {
                case NORMAL:
                    rn.type = 0;
                    rn.color = NOTE_COLOR_NORMAL;
                    break;
                case CHAIN:
                    rn.type = 1;
                    rn.color = NOTE_COLOR_CHAIN;
                    break;
                case HOLD: {
                    rn.type = 2;
                    rn.color = NOTE_COLOR_HOLD_FILL;
                    // HOLD 音符的 end 时间从对应的 SUB 音符获取
                    auto it = holdMap.find(n.id);
                    if (it != holdMap.end()) {
                        rn.hasSub = true;
                        rn.subId = it->second;
                        // 用 SUB 音符的 time 作为 HOLD 的结束时间
                        auto subIt = subPositions.find(it->second);
                        if (subIt != subPositions.end()) {
                            rn.end = subIt->second;
                            /*
                            std::cout<<"[Info] HOLD note with ID " << n.id << " on " 
                                     << (side == -1 ? "Left" : (side == 0 ? "Front" : "Right")) 
                                     << " side has corresponding SUB note with ID " << it->second 
								     << " at time " << subIt->second << "." << std::endl;
                            */
                        } else {
                            switch (side) {
                            case -1:  // LEFT
                                std::cout << "[Warning] HOLD note with ID " << n.id << " on Left side has no corresponding SUB note." << std::endl;
                                break;
                            case 0:   // FRONT
                                std::cout << "[Warning] HOLD note with ID " << n.id << " on Front side has no corresponding SUB note." << std::endl;
                                break;
                            case 1:   // RIGHT
                                std::cout << "[Warning] HOLD note with ID " << n.id << " on Right side has no corresponding SUB note." << std::endl;
                                break;
                            }
                            // 如果找不到 SUB，用 width 计算
                            rn.end = n.time + n.width;
                            
                        }
                    } else {
                        switch (side) {
						case -1:  // LEFT
                            std::cout << "[Warning] HOLD note with ID " << n.id << " on Left side has no corresponding SUB note." << std::endl;
                            break;
                        case 0:   // FRONT
                            std::cout << "[Warning] HOLD note with ID " << n.id << " on Front side has no corresponding SUB note." << std::endl;
							break;
                        case 1:   // RIGHT
							std::cout << "[Warning] HOLD note with ID " << n.id << " on Right side has no corresponding SUB note." << std::endl;
                            break;
                        }
                        // 没有 SUB 的 HOLD，用 width 计算
                        rn.end = n.time + n.width;
                    }
                    break;
                }
                default:
                    rn.type = 0;
                    rn.color = NOTE_COLOR_NORMAL;
                    break;
            }
            
            if (rn.type != 2) {
                rn.end = rn.start;
            }
            
            notes.push_back(rn);
        }
    };
    
    processSide(chart.m_left, -1, chart.hold_left, subPositionsLeft);    // SIDE_LEFT
    processSide(chart.m_notes, 0, chart.hold_mid, subPositionsFront);     // SIDE_FRONT
    processSide(chart.m_right, 1, chart.hold_right, subPositionsRight);   // SIDE_RIGHT
    
    // 按开始时间排序
    std::sort(notes.begin(), notes.end(), 
              [](const RenderNote& a, const RenderNote& b) {
                  if (a.start != b.start) return a.start < b.start;
                  // 同一起始时间的 hold 音符，应用碰撞检测逻辑
                  if (a.type == 2 && b.type == 2) {
                      // DO NOT CONVERT THE POSITIONS TO INT!!!!!!!!!
					  double a_left = a.pos - a.width / 2.0; //position 是中心位置，计算左右边界
                      double a_right = a.pos + a.width / 2.0;
                      double b_left = b.pos - b.width / 2.0;
                      double b_right = b.pos + b.width / 2.0;
                      
                      // 检查是否一个 hold 头完全盖住另一个 hold 头
                      bool a_covers_b = (a_left <= b_left && a_right >= b_right);  // a 完全盖住 b
                      bool b_covers_a = (b_left <= a_left && b_right >= a_right);  // b 完全盖住 a
					  //std::cout << "Detected coverage.\nA: [" << a_left << ", " << a_right << "], B: [" << b_left << ", " << b_right << "] at time " << a.start << std::endl;
                      if (a_covers_b || b_covers_a) {
                          // 完全覆盖情况：按宽度排序
                          // 宽度大的排在前面（先绘制），宽度小的排在后面（后绘制，显示在上面）
                          if (a.width != b.width) return a.width > b.width;  // 宽度降序
                          // 宽度相同，先结束的（end 更小）排在后面（显示在上面）
                          if (a.end != b.end) return a.end > b.end;  // 结束时间降序
                          // 起始时间、宽度、结束时间都相同，id 靠后的排在后面（显示在上面）
                          return a.id < b.id;  // id 升序
                      } else {
                          // 非完全覆盖情况（有重叠但不完全覆盖，或完全不重叠）
                          // 结束时间早的放在结束时间晚的前面（后绘制，在上面）
                          return a.end > b.end;  // 结束时间降序
                      }
                  }
                  return false;
              });
    
    return notes;
}

cv::Mat DynachartRenderer::generateNoteImage(const RenderNote& note,
                                            double widthPerUnit,
                                            double barHeight,
                                            const Options& options) {
    cv::Scalar fillColor = note.color;
    cv::Scalar outlineColor = cv::Scalar(0, 0, 0, 0);
    int lineWidth = 1;
    
    // 使用 BOARD_SIZE 而不是 NOTE_SIZE 来计算宽度，确保 X 坐标和图像宽度使用相同的基准
    double baseWidthPerUnit = BOARD_SIZE * options.scale;
    if (note.side == 0) {  // FRONT
        baseWidthPerUnit *= FRONT_BOARD_RATE;
    }
    double width = baseWidthPerUnit * note.width;
    int height = static_cast<int>(std::max(1.0, std::round(options.scale * NOTE_WIDTH_NORMAL)));
    double radius = height / 2.0;
    
    if (note.type == 1) {  // CHAIN
        fillColor = NOTE_COLOR_CHAIN;
        height = static_cast<int>(std::max(1.0, std::round(options.scale * NOTE_WIDTH_CHAIN)));
        radius = height / 2.0;
    } else if (note.type == 2) {  // HOLD
        fillColor = NOTE_COLOR_HOLD_FILL;
        outlineColor = NOTE_COLOR_HOLD_BOARD;
        double widthHold = std::max(1.0, std::round(options.scale * NOTE_WIDTH_HOLD));
        lineWidth = static_cast<int>(widthHold);
        // 参考 Python 逻辑：height = bar_height * (self.end - self.start) + WIDTH_HOLD
        // 确保高度至少为 1
        double holdDuration = std::max(0.001, note.end - note.start);
        height = static_cast<int>(std::max(1.0, std::round(barHeight * holdDuration + widthHold)));
        radius = std::max(1.0, std::round(options.scale * NOTE_WIDTH_HOLD / 2.0));
    }
    
    int w = static_cast<int>(std::round(width));
    int h = static_cast<int>(std::round(height));
    
    int minWidth = static_cast<int>(std::max(1.0, std::round(options.scale * NOTE_WIDTH_MIN)));
    w = std::max(w, minWidth);
    h = std::max(h, minWidth);
    
    // 微调：减小 note 的视觉宽度，让同一时间相邻位置的 note 在视觉上有间隙
    // 减去一个微小的固定值（约 10 像素 * scale），保持中心不变
    int widthReduction = static_cast<int>(std::round(10.0 * options.scale));
    w = std::max(w - widthReduction, minWidth);
    
    // 创建透明背景
    cv::Mat img = cv::Mat::zeros(h, w, CV_8UC4);
    
    // 绘制圆角矩形 (简化为普通矩形)
    cv::Point pt1(0, 0);
    cv::Point pt2(w - 1, h - 1);
    
    cv::rectangle(img, pt1, pt2, fillColor, -1);
    
    // HOLD 音符添加边框
    if (note.type == 2) {
        cv::rectangle(img, pt1, pt2, outlineColor, lineWidth);
        
        // 如果有 SUB 尾部标记，在底部绘制 SUB 标记
        if (note.hasSub) {
            int subHeight = static_cast<int>(std::round(options.scale * NOTE_WIDTH_HOLD));
            int subY = h - subHeight;
            cv::Rect subRect(0, subY, w, subHeight);
            cv::rectangle(img, subRect, outlineColor, -1);
        }
    }
    
    return img;
}

cv::Mat DynachartRenderer::drawPage(const Options& options, PageLayout& layout) {
    // 计算页面尺寸 (参考 Python 项目 lib/chart.py draw_page 函数)
    // Python: page_width = (2 * (SIDE_VISIBLE_CAP - SIDE_VISIBLE_LIMIT) + FRONT_BOARD_RATE * (FRONT_VISIBLE_RIGHT_LIMIT - FRONT_VISIBLE_LEFT_LIMIT)) * BOARD_SIZE * scale
    layout.pageWidth = std::round((
        2 * (SIDE_VISIBLE_CAP - SIDE_VISIBLE_LIMIT) +
        FRONT_BOARD_RATE * (FRONT_VISIBLE_RIGHT_LIMIT - FRONT_VISIBLE_LEFT_LIMIT)
    ) * BOARD_SIZE * options.scale);
    
    double pageBottom = std::round(BOARD_SIZE * (SIDE_BORDER - SIDE_LIMIT) * options.scale);
    layout.barHeight = options.speed * TIME_SIZE;
    layout.pageHeight = std::round(2 * pageBottom + (layout.barHeight * options.timeLimit) * options.scale);
    
    layout.bottomLineY = layout.pageHeight - pageBottom;
    
    // 计算侧线位置 (参考 Python 项目 lib/chart.py draw_page 函数)
    // Python 公式：
    // side_line_leftside_x = (SIDE_VISIBLE_CAP - SIDE_BORDER) * BOARD_SIZE * scale = 6.8 * BOARD_SIZE * scale
    // side_line_left_x = (SIDE_VISIBLE_CAP - SIDE_VISIBLE_LIMIT) + FRONT_BOARD_RATE * (FRONT_LEFT_BORDER - FRONT_VISIBLE_LEFT_LIMIT) = 7.8 + 0.8 = 8.6
    // side_line_right_x = (SIDE_VISIBLE_CAP - SIDE_VISIBLE_LIMIT) + FRONT_BOARD_RATE * (FRONT_RIGHT_BORDER - FRONT_VISIBLE_LEFT_LIMIT) = 7.8 + 12.0 = 19.8
    // side_line_rightside_x = (SIDE_VISIBLE_CAP - SIDE_VISIBLE_LIMIT) + (SIDE_BORDER - SIDE_VISIBLE_LIMIT) + FRONT_BOARD_RATE * (FRONT_VISIBLE_RIGHT_LIMIT - FRONT_VISIBLE_LEFT_LIMIT) = 7.8 + 1.0 + 12.8 = 21.6
    // page_width = 2 * (SIDE_VISIBLE_CAP - SIDE_VISIBLE_LIMIT) + FRONT_BOARD_RATE * (FRONT_VISIBLE_RIGHT_LIMIT - FRONT_VISIBLE_LEFT_LIMIT) = 15.6 + 12.8 = 28.4
    
    // Left Side 左边界 (最左侧)
    layout.sideLineLeftSideX = std::round((SIDE_VISIBLE_CAP - SIDE_BORDER) * BOARD_SIZE * options.scale);
    // Front 板左边界 (Left Side 右边界)
    layout.sideLineLeftX = std::round((
        (SIDE_VISIBLE_CAP - SIDE_VISIBLE_LIMIT) +
        FRONT_BOARD_RATE * (FRONT_LEFT_BORDER - FRONT_VISIBLE_LEFT_LIMIT)
    ) * BOARD_SIZE * options.scale);
    // Front 板右边界
    layout.sideLineRightX = std::round((
        (SIDE_VISIBLE_CAP - SIDE_VISIBLE_LIMIT) +
        FRONT_BOARD_RATE * (FRONT_RIGHT_BORDER - FRONT_VISIBLE_LEFT_LIMIT)
    ) * BOARD_SIZE * options.scale);
    // Right Side 右边界 (最右侧)
    layout.sideLineRightSideX = std::round((
        (SIDE_VISIBLE_CAP - SIDE_VISIBLE_LIMIT) +
        (SIDE_BORDER - SIDE_VISIBLE_LIMIT) +
        FRONT_BOARD_RATE * (FRONT_VISIBLE_RIGHT_LIMIT - FRONT_VISIBLE_LEFT_LIMIT)
    ) * BOARD_SIZE * options.scale);
    
    // 创建背景
    cv::Mat img = cv::Mat::zeros(layout.pageHeight, layout.pageWidth, CV_8UC4);
    img.setTo(COLOR_BACKGROUND);
    
    // 绘制侧边背景色块 (在线条之下)
    // 左侧区域：从最左侧到左侧边线 - 暗绿色
    if (layout.sideLineLeftSideX > 0) {
        cv::rectangle(img, 
                     cv::Point(0, static_cast<int>(pageBottom)),
                     cv::Point(static_cast<int>(layout.sideLineLeftSideX), layout.pageHeight),
                     COLOR_LEFT_SIDE_BG, -1);
    }
    
    // 右侧区域：从右侧边线到最右侧 - 暗紫色
    if (layout.pageWidth > layout.sideLineRightSideX) {
        cv::rectangle(img,
                     cv::Point(static_cast<int>(layout.sideLineRightSideX), static_cast<int>(pageBottom)),
                     cv::Point(layout.pageWidth, layout.pageHeight),
                     COLOR_RIGHT_SIDE_BG, -1);
    }
    
    // 绘制线条
    int bottomLineWidth = std::max(1, static_cast<int>(std::round(options.scale * BOTTOM_LINE_WIDTH)));
    int sideLineWidth = std::max(1, static_cast<int>(std::round(options.scale * SIDE_LINE_WIDTH)));
    int barLineWidth = std::max(1, static_cast<int>(std::round(options.scale * BAR_LINE_WIDTH)));
    int semiBarLineWidth = std::max(1, static_cast<int>(std::round(options.scale * SEMI_BAR_LINE_WIDTH)));
    
    /* 不要在drawPage中绘制小节线，改为在drawBoard中绘制跨页小节线，避免重复绘制
       只需绘制半小节线作为全图参考。*/

	// 绘制顺序: 半小节线 -> 底部线 -> 顶部线 -> 侧线

    // 绘制半小节线 (semi bar lines)
    double scaledBarHeight = layout.barHeight * options.scale;
    for (int i = 0; i < options.timeLimit + options.barSpan; i += options.barSpan) {

        if (options.semiBarSpan > 0) {
            // 顶部也要画半小节线。故终止条件为<=
            for (double j = options.semiBarSpan; j <= options.barSpan; j += options.semiBarSpan) {
                double semiY = layout.bottomLineY - scaledBarHeight * (i + j);
                cv::line(img, cv::Point(0, static_cast<int>(semiY)),
                    cv::Point(layout.pageWidth, static_cast<int>(semiY)),
                    COLOR_SEMI_BAR_LINE, semiBarLineWidth);
            }
        }
    }

    // 底部线
    cv::line(img, cv::Point(0, layout.bottomLineY), 
             cv::Point(layout.pageWidth, layout.bottomLineY),
             COLOR_LINE, bottomLineWidth);
    
    // 顶部线
    cv::line(img, cv::Point(0, layout.bottomLineY - layout.barHeight * options.timeLimit * options.scale),
             cv::Point(layout.pageWidth, layout.bottomLineY - layout.barHeight * options.timeLimit * options.scale),
             COLOR_LINE, bottomLineWidth);
    
    // 侧线 (左边蓝色，中间白色，右边红色)
    // 侧线从 pageBottom 画到 bottomLineY
    double topY = layout.bottomLineY - layout.barHeight * options.timeLimit * options.scale;
    
    // Left Side 左边界 - 绿色 (BGR: 255,255,0 + Alpha:255)
    cv::line(img, cv::Point(layout.sideLineLeftSideX, static_cast<int>(pageBottom)),
             cv::Point(layout.sideLineLeftSideX, static_cast<int>(layout.bottomLineY)),
             cv::Scalar(0, 255, 0, 255), sideLineWidth);
    
    // Left Side 右边界 , Front 左边界 - 白色 (BGR: 255,255,255 + Alpha:255)
    cv::line(img, cv::Point(layout.sideLineLeftX, static_cast<int>(pageBottom)),
             cv::Point(layout.sideLineLeftX, static_cast<int>(layout.bottomLineY)),
             cv::Scalar(255, 255, 255, 255), sideLineWidth);
    
    // Front 右边界 , Right Side 左边界 - 白色 (BGR: 255,255,255 + Alpha:255)
    cv::line(img, cv::Point(layout.sideLineRightX, static_cast<int>(pageBottom)),
             cv::Point(layout.sideLineRightX, static_cast<int>(layout.bottomLineY)),
             cv::Scalar(255, 255, 255, 255), sideLineWidth);
    
    // Right Side 右边界 - 紫色 (BGR: 255,50,255 + Alpha:255)
    cv::line(img, cv::Point(layout.sideLineRightSideX, static_cast<int>(pageBottom)),
             cv::Point(layout.sideLineRightSideX, static_cast<int>(layout.bottomLineY)),
             cv::Scalar(255, 50, 255, 255), sideLineWidth);
    
	

    
    return img;
}

cv::Mat DynachartRenderer::drawBoard(const chart_store& chart, const Options& options, PageLayout& layout) {
    double maxTime = getMaxTime(chart);
    int pages = static_cast<int>(std::ceil(maxTime / options.timeLimit));
    
    if (pages <= 0) pages = 1;
    
    // 绘制单页
    cv::Mat pageImg = drawPage(options, layout);
    
    int pageWidth = layout.pageWidth;
    int pageHeight = layout.pageHeight;
    
    // 确保 pageImg 大小正确
    if (pageImg.empty() || pageImg.cols != pageWidth || pageImg.rows != pageHeight) {
        // 如果 pageImg 大小不对，重新创建
        pageImg = cv::Mat::zeros(pageHeight, pageWidth, CV_8UC4);
        pageImg.setTo(COLOR_BACKGROUND);
    }
    
    // 创建多页图像
    cv::Mat board = cv::Mat::zeros(pageHeight, pageWidth * pages, CV_8UC4);
    board.setTo(COLOR_BACKGROUND);
    
    // 粘贴每一页 (添加边界检查)
    for (int i = 0; i < pages; i++) {
        int x = i * pageWidth;
        if (x >= board.cols) continue;
        
        int copyWidth = std::min(pageWidth, board.cols - x);
        cv::Rect srcRect(0, 0, copyWidth, pageHeight);
        cv::Rect dstRect(x, 0, copyWidth, pageHeight);
        
        if (dstRect.y + dstRect.height <= board.rows) {
            pageImg(srcRect).copyTo(board(dstRect));
        }
    }
    
    // 绘制分割线
    int splitLineWidth = std::max(1, static_cast<int>(std::round(options.scale * SPLIT_LINE_WIDTH)));
    for (int i = 1; i < pages; i++) {
        cv::line(board, cv::Point(i * pageWidth, 0),
                 cv::Point(i * pageWidth, pageHeight),
                 COLOR_SPLIT_LINE, splitLineWidth);
    }
    
    // 绘制跨页小节线
    int barLineWidth = std::max(1, static_cast<int>(std::round(options.scale * BAR_LINE_WIDTH)));
    int semiBarLineWidth = std::max(1, static_cast<int>(std::round(options.scale * SEMI_BAR_LINE_WIDTH)));
    double scaledBarHeight = layout.barHeight * options.scale;

    // 绘制小节线（跳过页面边界线）
    for (int i = 0; i < pages * options.timeLimit; i += options.barSpan) {
        int pg = i / options.timeLimit;

        // 边界检查：确保页码在有效范围内
        if (pg >= pages) continue;

        int x = pageWidth * pg;
        double y = layout.bottomLineY - scaledBarHeight * (i - pg * options.timeLimit);

        // 小节线（跳过页面边界线）
        if (i % options.timeLimit != 0) {
            // 确保线条只绘制到当前页右边界
            int endX = x + pageWidth;
            cv::line(board, cv::Point(x, static_cast<int>(y)),
                cv::Point(endX, static_cast<int>(y)),
                COLOR_BAR_LINE, barLineWidth);
        }
    }

    return board;
}

void DynachartRenderer::drawNotes(cv::Mat& board, const PageLayout& layout,
                                 const std::vector<RenderNote>& notes, 
                                 const Options& options,
                                 const std::function<void(int, int)>& progressCallback) {
    double barHeight = layout.barHeight * options.scale;
    
    cv::Mat boardFront = cv::Mat::zeros(board.size(), CV_8UC4);
    
    int totalNotes = notes.size();
    int processedNotes = 0;
    
    for (const auto& note : notes) {
        double widthPerUnit = NOTE_SIZE * options.scale;
        if (note.side == 0) {  // FRONT
            widthPerUnit *= FRONT_NOTE_RATE;
        }
        
        // 计算 X 坐标 (参考 Python 项目 lib/chart.py draw_notes 函数)
        double x;
        if (note.side == -1) {  // LEFT
            // LEFT 侧：pos 越大越靠左
            x = (SIDE_VISIBLE_CAP - note.pos) * BOARD_SIZE * options.scale;
        } else if (note.side == 0) {  // FRONT
            // FRONT 侧：从 sideLineLeftX 开始
            double noteXInBoard = (note.pos - FRONT_LEFT_BORDER) * BOARD_SIZE * FRONT_BOARD_RATE * options.scale;
            x = layout.sideLineLeftX + noteXInBoard;
        } else {  // RIGHT
            // RIGHT 侧：从 sideLineRightSideX 开始
            x = layout.sideLineRightSideX + (note.pos - SIDE_BORDER) * BOARD_SIZE * options.scale;
        }
        
        // 跳过完全在画面外的音符
        if (x < -100 || x > board.cols + 100) continue;
        
        int pageNumber = static_cast<int>(note.start / options.timeLimit);
        int endPageNumber = pageNumber;
        if (note.type == 2) {  // HOLD
            endPageNumber = static_cast<int>(note.end / options.timeLimit);
        }
        
        // 生成音符图像
        cv::Mat noteImg = generateNoteImage(note, widthPerUnit, barHeight, options);
        
        if (noteImg.empty()) continue;  // 跳过空图像
        
        // 更新进度
        processedNotes++;
        if (progressCallback) {
            progressCallback(processedNotes, totalNotes);
        }
        
        double widthHold2 = std::max(1.0, std::round(options.scale * NOTE_WIDTH_HOLD / 2.0));
        
        if (pageNumber == endPageNumber) {
            x += pageNumber * layout.pageWidth;
            double y = layout.bottomLineY - (note.start - pageNumber * options.timeLimit) * barHeight;
            int realX = static_cast<int>(x - noteImg.cols / 2.0);
            int realY;
            
            if (note.type == 2) {  // HOLD
                realY = static_cast<int>(y - noteImg.rows + widthHold2);
            } else {
                realY = static_cast<int>(y - noteImg.rows / 2.0);
            }
            
            // 提取 alpha 通道作为 mask
            cv::Mat mask;
            if (noteImg.channels() == 4) {
                std::vector<cv::Mat> channels;
                cv::split(noteImg, channels);
                mask = channels[3];
            }
            
            // 边界检查：确保 ROI 在矩阵范围内
            int clippedRealX = std::max(0, std::min(realX, board.cols - 1));
            int clippedRealY = std::max(0, std::min(realY, board.rows - 1));
            int clippedWidth = std::min(noteImg.cols, std::max(0, board.cols - clippedRealX));
            int clippedHeight = std::min(noteImg.rows, std::max(0, board.rows - clippedRealY));
            
            if (clippedWidth > 0 && clippedHeight > 0) {
                if (note.type == 2) {  // HOLD 画在 board 上（底层）
                    cv::Rect srcRect(std::max(0, -realX), std::max(0, -realY), clippedWidth, clippedHeight);
                    cv::Rect dstRect(clippedRealX, clippedRealY, clippedWidth, clippedHeight);
                    noteImg(srcRect).copyTo(board(dstRect), mask(srcRect));
                } else {  // 其他画在 boardFront 上（顶层，会在最后覆盖到 board 上）
                    cv::Rect srcRect(std::max(0, -realX), std::max(0, -realY), clippedWidth, clippedHeight);
                    cv::Rect dstRect(clippedRealX, clippedRealY, clippedWidth, clippedHeight);
                    noteImg(srcRect).copyTo(boardFront(dstRect), mask(srcRect));
                }
            }
        } else {
            // HOLD 音符跨页处理 - 参考 Python 的 draw_notes 函数逻辑
            // Python: start_crop = img.crop((0, img.height - start_clip, img.width, img.height))
            // Python: end_crop = img.crop((0, 0, img.width, end_clip))
            int capY = board.rows - static_cast<int>(layout.bottomLineY);
            int endY = static_cast<int>(layout.bottomLineY - (note.end - endPageNumber * options.timeLimit) * barHeight - widthHold2);
            int startClip = static_cast<int>(((pageNumber + 1) * options.timeLimit - note.start) * barHeight + widthHold2);
            int endClip = static_cast<int>((note.end - endPageNumber * options.timeLimit) * barHeight + widthHold2);
            
            // 确保裁剪值为正数
            startClip = std::max(1, startClip);
            endClip = std::max(1, endClip);
            
            // 裁剪 start 部分：从底部裁剪 startClip 高度 (参考 Python: img.crop((0, img.height - start_clip, img.width, img.height)))
            int startRectY = std::max(0, noteImg.rows - startClip);
            int startRectH = noteImg.rows - startRectY;
            
            // 裁剪 end 部分：从顶部裁剪 endClip 高度 (参考 Python: img.crop((0, 0, img.width, end_clip)))
            int endRectH = std::min(endClip, noteImg.rows);
            
            // 只有当裁剪区域有效时才进行裁剪
            if (startRectH > 0 && endRectH > 0) {
                cv::Rect startRect(0, startRectY, noteImg.cols, startRectH);
                cv::Rect endRect(0, 0, noteImg.cols, endRectH);
                
                // 确保裁剪矩形有效
                if (startRect.x >= 0 && startRect.y >= 0 && startRect.width > 0 && startRect.height > 0 &&
                    startRect.x + startRect.width <= noteImg.cols && startRect.y + startRect.height <= noteImg.rows &&
                    endRect.x >= 0 && endRect.y >= 0 && endRect.width > 0 && endRect.height > 0 &&
                    endRect.x + endRect.width <= noteImg.cols && endRect.y + endRect.height <= noteImg.rows) {
                    
                    cv::Mat startCrop = noteImg(startRect).clone();
                    cv::Mat endCrop = noteImg(endRect).clone();
                    
                    // 提取 alpha 通道
                    cv::Mat startMask, endMask;
                    if (startCrop.channels() == 4) {
                        std::vector<cv::Mat> channels;
                        cv::split(startCrop, channels);
                        startMask = channels[3];
                    }
                    if (endCrop.channels() == 4) {
                        std::vector<cv::Mat> channels;
                        cv::split(endCrop, channels);
                        endMask = channels[3];
                    }
                    
                    // 计算 start 和 end 的 X 坐标
                    int startX = static_cast<int>(x + pageNumber * layout.pageWidth - noteImg.cols / 2.0);
                    int endX = static_cast<int>(x + endPageNumber * layout.pageWidth - noteImg.cols / 2.0);
                    
                    // 边界检查：确保粘贴位置在矩阵范围内
                    int startDstX = std::max(0, std::min(startX, board.cols - startCrop.cols));
                    int endDstX = std::max(0, std::min(endX, board.cols - endCrop.cols));
                    
                    // 粘贴 start 部分
                    if (capY >= 0 && capY + startCrop.rows <= board.rows) {
                        cv::Rect dstRect(startDstX, capY, startCrop.cols, startCrop.rows);
                        if (dstRect.x + dstRect.width <= board.cols && dstRect.y + dstRect.height <= board.rows) {
                            startCrop.copyTo(board(dstRect), startMask.empty() ? cv::noArray() : startMask);
                        }
                    }
                    
                    // 粘贴 end 部分
                    int actualEndY = std::max(0, std::min(endY, board.rows - endCrop.rows));
                    if (endY >= 0 && endY + endCrop.rows <= board.rows) {
                        cv::Rect dstRect(endDstX, actualEndY, endCrop.cols, endCrop.rows);
                        if (dstRect.x + dstRect.width <= board.cols && dstRect.y + dstRect.height <= board.rows) {
                            endCrop.copyTo(board(dstRect), endMask.empty() ? cv::noArray() : endMask);
                        }
                    }
                    
                    // 处理中间页面
                    double pagesize = options.timeLimit * barHeight;
                    for (int pg = pageNumber + 1; pg < endPageNumber; pg++) {
                        double cy = noteImg.rows - startClip - (pg - pageNumber) * pagesize;
                        int cropY = static_cast<int>(cy);
                        int cropH = static_cast<int>(pagesize);
                        
                        if (cropY < 0) cropY = 0;
                        if (cropY + cropH > noteImg.rows) cropH = noteImg.rows - cropY;
                        if (cropH <= 0) continue;
                        
                        cv::Rect cropRect(0, cropY, noteImg.cols, cropH);
                        cv::Mat crop = noteImg(cropRect).clone();
                        
                        cv::Mat cropMask;
                        if (crop.channels() == 4) {
                            std::vector<cv::Mat> channels;
                            cv::split(crop, channels);
                            cropMask = channels[3];
                        }
                        
                        int pgX = static_cast<int>(x + pg * layout.pageWidth - noteImg.cols / 2.0);
                        int pgDstX = std::max(0, std::min(pgX, board.cols - crop.cols));
                        
                        if (capY >= 0 && capY + crop.rows <= board.rows) {
                            cv::Rect dstRect(pgDstX, capY, crop.cols, crop.rows);
                            if (dstRect.x + dstRect.width <= board.cols && dstRect.y + dstRect.height <= board.rows) {
                                crop.copyTo(board(dstRect), cropMask.empty() ? cv::noArray() : cropMask);
                            }
                        }
                    }
                }  // 内层 if 闭合
            }
        }
    }
    
    // 将 boardFront 粘贴到 board 上（boardFront 包含非 HOLD 音符，应该在 HOLD 之上）
    if (!boardFront.empty()) {
        cv::Mat mask;
        std::vector<cv::Mat> channels;
        cv::split(boardFront, channels);
        mask = channels[3];  // 使用 alpha 通道作为 mask
        
        // 边界检查：确保 boardFront 不超出 board 范围
        int copyWidth = std::min(boardFront.cols, board.cols);
        int copyHeight = std::min(boardFront.rows, board.rows);
        
        if (copyWidth > 0 && copyHeight > 0) {
            cv::Rect srcRect(0, 0, copyWidth, copyHeight);
            cv::Rect dstRect(0, 0, copyWidth, copyHeight);
            boardFront(srcRect).copyTo(board(dstRect), mask(srcRect));
        }
    }
}

void DynachartRenderer::drawTimeMarker(cv::Mat& img, int pageX, int y, int barIndex,
                                      const chart_store& chart, const Options& options,
                                      const PageLayout& layout) {
    if (!freeTypeAvailable_ || !ftFace_) {
        std::cerr << "[WARNING] drawTimeMarker skipped: freeTypeAvailable_=" << freeTypeAvailable_ 
                  << ", ftFace_=" << (ftFace_ ? "valid" : "null") << std::endl;
        return;
    }
    
    // 计算时间（毫秒）
    double timeMs = (barIndex / chart.get_barpm() * 60.0 - chart.get_offset()) * 1000.0;
    
    int h = static_cast<int>(timeMs / 3600000);
    int m = static_cast<int>((static_cast<long long>(timeMs) % 3600000) / 60000);
    int s = static_cast<int>((static_cast<long long>(timeMs) % 60000) / 1000);
    int ms = static_cast<int>(static_cast<long long>(timeMs) % 1000);
    
    // 分别构建时间字符串和小节号字符串
    std::ostringstream timeStr, barStr;
    if (h == 0) {
        timeStr << std::setfill('0') << std::setw(2) << m << ":"
             << std::setw(2) << s << ":"
             << std::setw(3) << ms;
    } else {
        timeStr << std::setfill('0') << std::setw(2) << h << ":"
             << std::setw(2) << m << ":"
             << std::setw(2) << s << ":"
             << std::setw(3) << ms;
    }
    barStr << " " << barIndex;
    
    std::string timeText = timeStr.str();
    std::string barText = barStr.str();
    
    // 字体大小增加 50%
    int fontHeight = static_cast<int>(std::round(FONT_SIZE * options.scale * 1.5));
    FT_Set_Pixel_Sizes(ftFace_, 0, fontHeight);
    
    // 渲染每个字符
    int cursorX = pageX + 5;  // 小节线右侧 5 像素
    int textY = y - 5;        // 底部线上方 5 像素
    int renderedChars = 0;
    
    std::cout << "[Info] Rendering text: '" << timeText << " " << barIndex << "' at (" << pageX << ", " << y << "), fontHeight=" << fontHeight << std::endl;
    
    // 设置字体大小
    FT_Error error = FT_Set_Pixel_Sizes(ftFace_, 0, fontHeight);
    if (error != 0) {
        std::cerr << "[ERROR] FT_Set_Pixel_Sizes failed: error=" << error << std::endl;
    }
    //std::cout << "[DEBUG] FT_Set_Pixel_Sizes result: error=" << error << ", face->size->height=" << ftFace_->size->metrics.height << std::endl;
    
    // 时间部分（white, per character）
    for (char c : timeText) {
        // 使用 FT_LOAD_RENDER 让 FreeType 自动渲染 glyph 到 bitmap
        FT_Error loadError = FT_Load_Char(ftFace_, c, FT_LOAD_RENDER);
        if (loadError != 0) {
            std::cerr << "[WARNING] FT_Load_Char failed for '" << c << "': error=" << loadError << std::endl;
            continue;
        }
        
        FT_Bitmap* bitmap = &ftFace_->glyph->bitmap;
        int bitmapWidth = bitmap->width;
        int bitmapHeight = bitmap->rows;
        int bitmapLeft = ftFace_->glyph->bitmap_left;
        int bitmapTop = ftFace_->glyph->bitmap_top;
		
        /*
        std::cout << "[DEBUG] Char '" << c << "': loadError=" << loadError 
                  << ", bitmap=" << bitmapWidth << "x" << bitmapHeight 
                  << ", bitmap_left=" << bitmapLeft << ", bitmap_top=" << bitmapTop
                  << ", advance=" << (ftFace_->glyph->advance.x >> 6) << std::endl;
        */
        int charX = cursorX + bitmapLeft;
        int charY = textY - bitmapTop;
        
        // bitmap 尺寸检查
        if (bitmapWidth == 0 || bitmapHeight == 0) {
            //std::cout << "[DEBUG] Char '" << c << "': skipped (empty bitmap)" << std::endl;
            cursorX += ftFace_->glyph->advance.x >> 6;
            continue;
        }
        
        //std::cout << "[DEBUG] Char '" << c << "': pos=(" << charX << "," << charY << "), img_size=" << img.cols << "x" << img.rows << std::endl;
        
        // 确保在图像范围内
        if (charX < 0 || charY < 0 || charX + bitmapWidth > img.cols || charY + bitmapHeight > img.rows) {
            //std::cout << "[DEBUG] Char '" << c << "' skipped: out of bounds" << std::endl;
            cursorX += ftFace_->glyph->advance.x >> 6;
            continue;
        }
        
        // 将 FreeType bitmap 复制到 cv::Mat
        int pixelsWritten = 0;
        for (int row = 0; row < bitmapHeight; ++row) {
            int imgY = charY + row;
            if (imgY < 0 || imgY >= img.rows) continue;
            
            for (int col = 0; col < bitmapWidth; ++col) {
                int imgX = charX + col;
                if (imgX < 0 || imgX >= img.cols) continue;
                
                int bitmapIndex = row * bitmap->width + col;
                unsigned char alpha = bitmap->buffer[bitmapIndex];
                
                if (alpha > 0) {
                    // 白色文字
                    img.at<cv::Vec4b>(imgY, imgX)[0] = 255;  // B
                    img.at<cv::Vec4b>(imgY, imgX)[1] = 255;  // G
                    img.at<cv::Vec4b>(imgY, imgX)[2] = 255;  // R
                    img.at<cv::Vec4b>(imgY, imgX)[3] = alpha; // A
                    
                    pixelsWritten++;
                }
            }
        }
        
		// 如果 bitmap 不空但没有写入任何像素，输出调试信息
        if (pixelsWritten == 0 && bitmapWidth > 0 && bitmapHeight > 0) {
            std::cout << "[Info] Char '" << c << "': bitmap not empty but all pixels alpha=0" << std::endl;
        }
        renderedChars++;
        cursorX += ftFace_->glyph->advance.x >> 6;
        
    }
    
    // 小节号（green, per character）
    for (char c : barText) {
        FT_Error loadError = FT_Load_Char(ftFace_, c, FT_LOAD_RENDER);
        if (loadError != 0) {
            std::cerr << "[WARNING] FT_Load_Char failed for '" << c << "': error=" << loadError << std::endl;
            continue;
        }
        
        FT_Bitmap* bitmap = &ftFace_->glyph->bitmap;
        int bitmapWidth = bitmap->width;
        int bitmapHeight = bitmap->rows;
        int bitmapLeft = ftFace_->glyph->bitmap_left;
        int bitmapTop = ftFace_->glyph->bitmap_top;
        
        int charX = cursorX + bitmapLeft;
        int charY = textY - bitmapTop;
        
        // bitmap 尺寸检查
        if (bitmapWidth == 0 || bitmapHeight == 0) {
            cursorX += ftFace_->glyph->advance.x >> 6;
            continue;
        }
        
        // 确保在图像范围内
        if (charX < 0 || charY < 0 || charX + bitmapWidth > img.cols || charY + bitmapHeight > img.rows) {
            cursorX += ftFace_->glyph->advance.x >> 6;
            continue;
        }
        
        // 将 FreeType bitmap 复制到 cv::Mat
        int pixelsWritten = 0;
        for (int row = 0; row < bitmapHeight; ++row) {
            int imgY = charY + row;
            if (imgY < 0 || imgY >= img.rows) continue;
            
            for (int col = 0; col < bitmapWidth; ++col) {
                int imgX = charX + col;
                if (imgX < 0 || imgX >= img.cols) continue;
                
                int bitmapIndex = row * bitmap->width + col;
                unsigned char alpha = bitmap->buffer[bitmapIndex];
                
                if (alpha > 0) {
                    // 绿色文字（小节号部分）
                    img.at<cv::Vec4b>(imgY, imgX)[0] = 0;    // B
                    img.at<cv::Vec4b>(imgY, imgX)[1] = 255;  // G
                    img.at<cv::Vec4b>(imgY, imgX)[2] = 0;    // R
                    img.at<cv::Vec4b>(imgY, imgX)[3] = alpha; // A
                    
                    pixelsWritten++;
                }
            }
        }
        
        if (pixelsWritten == 0 && bitmapWidth > 0 && bitmapHeight > 0) {
            std::cout << "[Info] Char '" << c << "': bitmap not empty but all pixels alpha=0" << std::endl;
        }
        
        renderedChars++;
        cursorX += ftFace_->glyph->advance.x >> 6;
    }
    
    std::cout << "[Info] Finished rendering time marker, character count: " << renderedChars << std::endl;
}

cv::Mat DynachartRenderer::render(const chart_store& chart, const Options& options) {
    PageLayout layout;
    
    // 绘制棋盘框架（不包含时间标记）
    cv::Mat board = drawBoard(chart, options, layout);
    
    
    
    // 转换音符
    std::vector<RenderNote> notes = convertNotes(chart);
    std::cout << "Convert complete. Start rendering......" << std::endl;
    // 绘制音符
    drawNotes(board, layout, notes, options, options.progressCallback);
    


    // 重新绘制时间标记（在音符之上）
    double maxTime = getMaxTime(chart);
    int pages = static_cast<int>(std::ceil(maxTime / options.timeLimit));
    if (pages <= 0) pages = 1;
    
    int pageWidth = layout.pageWidth;
    double scaledBarHeight = layout.barHeight * options.scale;
    
    for (int i = 0; i < pages * options.timeLimit; i += options.barSpan) {
        int pg = i / options.timeLimit;
        
        // 边界检查：确保页码在有效范围内
        if (pg >= pages) continue;
        
        int x = pageWidth * pg;
        double y = layout.bottomLineY - scaledBarHeight * (i - pg * options.timeLimit);
        
        // 在每个小节线处都显示时间标注
        drawTimeMarker(board, x, static_cast<int>(y), i, chart, options, layout);
    }

	

    return board;
}

bool DynachartRenderer::generate(const chart_store& chart,
                                const std::string& outputPath,
                                const Options& options) {
    try {
        cv::Mat image = render(chart, options);
        std::cout << "\r" << std::string(80, ' ') << "\r"; // 清除进度条
        std::cout << "\rRendering complete. Saving image......" << std::endl;
        // 保存图像
        std::vector<int> params;
        params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        params.push_back(9);
        
        bool success = cv::imwrite(outputPath, image, params);
        
        if (!success) {
            std::cerr << "Failed to save image to: " << outputPath << std::endl;
        }
        
        return success;
    } catch (const std::exception& e) {
        std::cerr << "Error generating image: " << e.what() << std::endl;
        return false;
    }
}
