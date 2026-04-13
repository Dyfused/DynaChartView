# DynaChartView

Dynamite音乐游戏谱面图片生成工具（C++ 实现）

## 项目结构

```
Chartstore_Project/
├── ChartStore/              # 谱面存储与解析库
│   ├── include/
│   │   ├── chart_store.h    # 谱面类定义
│   │   └── defs.h           # 全局定义
│   ├── src/
│   │   └── chart_store.cpp  # 谱面解析实现
│   └── CMakeLists.txt
│
├── ImageGenerator/          # 谱面图片生成库
│   ├── include/ 
│   │   └── dynachart_renderer.h
│   ├── src/ 
│   │   └── dynachart_renderer.cpp
│   └── CMakeLists.txt
│
├── src/                     # 主程序入口
│   └── main.cpp
│
├── CMakeLists.txt           # 根 CMake 配置
└── README.md
```

---

## 🔭 结果示例



## 🚀 使用方法

### 命令行工具

#### 基本用法

```bash
# Windows
.\build\Release\DynaChartView.exe input.xml output.png

# Linux/macOS
./build/DynaChartView input.xml output.png
```

#### 命令行参数

```
用法：DynaChartView <输入文件> <输出文件> [选项]

选项:
  -s <float>, --scale <float>        缩放比例 (默认：1.0)
  -S, --speed <float>   显示速度 (决定小节高度，默认：0.5)
  -l, --page-limit <int>    每页小节数 (默认：32)
  -f, --font <path>         字体路径 (默认：系统文件夹的arial.ttf)
  --help                 显示帮助信息

示例:
  DynaChartView chart.xml
  DynaChartView chart.xml chart.png
  DynaChartView chart.xml chart.png -s 1.2
  DynaChartView chart.xml chart.png -S 0.7 -l 24
  DynaChartView chart.xml chart.png -f /usr/share/fonts/arial.ttf
```

#### 缺少MSVC运行时？

release中提供的.exe安装包，安装后看安装文件夹，里面有vc_redist.x64.exe，运行即可安装MSVC运行时

### C++ API 集成

```cpp
#include "chart_store.h"
#include "dynachart_renderer.h"

int main() {
    // 1. 读取谱面
    chart_store chart;
    int result = chart.readfile("chart.xml");

    if (result != 0) {
        std::cerr << "Failed to load chart: " << result << std::endl;
        return 1;
    }

    // 2. 创建渲染器
    DynachartRenderer renderer;

    // 3. 设置选项
    DynachartRenderer::Options options;
    options.scale = 1.5;
    options.barHeight = 60.0;
    options.gridVisible = true;

    // 4. 生成图片
    renderer.render(chart, "output.png", options);

    return 0;
}
```

---

## 📦 编译方法

### Windows 端 (MSVC)

#### 前置要求

1. **Visual Studio 2019/2022** (包含 C++ 桌面开发)
2. **CMake 3.15+**
3. **vcpkg** (推荐，用于管理依赖)

#### 使用 vcpkg 安装依赖 (强烈推荐)

**vcpkg 是 Microsoft 开发的 C/C++ 包管理器，可自动解决 OpenCV 和 FreeType 依赖。**

```powershell
# 1. 克隆 vcpkg (在项目目录外或任意位置)
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# 2. 初始化 vcpkg
.\bootstrap-vcpkg.bat

# 3. 安装项目所需依赖
.\vcpkg install opencv4:x64-windows freetype:x64-windows

# 4. 将 vcpkg 集成到 Visual Studio (可选，但推荐)
.\vcpkg integrate install

# 完成后，CMake 会自动找到这些库
```

**vcpkg 安装说明：**

- `opencv4:x64-windows` - 图像处理库 (包含 core, imgcodecs, highgui 等模块)
- `freetype:x64-windows` - 字体渲染库 (用于时间戳和文本显示)

**CMake 配置时指定 vcpkg toolchain：**

```powershell
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="<Your vcpkg install path>\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>"
```

**静态链接 MSVC 运行时**

如果使用 vcpkg 静态版本，记得：

安装 -static 版本的库 (opencv4:x64-windows-static freetype:x64-windows-static)
可能需要定义 OPENCV_STATIC 宏

```
# 1. 安装静态依赖
.\vcpkg install opencv4:x64-windows-static freetype:x64-windows-static

# 2. 配置 CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE="<Your vcpkg install path>\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>"

# 3. 编译
cmake --build . --config Release
```



#### 手动安装依赖 (不推荐)

**方法 1：OpenCV 官方安装包 + FreeType 手动编译**

```powershell
# 下载 OpenCV 4.x Windows 安装包
# https://github.com/opencv/opencv/releases

# 解压后设置环境变量
$env:OPENCV_DIR = "C:\opencv\build\x64\vc15\lib"
$env:Path += ";C:\opencv\build\x64\vc15\bin"

# 注意：FreeType 需要单独下载源码编译，较麻烦
```

#### 编译步骤

```powershell
# 1. 进入项目目录
cd path/to/DynaChartView

# 2. 创建构建目录
if (Test-Path build) { Remove-Item -Recurse -Force build }
mkdir build
cd build

# 3. 配置项目 (自动检测 MSVC)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 或者指定生成器
# cmake .. -G "Visual Studio 17 2022" -A x64

# 4. 编译
cmake --build . --config Release

# 5. 运行程序
.\Release\chartstore.exe input.xml output.png
```

