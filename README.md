# CppUtils & Graphics 库使用文档

## 项目概述

CppUtils 和 Graphics 是两个功能强大的 Windows C++ 工具库，提供了丰富的系统级功能和图形绘制能力。项目采用 Visual Studio 2022 开发，支持 x86/x64 架构和 MT/MD,Debug/Release 配置。

## 项目结构

```
CppUtils/
├── Graphics/           # 图形库
│   ├── Graphics.h/.cpp    # 核心图形类
│   ├── Factory.h/.cpp     # Direct2D 工厂类
│   ├── Colors.h           # 预定义颜色常量
│   ├── Font.h/.cpp        # 字体处理
│   └── nanosvg.h/.cpp     # SVG 支持
├── Utils/              # 工具库
│   ├── Utils.h            # 主头文件
│   ├── defines.h          # 宏定义和工具
│   ├── StringHelper.h/.cpp    # 字符串处理
│   ├── File.h/.cpp            # 文件操作
│   ├── Process.h/.cpp         # 进程管理
│   ├── Registry.h/.cpp        # 注册表操作
│   ├── Socket.h/.cpp          # 网络编程
│   ├── HttpHelper.h/.cpp      # HTTP 客户端
│   ├── SqliteHelper.h/.cpp    # SQLite 数据库
│   ├── json.h                 # JSON 处理
│   ├── Thread.h/.cpp          # 多线程
│   ├── Clipboard.h/.cpp       # 剪贴板操作
│   ├── MD5.h/.cpp             # MD5 加密
│   ├── SHA256.h/.cpp          # SHA256 加密
│   ├── zlib/                  # 压缩库
│   └── sqlite/                # SQLite 库
├── build/              # 编译输出
│   ├── CppUtils_x64_MT.lib     # x64 Release 静态库
│   ├── CppUtils_x64_MTd.lib    # x64 Debug 静态库
│   ├── Graphics_x64_MT.lib     # x64 Release 图形库
│   └── Graphics_x64_MTd.lib    # x64 Debug 图形库
└── xbyak/              # JIT 汇编库
    └── xbyak.h
```

## 编译配置

### 支持的平台和配置
- **架构**: x86, x64
- **配置**: Debug, Release
- **运行时库**: MT (静态), MD (动态)
- **工具集**: Visual Studio 2022 (v143)
- **C++标准**: C++17

### 自动链接机制
两个库都实现了智能自动链接，根据编译配置自动选择正确的库文件：

#### Graphics 库自动链接
```cpp
// 静态运行时库 (MT)
#ifdef _MT && !defined(_DLL)
    #ifdef _M_X64 && _DEBUG
        #pragma comment(lib, "Graphics_x64_MTd.lib")
    #elif defined(_M_X64) && !defined(_DEBUG)
        #pragma comment(lib, "Graphics_x64_MT.lib")
    // ... 其他配置
#endif
```

#### CppUtils 库自动链接
```cpp
// 类似机制，自动链接 CppUtils_*.lib
```

## Graphics 库详解

### 核心功能
Graphics 库基于 Direct2D 和 Direct3D 11，提供高性能 2D 图形绘制能力。

#### 主要类

##### D2DGraphics - 核心图形类
```cpp
class D2DGraphics {
public:
    // 构造函数
    D2DGraphics();                                    // 空构造函数
    D2DGraphics(UINT width, UINT height);              // 指定位图尺寸
    D2DGraphics(ID2D1Bitmap1* bmp);                    // 基于现有位图
    
    // 基本绘制操作
    void Clear(D2D1_COLOR_F color);                   // 清空画布
    void DrawLine(float x1, float y1, float x2, float y2, D2D1_COLOR_F color, float width = 1.0f);
    void DrawRect(float left, float top, float width, float height, D2D1_COLOR_F color, float linewidth = 1.0f);
    void FillRect(float left, float top, float width, float height, D2D1_COLOR_F color);
    void DrawEllipse(float x, float y, float xr, float yr, D2D1_COLOR_F color, float linewidth = 1.0f);
    void FillEllipse(float cx, float cy, float xr, float yr, D2D1_COLOR_F color);
    
    // 高级绘制
    void FillPolygon(std::vector<D2D1_POINT_2F> points, D2D1_COLOR_F color);
    void DrawArc(D2D1_POINT_2F center, float size, float startAngle, float endAngle, D2D1_COLOR_F color, float width = 1.0f);
    void FillPie(D2D1_POINT_2F center, float width, float height, float startAngle, float sweepAngle, D2D1_COLOR_F color);
    
    // 文本绘制
    void DrawString(std::wstring str, float x, float y, D2D1_COLOR_F color, Font* font = nullptr);
    void DrawString(std::wstring str, float x, float y, float w, float h, D2D1_COLOR_F color, Font* font = nullptr);
    
    // 位图操作
    void DrawBitmap(ID2D1Bitmap* bmp, float x = 0.f, float y = 0.f, float opacity = 1.0f);
    ID2D1Bitmap* CreateBitmap(std::wstring path);     // 从文件创建位图
    ID2D1Bitmap* CreateBitmap(int width, int height); // 创建空位图
    
    // 渐变和效果
    ID2D1LinearGradientBrush* CreateLinearGradientBrush(D2D1_GRADIENT_STOP* stops, unsigned int stopcount);
    ID2D1RadialGradientBrush* CreateRadialGradientBrush(D2D1_GRADIENT_STOP* stops, unsigned int stopcount, D2D1_POINT_2F center);
    
    // 渲染控制
    virtual void BeginRender();
    virtual void EndRender();
    virtual void ReSize(UINT width, UINT height);
};
```

