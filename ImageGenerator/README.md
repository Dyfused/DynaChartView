# ImageGenerator Library

基于 ChartStore 的谱面可视化图片生成库。

## 功能特性

- 生成谱面可视化图片
- 支持单侧/完整谱面生成
- 自定义样式（缩放、颜色、布局）
- PNG 格式输出
- 网格和时间标记

## 核心类

### ImageGenerator

```cpp
class ImageGenerator {
public:
    // 生成选项
    struct Options {
        double scale = 1.0;           // 缩放比例
        double barHeight = 50.0;      // 每 bar 高度（像素）
        double noteWidth = 40.0;      // 音符宽度（像素）
        int padding = 50;             // 边距（像素）
        bool showGrid = true;         // 显示网格
        bool showTime = true;         // 显示时间标记
        std::string outputFormat = "png";
    };

    // 生成完整谱面图片
    bool generateFromChart(const chart_store& chart, 
                          const std::string& outputPath,
                          const Options& options = Options());
    
    // 生成单侧谱面
    cv::Mat generateLeftSide(const chart_store& chart, const Options& options);
    cv::Mat generateCenterSide(const chart_store& chart, const Options& options);
    cv::Mat generateRightSide(const chart_store& chart, const Options& options);
    
    // 生成三侧合并谱面
    cv::Mat generateFullChart(const chart_store& chart, const Options& options);
};
```

## 使用示例

### 基本用法

```cpp
#include "chart_store.h"
#include "image_generator.h"

int main() {
    // 1. 读取谱面
    chart_store chart;
    chart.readfile("chart.xml");
    
    // 2. 创建生成器
    ImageGenerator generator;
    
    // 3. 设置选项
    ImageGenerator::Options options;
    options.scale = 1.5;        // 1.5 倍缩放
    options.barHeight = 60.0;   // 每 bar 60 像素
    options.noteWidth = 50.0;   // 音符宽度 50 像素
    options.showGrid = true;
    options.showTime = true;
    
    // 4. 生成图片
    generator.generateFromChart(chart, "output.png", options);
    
    return 0;
}
```

### 生成单侧谱面

```cpp
// 只生成左侧谱面
cv::Mat left = generator.generateLeftSide(chart, options);
cv::imwrite("left.png", left);

// 只生成中间谱面
cv::Mat center = generator.generateCenterSide(chart, options);
cv::imwrite("center.png", center);

// 只生成右侧谱面
cv::Mat right = generator.generateRightSide(chart, options);
cv::imwrite("right.png", right);
```

### 自定义样式

```cpp
// 修改颜色（需要修改源码或扩展 API）
// 默认颜色：
// - 背景：白色 (255, 255, 255)
// - 网格：灰色 (200, 200, 200)
// - 普通音符：橙黄色 (0, 128, 255)
// - 连打音符：浅蓝色 (0, 200, 255)
// - 长按音符：绿色 (0, 180, 100)
// - 文字：黑色 (0, 0, 0)
```

## 音符类型与颜色映射

| 音符类型 | 颜色 | RGB 值 |
|---------|------|-------|
| NORMAL | 橙黄色 | (0, 128, 255) |
| CHAIN | 浅蓝色 | (0, 200, 255) |
| HOLD | 绿色 | (0, 180, 100) |

## 坐标系统

- **时间轴**：垂直方向，从上到下递增
- **位置轴**：水平方向，-1（左）~ 0（中）~ 1（右）
- **单位**：时间以 bar 为单位，位置以相对值表示

## 输出图片规格

### 完整谱面（三侧）

```
┌─────────────────────────────────────────────┐
│  时间标记 │ 左 │ 中 │ 右 │                   │
│    │     │   │   │   │                      │
│    │  ───│───│───│───│───  (网格线)         │
│    │  ───│───│───│───│───                   │
│    │  ───│───│───│───│───                   │
└─────────────────────────────────────────────┘
```

### 图片尺寸计算

```cpp
// 宽度 = 音符宽度 × 6 + 边距 × 4
int width = noteWidth * 6 + padding * 4;

// 高度 = 最大时间 × barHeight + 边距
int height = maxTime * barHeight + padding;
```

## 依赖库

- **OpenCV 4.x** - 图像处理核心库
- **ChartStore** - 谱面数据源

## 构建

```bash
# 确保 ChartStore 已构建
cd ChartStore && mkdir build && cd build
cmake ..
cmake --build . --config Release

# 构建 ImageGenerator
cd ../..
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## 集成到其他项目

### CMake

```cmake
find_package(OpenCV REQUIRED)
find_library(CHARTSTORE ChartStore PATHS ...)

add_subdirectory(ImageGenerator)
target_link_libraries(your_target PRIVATE ImageGenerator)
```

### 手动配置

```bash
# Windows (MSVC)
cl /I ImageGenerator\include /I ChartStore\include ^
   your_code.cpp /link ^
   ImageGenerator\lib\ImageGenerator.lib ^
   ChartStore\lib\ChartStore.lib ^
   opencv_world450.lib
```

## 性能优化建议

1. **缩放比例**：根据输出分辨率调整 `scale` 参数
2. **批处理**：一次性加载多个谱面，复用生成器实例
3. **缓存**：对相同谱面缓存生成的图片
4. **内存**：处理大谱面时注意 cv::Mat 的内存占用

## 常见问题

### Q: 生成的图片模糊？
A: 增加 `scale` 参数或 `barHeight` 值。

### Q: 音符显示不完整？
A: 检查 `maxTime` 计算是否正确，增加 `padding`。

### Q: 如何支持更多音符类型？
A: 在 `drawNotes()` 函数中添加新的颜色映射。

### Q: 支持其他输出格式？
A: 修改 `generateFromChart()` 中的编码参数，支持 JPEG、BMP 等。

## 扩展开发

### 添加自定义渲染器

```cpp
class CustomRenderer : public ImageGenerator {
protected:
    void drawNotes(cv::Mat& image, ...) override {
        // 自定义渲染逻辑
    }
};
```

### 添加滤镜效果

```cpp
void applyGlowEffect(cv::Mat& image) {
    cv::Mat blurred;
    cv::GaussianBlur(image, blurred, cv::Size(5,5), 0);
    // 添加光晕...
}
```

## 许可证

本项目由 AXIS5 开发，保留所有权利。