#### 静态链接 MSVC 运行时

如需生成不依赖 VCRedist 的独立可执行文件：

```powershell
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>"
```

---

### Linux 端

#### 前置要求

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libopencv-dev \
    libfreetype6-dev \
    git

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install -y cmake opencv-devel freetype-devel
```

#### 编译步骤

```bash
# 1. 进入项目目录
cd Chartstore_Project

# 2. 创建构建目录
mkdir -p build && cd build

# 3. 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. 编译 (使用所有 CPU 核心)
make -j$(nproc)

# 5. 运行程序
./chartstore input.xml output.png
```

#### 静态链接 (可选)

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++"
```

---

### macOS 端

#### 前置要求

```bash
# 安装 Homebrew (如果未安装)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装依赖
brew install cmake opencv freetype
```

#### 编译步骤

```bash
# 1. 进入项目目录
cd Chartstore_Project

# 2. 创建构建目录
mkdir -p build && cd build

# 3. 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. 编译
make -j$(sysctl -n hw.ncpu)

# 5. 运行程序
./chartstore input.xml output.png
```

---

# 文件编码

- **XML 编码**: UTF-8
- **输出图片**: PNG (RGBA)

---

## 🔧 常见问题

### Windows 编译问题

**问题 1: cmake 不是可识别的命令**

```powershell
# 解决方法：将 CMake 添加到 PATH
$env:Path += ";C:\Program Files\CMake\bin"
```

**问题 2: 找不到 OpenCV**

```powershell
# 设置 OpenCV 路径
$env:OpenCV_DIR = "C:\opencv\build"
cmake .. -DCMAKE_PREFIX_PATH="C:\opencv\build"
```

**问题 3: 缺少 MSVC 运行时**

```powershell
# 使用静态链接
cmake .. -DCMAKE_EXE_LINKER_FLAGS="/MT"
```

### Linux 编译问题

**问题 1: CMake 版本过低**

```bash
# 安装新版本
wget https://github.com/Kitware/CMake/releases/download/v3.24.0/cmake-3.24.0-linux-x86_64.tar.gz
tar -xzf cmake-3.24.0-linux-x86_64.tar.gz
sudo cp -r cmake-3.24.0-linux-x86_64/* /usr/local/
```

**问题 2: OpenCV 库找不到**

```bash
# 设置 OpenCV 路径
export OpenCV_DIR=/usr/lib/x86_64-linux-gnu/cmake/opencv4
```

### 运行时问题

**问题 1: 字体渲染不显示**

- Windows: 确保字体路径为 `C:\Windows\Fonts\arial.ttf`
- Linux: 确保字体文件存在且可读

**问题 2: 图片生成失败**

- 检查 OpenCV 是否正确安装
- 确认输入 XML 文件格式正确

---

## 📚 技术细节

### 坐标系统

- **BOARD_SIZE**: 149 像素/单位 (正面轨道宽度基准)
- **NOTE_SIZE**: 129 像素/单位 (侧边轨道宽度基准)
- **FRONT_BOARD_RATE**: 正面轨道缩放系数
- **TIME_SIZE**: 2880 像素 (单页时间宽度)

### 颜色定义

```cpp
// 音符颜色
COLOR_NORMAL  = (100, 200, 255)  // 蓝色
COLOR_CHAIN   = (100, 255, 100)  // 绿色
COLOR_HOLD    = (255, 200, 100)  // 橙色
```

### 进度回调支持

```cpp
DynachartRenderer::Options options;
options.progressCallback = [](int current, int total) {
    double percent = (double)current / total * 100;
    printf("Progress: %d/%d (%.1f%%)\n", current, total, percent);
};

renderer.render(chart, "output.png", options);
```

---

## 🤝 贡献指南

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

---

## 📄 许可证

本项目由 [AXIS5](https://github.com/AXIS5hacker)开发，保留所有权利。

---

## 📞 联系方式

- **开发者**: [AXIS5](https://github.com/AXIS5hacker)
- **项目仓库**: DynaChartView
- **问题反馈**: 通过 GitHub Issues

---

## 🙏 鸣谢

本项目参考了以下开源项目：

- **[Dynachart](https://github.com/Errno2048/Dynachart)** by [Errno2048](https://github.com/Errno2048)
  - Python 原版实现，提供了核心算法和渲染逻辑参考
  - 感谢原作者的开源贡献！

---

## 📝 更新日志

### v1.1.0 (2026-04-14)

* 🎨 **新增左右侧背景色** - 左侧暗绿色、右侧暗紫色，增强视觉区分度
* 📏 **小节线跨页问题修复** - 确保小节线当前页内绘制
* ⏱️ **时间戳渲染优化** - 修复时间戳跨页渲染问题，在正确位置显示
* 🔄 **半小节线逻辑修复** - 半小节线于drawPage函数绘制，通过drawBoard函数复制到整个图片

### v1.0.1 (2026-04-13)

- 🐛 修复无法正确显示汉字的bug。现在对于任何系统环境都能正确显示文件名了。已在中文和日语系统下测试。

### v1.0.0 (2026-04-12)

* ✨ 初始版本发布
* 🎨 支持三侧谱面渲染
* 📐 音符右端点对齐修复
* ⏱️ 时间戳渲染功能
* 🔄 进度条回调支持
* 🐛 修复多个已知问题 

---

**Happy Charting! 🎵**