##### HwndGraphics - 窗口图形类
```cpp
class HwndGraphics : public D2DGraphics {
public:
    HwndGraphics(HWND hWnd);  // 绑定到窗口句柄
    // 自动处理窗口尺寸变化和渲染
};
```

##### Factory - Direct2D 工厂类
```cpp
class Factory {
public:
    static ID2D1Factory1* D2DFactory();           // 获取 D2D 工厂
    static IDWriteFactory* DWriteFactory();     // 获取 DirectWrite 工厂
    static IWICImagingFactory* ImageFactory();  // 获取 WIC 图像工厂
    static ID3D11Device* D3DDevice();           // 获取 D3D 设备
    
    // 工具方法
    static IWICBitmap* CreateWICBitmap(std::wstring path);
    static ID2D1PathGeometry* CreateGeomtry();
    static void SaveBitmap(IWICBitmap* bmp, const wchar_t* path);
};
```

#### 颜色系统
提供 900+ 预定义颜色常量：
```cpp
// 基本颜色
Colors::Red, Colors::Green, Colors::Blue, Colors::White, Colors::Black

// Web 颜色
Colors::SkyBlue, Colors::LightGray, Colors::DarkOrange

// 专业颜色
Colors::RoyalBlue, Colors::MediumSeaGreen, Colors::PaleGoldenrod
```

#### 使用示例

##### 基本绘制
```cpp
#include "Graphics/Graphics.h"

// 创建图形对象
D2DGraphics gfx(800, 600);

gfx.BeginRender();
gfx.Clear(Colors::White);

// 绘制基本图形
gfx.FillRect(10, 10, 200, 100, Colors::SkyBlue);
gfx.DrawRect(10, 10, 200, 100, Colors::DarkBlue, 2.0f);

gfx.FillEllipse(400, 300, 100, 50, Colors::LightGreen);
gfx.DrawEllipse(400, 300, 100, 50, Colors::DarkGreen, 3.0f);

// 绘制文本
gfx.DrawString(L"Hello Graphics!", 50, 50, Colors::Black);

gfx.EndRender();
```

##### 窗口绘制
```cpp
HwndGraphics gfx(hWnd);

gfx.BeginRender();
gfx.Clear(Colors::White);
// 绘制内容
gfx.EndRender();
```

##### 渐变效果
```cpp
D2D1_GRADIENT_STOP stops[] = {
    {0.0f, Colors::Red},
    {0.5f, Colors::Yellow},
    {1.0f, Colors::Green}
};
auto brush = gfx.CreateLinearGradientBrush(stops, 3);
gfx.FillRect(0, 0, 200, 200, brush);
brush->Release();
```

## CppUtils 库详解

### 核心功能模块

#### 1. 字符串处理 (StringHelper)
```cpp
class StringHelper {
public:
    // 分割和连接
    static std::vector<std::string> Split(std::string str, std::string separator);
    static std::string Join(std::vector<std::string> strs, std::string separator);
    
    // 文本处理
    static std::string Replace(std::string str, std::string oldstr, std::string newstr);
    static std::string ToUpper(std::string str);
    static std::string ToLower(std::string str);
    static std::string Trim(std::string str);
    
    // 查询操作
    static int IndexOf(std::string str, std::string substr);
    static bool Contains(std::string str, std::string substr);
    static int GetHashCode(std::string str);
    
    // 格式化
    static std::string Format(const char* fmt, ...);
    static std::wstring Format(const wchar_t* fmt, ...);
};
```

