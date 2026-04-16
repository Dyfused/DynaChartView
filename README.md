<h1 style="margin-top: 0" align="center">DynaChartView</h1>

<h2 style="margin-top: 0" align="center"><a href="./README-zh-cn.md">中文</a> | <strong>English</strong></h2>

<p align="center">A Dynamite music game chart image generation tool (C++ implementation)</p>

---

## Project Structure

The main directories and files of the repository are listed below (top to bottom):

```
DynaChartView/
├─ CMakeLists.txt                # Top-level CMake build configuration
├─ README.md                     # Project documentation (including this section)
├─ README-zh-cn.md               # Project documentation in Chinese
├─ ImageGenerator/               # Image generation subproject (renderer and resources)
│  ├─ CMakeLists.txt
│  ├─ include/
│  │  └─ dynachart_renderer.h    # Renderer header file
│  └─ src/
│     ├─ dynachart_renderer.cpp  # Renderer implementation
│     └─ dynachart_renderer_legacy.cpp  # Legacy renderer implementation (v1.1.0 and earlier)
├─ src/
│  └─ main.cpp                    # Program entry point, command-line parsing and coordination logic
├─ include/
│  ├─ version.h.in                  # Version information template, version.h is generated during CMake configuration
│  └─ version.h                   # Project version constants
├─ ChartStore/                    # Chart data structure and parsing related code
│  └─ include/
│     └─ chart_store.h            # ChartStore class definition
├─ .gitignore
├─ .gitattributes
└─ LICENSE.txt
```

Notes:

- `ImageGenerator` contains the actual Dynachart-style rendering implementation (based on OpenCV and FreeType).
- `src/main.cpp` is the entry point of the command-line tool, responsible for reading charts (XML), initializing the renderer, and saving output images.
- `ChartStore/include/chart_store.h` provides in-memory structures and parsing functionality for chart data.
- The top-level `CMakeLists.txt` and subdirectory `ImageGenerator/CMakeLists.txt` are used for configuration and building (using Ninja generator, C++14, MSVC /utf-8 encoding).

---

## 🔭 Result Examples

<img width="8344" height="23144" alt="Estahv H11" src="https://github.com/user-attachments/assets/4ce0dfd5-a5b2-4d72-841e-200727a07530" />
Estahv Hard 11 (default parameters)

<img width="16688" height="15224" alt="Finixtahv G15" src="https://github.com/user-attachments/assets/0467d717-ed8f-46d5-889d-72df26b80879" />
Finixtahv Giga 15 (-b 2 -l 15 -S 0.7 —— Highlight bar lines every 2 bars, show timestamps, 15 bars per page, speed 0.7)

## 🚀 Usage

### Command-Line Tool

#### Basic Usage

```bash
# Windows
.\build\Release\DynaChartView.exe input.xml output.png

# Linux/macOS
./build/DynaChartView input.xml output.png
```

#### Command-Line Parameters

```
Usage: DynaChartView <input file> <output file> [options]

Options:
  -s <float>, --scale <float>        Scaling factor (default: 0.25)
  -S, --speed <float>       Display speed (determines bar height, default: 0.5)
  -b, --bar-span <int>      Highlight bar line interval (default: 2)
  -l, --page-limit <int>    Number of bars per page (default: 32)
  -f, --font <path>         Font file path (default: arial.ttf from system folder)
  --help                    Show help information
  --legacy-render           Use legacy rendering (OVERLAPPING parts of HOLD notes are obscured instead of blended)
  --use-system-font         Use system font (default behavior)

Examples:
  DynaChartView chart.xml
  DynaChartView chart.xml chart.png
  DynaChartView chart.xml chart.png -s 1.2
  DynaChartView chart.xml chart.png -S 0.7 -l 24
  DynaChartView chart.xml chart.png -f /usr/share/fonts/arial.ttf
```

#### Missing MSVC Runtime?

For the .exe installer package provided in the release, check the installation folder after installation - it contains vc_redist.x64.exe. Run this file to install the MSVC runtime.

### C++ API Integration

```cpp
#include "chart_store.h"
#include "dynachart_renderer.h"

int main() {
    // 1. Read chart
    chart_store chart;
    int result = chart.readfile("chart.xml");

    if (result != 0) {
        std::cerr << "Failed to load chart: " << result << std::endl;
        return 1;
    }

    // 2. Create renderer
    DynachartRenderer renderer;

    // 3. Set options
    DynachartRenderer::Options options;
    options.scale = 1.5;
    options.barHeight = 60.0;
    options.gridVisible = true;

    // 4. Generate image
    renderer.render(chart, "output.png", options);

    return 0;
}
```

---

## 📦 Compilation Methods

### Windows (MSVC)

#### Prerequisites

1. **Visual Studio 2019/2022** (with "Desktop development with C++" workload)
2. **CMake 3.15+**
3. **vcpkg** (recommended, for dependency management)

#### Install Dependencies with vcpkg (Highly Recommended)

**vcpkg is a C/C++ package manager developed by Microsoft that automatically resolves OpenCV and FreeType dependencies.**

