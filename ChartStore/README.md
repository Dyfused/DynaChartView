# ChartStore Library

Dynamix 音乐游戏谱面解析库。

## 功能特性

- XML 谱面文件解析
- 支持多种音符类型（NORMAL, CHAIN, HOLD, SUB）
- 支持三侧音符（左、中、右）
- BPM 变化解析
- 谱面数据验证
- Hold-Sub 匹配检查

## 数据结构

### note 结构体

```cpp
struct note {
    int id;           // 音符 ID
    types notetype;   // 音符类型
    double time;      // 开始时间（bar）
    double position;  // 位置（-1~1）
    double width;     // 持续时间（bar）
    int subid;        // Hold 音符的 Sub ID
};
```

### bpmchange 结构体

```cpp
struct bpmchange {
    double time;  // 变化时间（bar）
    double bpm;   // BPM 值
};
```

### chart_store 类

```cpp
class chart_store {
public:
    // 音符存储
    std::map<int, note> m_notes;    // 中间音符
    std::map<int, note> m_left;     // 左侧音符
    std::map<int, note> m_right;    // 右侧音符
    
    // BPM 变化
    std::vector<bpmchange> bpm_list;
    
    // 读取文件
    int readfile(std::string fn);
    
    // 写入文件
    bool to_file(std::string f);
    
    // 统计信息
    int get_mid_count();
    int get_left_count();
    int get_right_count();
    int get_tap_count();
    int get_chain_count();
    int get_hold_count();
};
```

## 枚举类型

### types (音符类型)

- `NORMAL` - 普通音符
- `CHAIN` - 连打音符
- `HOLD` - 长按音符
- `SUB` - Hold 的结尾音符
- `NULLTP` - 未定义类型

### sides (区域类型)

- `PAD` - 按键区域
- `MIXER` - 混音器区域
- `MULTI` - 多键区域
- `UNKNOWN` - 未设置

## 使用示例

### 读取谱面文件

```cpp
#include "chart_store.h"

try {
    chart_store chart;
    int flags = chart.readfile("chart.xml");
    
    if (flags & BARPM_MISSING) {
        std::cerr << "Warning: BarPerMin not set" << std::endl;
    }
    
    std::cout << "Total notes: " 
              << chart.get_mid_count() + chart.get_left_count() 
              + chart.get_right_count() << std::endl;
              
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

### 遍历音符

```cpp
// 遍历中间音符
for (const auto& pair : chart.m_notes) {
    const note& n = pair.second;
    
    std::cout << "Note #" << n.id << ": ";
    std::cout << "time=" << n.time << ", ";
    std::cout << "pos=" << n.position << ", ";
    std::cout << "width=" << n.width << std::endl;
}
```

### 创建谱面

```cpp
chart_store chart;

// 设置基本信息
chart.set_barpm(120.0);
chart.set_lside(PAD);
chart.set_rside(PAD);

// 添加音符（通过直接操作 m_notes 等）
note n;
n.id = 1;
n.notetype = NORMAL;
n.time = 0.0;
n.position = 0.0;
n.width = 1.0;

chart.m_notes.insert({1, n});

// 保存文件
chart.to_file("output.xml");
```

## XML 格式说明

谱面文件采用 XML 格式，结构如下：

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<CMap>
    <m_path>Song Name</m_path>
    <m_barPerMin>120</m_barPerMin>
    <m_timeOffset>0</m_timeOffset>
    <m_leftRegion>PAD</m_leftRegion>
    <m_rightRegion>PAD</m_rightRegion>
    <m_mapID>MAP001</m_mapID>
    
    <m_notes>
        <CMapNoteAsset>
            <m_id>1</m_id>
            <m_type>NORMAL</m_type>
            <m_time>0.0</m_time>
            <m_position>0.0</m_position>
            <m_width>1.0</m_width>
            <m_subId>-1</m_subId>
        </CMapNoteAsset>
    </m_notes>
    
    <m_notesLeft>
        <!-- 左侧音符 -->
    </m_notesLeft>
    
    <m_notesRight>
        <!-- 右侧音符 -->
    </m_notesRight>
    
    <m_argument>
        <m_bpmchange>
            <CBpmchange>
                <m_time>10.0</m_time>
                <m_value>140</m_value>
            </CBpmchange>
        </m_bpmchange>
    </m_argument>
</CMap>
```

## 错误处理

`readfile()` 函数返回以下标志：

- `0` - 成功
- `BARPM_MISSING` (0x1) - BarPerMin 未设置
- `LEFT_SIDE_MISSING` (0x2) - 左侧类型未设置
- `RIGHT_SIDE_MISSING` (0x4) - 右侧类型未设置
- `HOLD_SUB_MISMATCH` (0x8) - Hold-Sub 不匹配

异常类型：
- `std::logic_error` - 解析错误，包含详细的错误信息

## 构建

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

生成的库文件：
- Windows: `ChartStore.lib`
- Linux/macOS: `libChartStore.a`

## 集成到其他项目

### CMake

```cmake
add_subdirectory(ChartStore)
target_link_libraries(your_target PRIVATE ChartStore)
```

### 手动链接

```bash
# Windows
cl /I ChartStore\include your_code.cpp ChartStore\lib\ChartStore.lib

# Linux
g++ -I ChartStore/include your_code.cpp -L ChartStore/lib -lChartStore
```