#### 2. 文件操作 (File)
```cpp
class File {
public:
    // 基本操作
    static bool Exists(const std::string path);
    static void Delete(const std::string path);
    static void Copy(const std::string src, const std::string dest);
    static void Move(const std::string src, const std::string dest);
    static void Create(const std::string path);
    
    // 读写操作
    static std::string ReadAllText(const std::string path);
    static std::vector<uint8_t> ReadAllBytes(const std::string path);
    static std::vector<std::string> ReadAllLines(const std::string path);
    
    static void WriteAllText(const std::string path, const std::string content);
    static void WriteAllBytes(const std::string path, const std::vector<uint8_t> content);
    static void WriteAllLines(const std::string path, const std::vector<std::string> content);
    
    // 追加操作
    static void AppendAllText(const std::string path, const std::string content);
    static void AppendAllBytes(const std::string path, const std::vector<uint8_t> content);
    static void AppendAllLines(const std::string path, const std::vector<std::string> content);
};
```

#### 3. 进程管理 (Process)
```cpp
class Process {
public:
    Process(DWORD pid);
    
    // 进程信息
    std::string MainWindowTitle();
    HWND MainWindowHandle();
    int ExitCode();
    bool HasExited();
    
    // 进程控制
    void Kill();
    static Process Start(std::string fileName, std::string arguments = "", std::string workingDirectory = "");
    
    // 内存信息
    long long VirtualMemorySize();
    long long WorkingSet64();
    long long NonpagedSystemMemorySize();
    
    // 模块信息
    std::vector<MODULEENTRY32> Modules();
    MODULEINFO MainModule();
};
```

#### 4. 注册表操作 (Registry)
```cpp
class Registry {
public:
    // 预定义根键
    static RegistryKey ClassesRoot;
    static RegistryKey CurrentConfig;
    static RegistryKey CurrentUser;
    static RegistryKey LocalMachine;
    static RegistryKey Users;
    
    // 打开注册表键
    static RegistryKey OpenBaseKey(HKEY hKey, const std::string& subKey);
    static RegistryKey OpenBaseKey(HKEY hKey, const std::string& subKey, bool writable);
};
```

#### 5. 网络编程 (Socket)
```cpp
class Socket {
public:
    Socket();
    ~Socket();
    
    // 基本操作
    bool Connect(const std::string& host, int port);
    void Close();
    bool IsConnected();
    
    // 数据收发
    int Send(const std::vector<uint8_t>& data);
    int Send(const std::string& data);
    std::vector<uint8_t> Receive(int size);
    std::string ReceiveString(int size);
    
    // 设置选项
    void SetReceiveTimeout(int milliseconds);
    void SetSendTimeout(int milliseconds);
};
```

#### 6. HTTP 客户端 (HttpHelper)
```cpp
class HttpHelper {
public:
    // GET 请求
    static std::string Get(const std::string& url);
    static std::vector<uint8_t> GetBytes(const std::string& url);
    
    // POST 请求
    static std::string Post(const std::string& url, const std::string& data);
    static std::string Post(const std::string& url, const std::vector<uint8_t>& data);
    
    // 异步请求
    static void GetAsync(const std::string& url, std::function<void(std::string)> callback);
    static void PostAsync(const std::string& url, const std::string& data, std::function<void(std::string)> callback);
};
```

#### 7. 数据库操作 (SqliteHelper)
```cpp
class SqliteHelper {
public:
    SqliteHelper(const std::string& dbPath);
    ~SqliteHelper();
    
    // 基本操作
    bool Execute(const std::string& sql);
    bool Open();
    void Close();
    bool IsOpen();
    
    // 查询操作
    std::vector<std::vector<std::string>> Query(const std::string& sql);
    std::vector<std::map<std::string, std::string>> QueryMap(const std::string& sql);
    
    // 参数化查询
    bool Execute(const std::string& sql, const std::vector<std::string>& params);
};
```

#### 8. 加密功能 (MD5, SHA256)
```cpp
class MD5 {
public:
    static std::string Hash(const std::string& input);
    static std::string Hash(const std::vector<uint8_t>& input);
    static std::string HashFile(const std::string& filePath);
};

class SHA256 {
public:
    static std::string Hash(const std::string& input);
    static std::string Hash(const std::vector<uint8_t>& input);
    static std::string HashFile(const std::string& filePath);
};
```

#### 9. 数据压缩 (zlib)
```cpp
// 压缩函数
std::vector<uint8_t> Compress(uint8_t* buffer, int len);
std::vector<uint8_t> Decompress(uint8_t* buffer, int len, int maxlen);

// GZip 格式
std::vector<uint8_t> GCompress(std::vector<uint8_t> input);
std::vector<uint8_t> GDecompress(std::vector<uint8_t> compressedBytes);
```

#### 10. JSON 处理
```cpp
// 使用 json 对象
json j;
j["name"] = "test";
j["value"] = 123;
j["array"] = {1, 2, 3};

std::string jsonStr = j.dump();          // 序列化
json parsed = json::parse(jsonStr);     // 反序列化
```

