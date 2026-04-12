# 构建指南

## 前置要求

### Windows (MSVC)

1. **Visual Studio 2019/2022**
   - 安装 C++ 桌面开发工作负载
   - 确保包含 CMake 工具

2. **CMake 3.15+**
   - 下载地址：https://cmake.org/download/
   - 添加到系统 PATH

3. **OpenCV 4.x**
   - 下载地址：https://opencv.org/releases/
   - 或使用 vcpkg: `vcpkg install opencv`

### Linux (Ubuntu/Debian)

```bash
# 安装依赖
sudo apt update
sudo apt install build-essential cmake libopencv-dev
```

### macOS

```bash
# 使用 Homebrew
brew install cmake opencv
```

## 快速构建

### Windows

```powershell
# 1. 创建构建目录
New-Item -ItemType Directory -Force -Path build
cd build

# 2. 配置项目
cmake .. -G "Visual Studio 17 2022" -A x64

# 3. 编译
cmake --build . --config Release

# 4. 运行示例
cd bin/Release
.\chart_viewer.exe ..\..\test_data\sample.xml output.png
```

### Linux/macOS

```bash
# 1. 创建构建目录
mkdir build && cd build

# 2. 配置项目
cmake ..

# 3. 编译
make -j$(nproc)

# 4. 运行示例
./bin/chart_viewer ../test_data/sample.xml output.png
```

## 构建选项

### 仅构建 ChartStore

```bash
cd ChartStore
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 仅构建 ImageGenerator

```bash
# 先确保 ChartStore 已构建
cd ImageGenerator
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Debug 构建

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

## 验证构建

### 检查库文件

**Windows:**
```powershell
# ChartStore
Test-Path "build/lib/ChartStore.lib"

# ImageGenerator  
Test-Path "build/lib/ImageGenerator.lib"

# 可执行文件
Test-Path "build/bin/Release/chart_viewer.exe"
```

**Linux/macOS:**
```bash
# ChartStore
ls -l build/lib/libChartStore.a

# ImageGenerator
ls -l build/lib/libImageGenerator.a

# 可执行文件
ls -l build/bin/chart_viewer
```

### 运行测试

```bash
# 使用测试数据（需要准备好测试 XML 文件）
./bin/chart_viewer test_data/sample.xml output.png

# 查看输出
# Windows
explorer output.png

# Linux
xdg-open output.png

# macOS
open output.png
```

## 常见问题

### Q: CMake 配置失败 - 找不到 OpenCV

**Windows:**
```powershell
# 指定 OpenCV 路径
cmake .. -DOpenCV_DIR="C:/opencv/build/x64/vc15/lib"
```

**Linux:**
```bash
# 确保 opencv-dev 已安装
sudo apt install libopencv-dev
```

### Q: 链接错误 - 找不到 ChartStore

```bash
# 确保先构建 ChartStore
cd ChartStore/build
cmake --build .

# 然后构建 ImageGenerator
cd ../../ImageGenerator/build
cmake ..
```

### Q: MSVC 运行时库冲突

确保所有库使用相同的运行时设置：

```cmake
# 在 CMakeLists.txt 中添加
foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
    if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
endforeach()
```

### Q: OpenCV 版本不兼容

- 最低要求：OpenCV 4.0
- 推荐版本：OpenCV 4.5+
- 检查版本：`pkg-config --modversion opencv4` (Linux)

## 性能优化

### 启用优化标志

```cmake
# 在 CMakeLists.txt 中添加
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(ChartStore PRIVATE /O2)
    else()
        target_compile_options(ChartStore PRIVATE -O3 -march=native)
    endif()
endif()
```

### 静态链接

默认已配置为静态链接运行时（/MT），减小部署体积。

## 部署

### Windows

```powershell
# 复制必要文件
Copy-Item "build/bin/Release/chart_viewer.exe" -Destination "deploy/"
Copy-Item "deploy/test_data/" -Destination "deploy/" -Recurse

# 创建压缩包
Compress-Archive -Path "deploy/*" -DestinationPath "chart_viewer.zip"
```

### Linux

```bash
# 创建部署包
mkdir -p deploy/bin
cp build/bin/chart_viewer deploy/bin/
cp -r test_data deploy/

# 创建 tarball
tar -czf chart_viewer.tar.gz deploy/
```

## CI/CD 集成

### GitHub Actions

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Configure CMake
      run: cmake -B build
    
    - name: Build
      run: cmake --build build --config Release
    
    - name: Upload Artifact
      uses: actions/upload-artifact@v2
      with:
        name: chart_viewer
        path: build/bin/Release/
```

## 下一步

- [ ] 添加单元测试
- [ ] 添加性能基准测试
- [ ] 配置持续集成
- [ ] 创建安装包

---

**最后更新：** 2026-04-10