```powershell
# 1. Clone vcpkg (outside the project directory or any location)
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# 2. Initialize vcpkg
.\bootstrap-vcpkg.bat

# 3. Install project dependencies
.\vcpkg install opencv4:x64-windows freetype:x64-windows

# 4. Integrate vcpkg with Visual Studio (optional but recommended)
.\vcpkg integrate install

# After completion, CMake will automatically find these libraries
```

**vcpkg Installation Notes:**

- `opencv4:x64-windows` - Image processing library (includes core, imgcodecs, highgui modules, etc.)
- `freetype:x64-windows` - Font rendering library (used for timestamp and text display)

**Specify vcpkg toolchain during CMake configuration:**

```powershell
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="<Your vcpkg install path>\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>"
```

**Static Linking of MSVC Runtime**

If using static versions of vcpkg libraries:

Install static versions of the libraries (opencv4:x64-windows-static freetype:x64-windows-static)
You may need to define the OPENCV_STATIC macro

```powershell
# 1. Install static dependencies (x64/arm64)
# x64
.\vcpkg install opencv4:x64-windows-static freetype:x64-windows-static
# arm64 (optional)
.\vcpkg install opencv4:arm64-windows-static freetype:arm64-windows-static

# 2. Configure CMake (x64 example)
cmake .. -G "Visual Studio 17 2022" -A x64 `   # -A arm64 for arm
  -DCMAKE_TOOLCHAIN_FILE="<Your vcpkg install path>\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows-static `
  -DOpenCV_STATIC=ON `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>"

# 3. Compile
cmake --build . --config Release
```

> Note:
> 
> To statically link the MSVC runtime, use `-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>"` (automatically uses `/MT` for Release, `/MTd` for Debug) instead of directly setting `-DCMAKE_EXE_LINKER_FLAGS="/MT"`.

#### Manual Dependency Installation (Not Recommended)

**Method 1: Official OpenCV Installer + Manual FreeType Compilation**

```powershell
# Download OpenCV 4.x Windows installer
# https://github.com/opencv/opencv/releases

# Extract and set environment variables
$env:OPENCV_DIR = "C:\opencv\build\x64\vc15\lib"
$env:Path += ";C:\opencv\build\x64\vc15\bin"

# Note: FreeType needs to be downloaded and compiled separately, which is cumbersome
```

#### Compilation Steps

```powershell
# 1. Navigate to project directory
cd path/to/DynaChartView

# 2. Create build directory
if (Test-Path build) { Remove-Item -Recurse -Force build }
mkdir build
cd build

# 3. Configure project (auto-detects MSVC)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Or specify generator
# cmake .. -G "Visual Studio 17 2022" -A x64

# 4. Compile
cmake --build . --config Release

# 5. Run the program
.\Release\DynaChartView.exe input.xml output.png
```

#### Static Linking of MSVC Runtime

To generate a standalone executable that does not depend on VCRedist:

```powershell
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>"
```

---

### Linux

#### Prerequisites

```bash
# Ubuntu / Debian / Linux Mint
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libopencv-dev \
    libfreetype6-dev

# Fedora / Rocky Linux / RHEL
sudo dnf groupinstall "Development Tools"
sudo dnf install -y cmake opencv-devel freetype-devel
```

#### Compilation Steps

```bash
# 1. Go to project directory
cd DynaChartView

# 2. Create build directory
mkdir -p build && cd build

# 3. Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. Build (use all CPU cores)
make -j$(nproc)

# 5. Run program
./bin/DynaChartView input.xml output.png
```

#### Static Linking (Optional)

To build a fully static executable that runs on all Linux distributions without dependencies:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS="-static -static-libgcc -static-libstdc++"
make -j$(nproc)
```

> Note: Ensure `opencv-devel` / `libopencv-dev` (static library version) is installed.

---

### macOS

#### Prerequisites

```bash
# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake opencv freetype
```

#### Compilation Steps

```bash
# 1. Navigate to project directory
cd DynaChartView

# 2. Create build directory
mkdir -p build && cd build

# 3. Configure project
cmake .. -DCMAKE_BUILD_TYPE=Release

# 4. Compile
make -j$(sysctl -n hw.ncpu)