#### 11. 实用工具

##### 时间和日期 (DateTime, TimeSpan)
```cpp
DateTime now = DateTime::Now();
std::string formatted = now.ToString();
TimeSpan span(1, 2, 30, 0);  // 1天2小时30分钟
```

##### 随机数 (CRandom)
```cpp
CRandom rand;
int value = rand.Next(100);           // 0-99
double dvalue = rand.NextDouble();    // 0.0-1.0
```

##### GUID 生成 (Guid)
```cpp
std::string guid = Guid::NewGuid().ToString();
```

##### 剪贴板操作 (Clipboard)
```cpp
Clipboard::SetText("Hello World");
std::string text = Clipboard::GetText();
```

### 使用示例

#### 文件和字符串操作
```cpp
#include "Utils/Utils.h"

// 文件操作
std::string content = File::ReadAllText("config.txt");
File::WriteAllText("output.txt", "Hello World");

// 字符串处理
std::vector<std::string> parts = StringHelper::Split("a,b,c", ",");
std::string joined = StringHelper::Join(parts, ";");
std::string upper = StringHelper::ToUpper("hello");
```

#### 网络请求
```cpp
// HTTP 请求
std::string html = HttpHelper::Get("http://example.com");
std::string response = HttpHelper::Post("http://api.example.com", "{\"data\":123}");

// Socket 通信
Socket socket;
if (socket.Connect("127.0.0.1", 8080)) {
    socket.Send("Hello Server");
    std::string response = socket.ReceiveString(1024);
    socket.Close();
}
```

#### 数据库操作
```cpp
SqliteHelper db("data.db");
if (db.Open()) {
    db.Execute("CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT)");
    db.Execute("INSERT INTO users (name) VALUES ('Alice')");
    
    auto results = db.Query("SELECT * FROM users");
    for (auto& row : results) {
        std::cout << row[1] << std::endl;  // 输出用户名
    }
    
    db.Close();
}
```

#### 进程管理
```cpp
// 启动进程
Process proc = Process::Start("notepad.exe", "document.txt");

// 获取进程信息
std::string title = proc.MainWindowTitle();
if (!proc.HasExited()) {
    proc.Kill();
}

// 枚举进程
std::vector<Process> processes = Process::GetProcesses();
```

#### 加密和哈希
```cpp
// MD5 哈希
std::string hash1 = MD5::Hash("password");
std::string fileHash = MD5::HashFile("file.txt");

// SHA256 哈希
std::string hash2 = SHA256::Hash("sensitive data");
```

## 集成指南

### 1. 添加到头文件引用
- 添加下面两个头文件引用即可,用户已经将所需头文件和库放入全局系统SDK对应目录:
#include <CppUtils/Utils/Utils.h>
#include <CppUtils/Graphics/Graphics.h>

### 2. 添加依赖项
确保项目依赖正确的系统库：
- Graphics 库需要: `d2d1.lib`, `dxgi.lib`, `d3d11.lib`
- CppUtils 库需要: `ws2_32.lib`, `wininet.lib`, `wtsapi32.lib`, `dbghelp.lib`

### 4. 编译顺序
确保先编译 CppUtils，再编译 Graphics（如果 Graphics 依赖 CppUtils）。

## 注意事项

### 1. 平台限制
- 仅支持 Windows 平台
- 需要 Windows 7 或更高版本
- 需要 DirectX 11 支持（Graphics 库）

### 2. 运行时要求
- Visual C++ Redistributable 2015-2022
- DirectX 运行时

### 3. 线程安全
- Graphics 对象不是线程安全的，每个线程应该使用独立的实例
- CppUtils 中的大部分工具类是线程安全的

### 4. 性能建议
- 重用 Graphics 对象而不是频繁创建销毁
- 批量绘制操作比单独绘制更高效
- 使用合适的位图缓存策略

### 5. 错误处理
- 大部分函数在失败时返回空值或默认值
- 关键操作应该检查返回值
- 使用 `GetLastError()` 获取详细的错误信息

## 扩展功能

### 自定义图形效果
可以通过 Direct2D 效果系统添加自定义滤镜和特效。

### 插件架构
CppUtils 支持通过动态加载扩展功能。

### 性能监控
内置性能计数器和内存使用监控功能。

## 技术支持

如遇到问题，请检查：
1. 编译配置是否匹配（x86/x64, Debug/Release, MT/MD）
2. 所有依赖项是否正确链接
3. DirectX 运行时是否安装
4. Visual C++ Redistributable 是否安装

## 更新日志

- v1.0: 初始版本，包含基础图形和工具功能
- 支持 Visual Studio 2022 和 C++17
- 完整的 x86/x64 架构支持
- 自动链接机制简化使用