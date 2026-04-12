# Dynachart Workspace

音乐游戏谱面处理项目，包含谱面解析和图片生成两个独立库。

## 项目结构

```
workspace/
├── ChartStore/              # 谱面存储与解析库
│   ├── include/
│   │   ├── chart_store.h    # 谱面类定义
│   │   └── defs.h           # 全局定义
│   ├── src/
│   │   └── chart_store.cpp  # 谱面解析实现
│   └── CMakeLists.txt
│
├── ImageGenerator/          # 图片生成库
│   ├── include/
│   │   └── image_generator.h
│   ├── src/
│   │   └── image_generator.cpp
│   └── CMakeLists.txt
│
├── examples/                # 示例程序
│   └── chart_viewer.cpp
│
├── CMakeLists.txt           # 根 CMake 配置
└── README.md
```

## 库说明

### ChartStore 库

谱面解析库，提供 XML 格式的 Dynamix 谱面文件解析功能。

**主要功能：**
- 解析 XML 格式的谱面文件
- 支持多种音符类型：NORMAL, CHAIN, HOLD, SUB
- 支持左右中三侧音符
- 支持 BPM 变化
- 谱面数据验证（Hold-Sub 匹配检查等）

**核心类：**
- `chart_store` - 谱面数据存储类

**使用示例：**
```cpp
#include "chart_store.h"

chart_store chart;
int result = chart.readfile("chart.xml");

std::cout << "Center notes: " << chart.get_mid_count() << std::endl;
std::cout << "Left notes: " << chart.get_left_count() << std::endl;
std::cout << "Right notes: " << chart.get_right_count() << std::endl;
```

### ImageGenerator 库

图片生成库，基于 ChartStore 解析的数据生成谱面可视化图片。

**主要功能：**
- 生成单侧谱面图片
- 生成完整谱面图片（三侧合并）
- 支持自定义样式（缩放、颜色、网格等）
- 支持 PNG 格式输出

**核心类：**
- `ImageGenerator` - 图片生成器类

**使用示例：**
```cpp
#include "image_generator.h"

ImageGenerator generator;
ImageGenerator::Options options;
options.scale = 1.0;
options.barHeight = 50.0;

generator.generateFromChart(chart, "output.png", options);
```

## 构建项目

### 前置要求

- CMake 3.15+
- C++14 兼容编译器
- OpenCV 4.x
- MSVC (Windows) 或 GCC/Clang (Linux/macOS)

### Windows (MSVC)

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake .. -G "Visual Studio 16 2019"

# 编译
cmake --build . --config Release
```

### Linux/macOS

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
make -j4
```

## 使用示例

### 命令行工具

```bash
# 编译后运行示例程序
./bin/chart_viewer input.xml output.png
```

### C++ 代码集成

```cpp
#include "chart_store.h"
#include "image_generator.h"

int main() {
    // 1. 读取谱面
    chart_store chart;
    chart.readfile("chart.xml");
    
    // 2. 生成图片
    ImageGenerator gen;
    ImageGenerator::Options opts;
    opts.scale = 1.5;
    
    gen.generateFromChart(chart, "chart.png", opts);
    
    return 0;
}
```

## 依赖库

- **OpenCV** - 图像处理
- **C++ Standard Library** - C++14

## 与旧项目对比

### Dynachart-Cpp

原项目将所有功能混合在一起，新架构的优势：

1. **模块化** - ChartStore 和 ImageGenerator 独立，可单独使用
2. **可维护性** - 清晰的职责分离
3. **可复用性** - ChartStore 可在其他项目中使用
4. **易测试** - 各模块可独立测试

### 迁移指南

如果你正在使用旧的 Dynachart-Cpp 项目：

1. 将 `chart_store.h/cpp` 和 `defs.h` 复制到 `ChartStore/include` 和 `ChartStore/src`
2. 将图片生成代码迁移到 `ImageGenerator` 目录
3. 更新 CMakeLists.txt 使用新的库结构
4. 修改 include 路径引用

## 开发计划

- [ ] 添加更多音符类型支持
- [ ] 支持 SVG 矢量图输出
- [ ] 添加 BPM 变化可视化
- [ ] 支持自定义主题和样式
- [ ] 添加批量处理功能

## 许可证

本项目由 AXIS5 开发，保留所有权利。

## 联系方式

开发者：AXIS5