# 5. Run the program
./DynaChartView input.xml output.png
```

---

# File Encoding

- **XML Encoding**: UTF-8
- **Output Image**: PNG (RGBA)

---

## 🔧 Common Issues

### Windows Compilation Issues

**Issue 1: 'cmake' is not recognized as a command**

```powershell
# Solution: Add CMake to PATH
$env:Path += ";C:\Program Files\CMake\bin"
```

**Issue 2: Cannot find OpenCV**

```powershell
# Set OpenCV path
$env:OpenCV_DIR = "C:\opencv\build"
cmake .. -DCMAKE_PREFIX_PATH="C:\opencv\build"
```

**Issue 3: Missing MSVC Runtime**

```powershell
# Use static linking
cmake .. -DCMAKE_EXE_LINKER_FLAGS="/MT"
```

### Linux Compilation Issues

**Issue 1: CMake version is too low**

```bash
# Install new version
wget https://github.com/Kitware/CMake/releases/download/v3.24.0/cmake-3.24.0-linux-x86_64.tar.gz
tar -xzf cmake-3.24.0-linux-x86_64.tar.gz
sudo cp -r cmake-3.24.0-linux-x86_64/* /usr/local/
```

**Issue 2: OpenCV library not found**

```bash
# Set OpenCV path
export OpenCV_DIR=/usr/lib/x86_64-linux-gnu/cmake/opencv4
```

### Runtime Issues

**Issue 1: Font rendering not displayed**

- Windows: Ensure the font path is `C:\Windows\Fonts\arial.ttf`
- Linux: Ensure the font file exists and is readable

**Issue 2: Image generation failed**

- Check if OpenCV is installed correctly
- Verify the input XML file format is valid

---

## 📚 Technical Details

### Coordinate System

- **BOARD_SIZE**: 149 pixels/unit (front track width baseline)
- **NOTE_SIZE**: 129 pixels/unit (side track width baseline)
- **FRONT_BOARD_RATE**: Front track scaling factor
- **TIME_SIZE**: 2880 pixels (single page time width)

### Color Definitions

```cpp
// Note colors (BGRA)
NOTE_COLOR_NORMAL(255, 255, 0, 255)  // cyan
NOTE_COLOR_CHAIN(51, 51, 255, 255)  // red
NOTE_COLOR_HOLD_FILL(0, 100, 50, 255)  // Green
```

### Progress Callback Support

```cpp
DynachartRenderer::Options options;
options.progressCallback = [](int current, int total) {
    double percent = (double)current / total * 100;
    printf("Progress: %d/%d (%.1f%%)\n", current, total, percent);
};

renderer.render(chart, "output.png", options);
```

---

## 🤝 Contribution Guidelines

1. Fork the project
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## 📄 License

This project is developed by [AXIS5](https://github.com/AXIS5hacker), all rights reserved.

---

## 📞 Contact

- **Developer**: [AXIS5](https://github.com/AXIS5hacker)
- **Project Repository**: DynaChartView
- **Issue Reporting**: Via GitHub Issues

---

## 🙏 Acknowledgments

This project references the following open-source projects:

- **[Dynachart](https://github.com/Errno2048/Dynachart)** by [Errno2048](https://github.com/Errno2048)
  - Original Python implementation that provided reference for core algorithms and rendering logic
  - Thanks to the original author for open-source contributions!

---

## 📝 Changelog

### v1.2.0 (2026-04-15)

* 🐛 **Reject invalid input**. Parameters -s, -S, -l, -b now validate input values (e.g., scaling factor must be greater than 0, bars per page must be positive integer, etc.). If input is invalid, the program outputs an error message and exits instead of continuing execution with unexpected behavior.
* 🎨 **Fix layer order** - The current Note layer order is: HOLD bottom layer, NOTE middle layer, CHAIN top layer. This prevents CHAIN notes from being covered by other notes.
* 🔄 **Optional v1.1.0 rendering** - With the new --legacy-render option, users can choose to use the legacy rendering method (rendering logic from v1.1.0) to accommodate certain special requirements or users who prefer the legacy visual effect.

### v1.1.4 (2026-04-15)

* 🎨 Reduce HOLD color brightness - changed from (0, 134, 70) to (0, 100, 50), making overlapping HOLDs less glaring.

### v1.1.3 (2026-04-14)

* 🐛 **Fix Linux compilation**. Fixed compilation failure on Linux due to compiler support issues. Now compiles correctly on Fedora 36.
* 🔄 **Change default font path on Linux** - fontPath = "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf";

### v1.1.2 (2026-04-14)

* 🐛 **Fix HOLD stacking logic**. Now for overlapping HOLD notes, subsequent HOLDs are correctly stacked on top of previous ones instead of overlapping.
* 🎨 **HOLD display changed to transparency** - By adding a new intermediate HoldBoard layer, HOLD notes now show overlapping relationships through adjusted transparency for enhanced visual effect.
* 📏 **Fix over-wide note cross-page display** - Notes extending beyond a single page are now correctly truncated.

### v1.1.0 (2026-04-14)

* 🎨 **Add left/right background colors** - Dark green on left, dark purple on right for better visual distinction
* 📏 **Fix bar line cross-page issue** - Ensure bar lines are drawn within the current page
* ⏱️ **Optimize timestamp rendering** - Fix cross-page timestamp rendering issues to display at correct positions
* 🔄 **Fix half-bar line logic** - Half-bar lines are drawn in the drawPage function and copied to the entire image via the drawBoard function

### v1.0.1 (2026-04-13)

* 🐛 Fix bug that prevented correct display of Chinese characters. Now filenames are displayed correctly in any system environment. Tested on Chinese and Japanese systems.

### v1.0.0 (2026-04-12)

* ✨ Initial release
* 🎨 Support three-side chart rendering
* 📐 Fix note right endpoint alignment
* ⏱️ Timestamp rendering functionality
* 🔄 Progress bar callback support
* 🐛 Fix multiple known issues 

---

**Happy Charting! 🎵**
