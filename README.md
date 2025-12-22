# CppUtils & Graphics 库

## 项目概述

这是一个功能完整的 Windows C++ 工具集，包含两个核心库：**Graphics** (基于 Direct2D/Direct3D 的高性能图形渲染库) 和 **Utils** (系统级工具库)。项目使用 Visual Studio 2022 开发，支持完整的 x86/x64 架构和多种运行时配置。

**主要特点：**
- 🎨 基于 Direct2D 1.1 的现代图形渲染引擎，支持 GPU 加速
- 🛠️ 丰富的系统工具类，涵盖文件、网络、数据库、加密等
- 📦 智能自动链接机制，零配置集成
- 🔧 完整的类型安全封装，符合现代 C++ 标准
- 🚀 高性能、低开销设计

---

## 快速开始

### 包含头文件
```cpp
#include <Graphics/Graphics.h>     // Graphics 库
#include <Utils/Utils.h>           // Utils 库
```

### 自动链接
库文件会根据编译配置自动链接，无需手动配置 `lib` 路径：
- **x86/x64**: 自动检测
- **Debug/Release**: 自动检测
- **MT/MD**: 自动检测运行时库类型

### 示例代码
```cpp
#include <Graphics/Graphics.h>
#include <Utils/Utils.h>

int main() {
    // 图形绘制
    D2DGraphics gfx(800, 600);
    gfx.BeginRender();
    gfx.Clear(Colors::White);
    gfx.FillRect(100, 100, 200, 150, Colors::SkyBlue);
    gfx.DrawString(L"Hello World!", 150, 175, Colors::Black);
    gfx.EndRender();
    
    // 文件操作
    File::WriteAllText("test.txt", "Hello CppUtils!");
    std::string content = File::ReadAllText("test.txt");
    
    return 0;
}
```

---

## 一、Graphics 库详解

### 1.1 核心概念

Graphics 库提供两个版本的图形引擎：
- **D2DGraphics**: 基于 ID2D1RenderTarget，兼容性好
- **D2DGraphics1**: 基于 ID2D1DeviceContext，功能更强大，支持 DXGI 互操作

### 1.2 核心类

#### D2DGraphics - 基础图形类

```cpp
// 构造方式
D2DGraphics();                                    // 空对象
D2DGraphics(UINT width, UINT height);            // 离屏渲染
D2DGraphics(IWICBitmap* bitmap);                 // 基于 WIC 位图
D2DGraphics(const BitmapSource* bitmap);         // 基于 BitmapSource

// 渲染控制
void BeginRender();
void EndRender();
void ReSize(UINT width, UINT height);
void Clear(D2D1_COLOR_F color);

// 基本绘制
void DrawLine(float x1, float y1, float x2, float y2, D2D1_COLOR_F color, float linewidth = 1.0f);
void DrawRect(float left, float top, float width, float height, D2D1_COLOR_F color, float linewidth = 1.0f);
void FillRect(float left, float top, float width, float height, D2D1_COLOR_F color);
void DrawEllipse(float x, float y, float xr, float yr, D2D1_COLOR_F color, float linewidth = 1.0f);
void FillEllipse(float x, float y, float xr, float yr, D2D1_COLOR_F color);

// 文本绘制
void DrawString(const std::wstring& str, float x, float y, D2D1_COLOR_F color, Font* font = nullptr);
void DrawString(const std::wstring& str, float x, float y, float w, float h, D2D1_COLOR_F color, Font* font = nullptr);
void DrawStringCentered(const std::wstring& str, float centerX, float centerY, D2D1_COLOR_F color, Font* font = nullptr);
void DrawStringOutlined(const std::wstring& str, float x, float y, D2D1_COLOR_F textColor, D2D1_COLOR_F outlineColor, Font* font = nullptr);

// 高级绘制
void FillPolygon(std::vector<D2D1_POINT_2F> points, D2D1_COLOR_F color);
void DrawArc(D2D1_POINT_2F center, float size, float startAngle, float endAngle, D2D1_COLOR_F color, float width = 1.0f);
void FillPie(D2D1_POINT_2F center, float width, float height, float startAngle, float sweepAngle, D2D1_COLOR_F color);
void DrawBitmap(ID2D1Bitmap* bmp, float x, float y, float opacity = 1.0f);

// 画刷创建
ID2D1LinearGradientBrush* CreateLinearGradientBrush(D2D1_GRADIENT_STOP* stops, unsigned int stopcount);
ID2D1RadialGradientBrush* CreateRadialGradientBrush(D2D1_GRADIENT_STOP* stops, unsigned int stopcount, D2D1_POINT_2F center);
ID2D1BitmapBrush* CreateBitmapBrush(ID2D1Bitmap* bmp);

// 裁剪与变换
void PushDrawRect(float left, float top, float width, float height);
void PopDrawRect();
void SetTransform(D2D1_MATRIX_3X2_F matrix);
void ClearTransform();
```

#### HwndGraphics - 窗口图形类

```cpp
HwndGraphics gfx(hWnd);  // 绑定到窗口
gfx.BeginRender();
gfx.Clear(Colors::White);
// ... 绘制操作
gfx.EndRender();
```

#### D2DGraphics1 - 增强图形类 (推荐)

```cpp
// 支持 DXGI SwapChain
D2DGraphics1(IDXGISwapChain* swapChain);

// 新增功能
ID2D1Bitmap1* CreateBitmapFromDxgiSurface(IDXGISurface* surface);
void DrawDxgiSurface(IDXGISurface* surface, float x, float y, float width, float height, float opacity = 1.0f);
ID2D1DeviceContext* GetDeviceContext() const;

// 全局设备访问
ID3D11Device* Graphics1_GetSharedD3DDevice();
IDXGIDevice* Graphics1_GetSharedDXGIDevice();
```

#### BitmapSource - 统一位图封装

```cpp
// 创建位图
static std::shared_ptr<BitmapSource> FromFile(const std::wstring& path);
static std::shared_ptr<BitmapSource> FromBuffer(const void* data, size_t size);
static std::shared_ptr<BitmapSource> FromHBitmap(HBITMAP bitmap);
static std::shared_ptr<BitmapSource> FromHIcon(HICON icon);
static std::shared_ptr<BitmapSource> CreateEmpty(int width, int height);

// GIF 支持
static std::vector<std::shared_ptr<BitmapSource>> FromGifFile(const std::wstring& path);
static std::vector<std::shared_ptr<BitmapSource>> FromGifBuffer(const void* data, size_t size);

// 属性访问
IWICBitmap* GetWicBitmap() const;
D2D1_SIZE_U GetPixelSize() const;
GUID GetPixelFormat() const;
std::vector<uint8_t> CopyPixels(UINT* stride = nullptr) const;

// 保存
bool Save(const std::wstring& path, const GUID fileFormat = GUID_ContainerFormatPng) const;
bool Save(std::vector<uint8_t>& buffer, const GUID fileFormat = GUID_ContainerFormatPng) const;
```

#### Font - 字体管理

```cpp
Font(std::wstring fontFamilyName, float fontSize);

// 属性
PROPERTY(float, FontSize);
PROPERTY(std::wstring, FontName);
READONLY_PROPERTY(IDWriteTextFormat*, FontObject);

// 文本测量
D2D1_SIZE_F GetTextSize(std::wstring str, float w = FLT_MAX, float h = FLT_MAX);
D2D1_SIZE_F GetTextSize(wchar_t c);

// 命中测试
int HitTestTextPosition(std::wstring str, float x, float y);
std::vector<DWRITE_HIT_TEST_METRICS> HitTestTextRange(std::wstring str, UINT32 start, UINT32 len);

// 系统字体
static std::vector<std::wstring> GetSystemFonts();
```

#### Colors - 预定义颜色

库提供 900+ 预定义颜色常量：
```cpp
Colors::Red, Colors::Green, Colors::Blue
Colors::White, Colors::Black, Colors::Opacity
Colors::SkyBlue, Colors::LightGray, Colors::DarkOrange
Colors::RoyalBlue, Colors::MediumSeaGreen, Colors::PaleGoldenrod
// ... 更多颜色
```

### 1.3 使用示例

#### 基本绘制

```cpp
D2DGraphics gfx(800, 600);
gfx.BeginRender();
gfx.Clear(Colors::White);

// 矩形
gfx.FillRect(10, 10, 200, 100, Colors::SkyBlue);
gfx.DrawRect(10, 10, 200, 100, Colors::DarkBlue, 2.0f);
gfx.FillRoundRect(220, 10, 200, 100, Colors::LightGreen, 10.0f);

// 圆形
gfx.FillEllipse(110, 200, 80, 80, Colors::Yellow);
gfx.DrawEllipse(110, 200, 80, 80, Colors::Orange, 3.0f);

// 多边形
std::vector<D2D1_POINT_2F> triangle = {
    {400, 100}, {350, 200}, {450, 200}
};
gfx.FillPolygon(triangle, Colors::Pink);

// 文本
Font font(L"Arial", 24);
gfx.DrawString(L"Hello Graphics!", 50, 300, Colors::Black, &font);
gfx.DrawStringOutlined(L"Outlined Text", 50, 350, Colors::White, Colors::Black, &font);

gfx.EndRender();
```

#### 渐变效果

```cpp
// 线性渐变
D2D1_GRADIENT_STOP stops[] = {
    {0.0f, Colors::Red},
    {0.5f, Colors::Yellow},
    {1.0f, Colors::Green}
};
auto linearBrush = gfx.CreateLinearGradientBrush(stops, 3);
linearBrush->SetStartPoint({0, 0});
linearBrush->SetEndPoint({200, 0});
gfx.FillRect(10, 10, 200, 100, linearBrush);
linearBrush->Release();

// 径向渐变
auto radialBrush = gfx.CreateRadialGradientBrush(stops, 3, {110, 110});
radialBrush->SetRadiusX(100);
radialBrush->SetRadiusY(100);
gfx.FillEllipse(110, 110, 100, 100, radialBrush);
radialBrush->Release();
```

#### 图像处理

```cpp
// 加载图像
auto bmpSrc = BitmapSource::FromFile(L"image.png");
auto d2dBitmap = gfx.CreateBitmap(bmpSrc);

// 绘制图像
gfx.DrawBitmap(d2dBitmap, 100, 100);
gfx.DrawBitmap(d2dBitmap, 200, 200, 150, 150, 0.5f); // 缩放 + 透明度

// 加载 GIF
auto gifFrames = BitmapSource::FromGifFile(L"animation.gif");
for (auto& frame : gifFrames) {
    auto bitmap = gfx.CreateBitmap(frame);
    gfx.DrawBitmap(bitmap, 0, 0);
    // ... 逐帧播放
    bitmap->Release();
}

d2dBitmap->Release();
```

#### 窗口渲染

```cpp
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HwndGraphics* gfx = nullptr;
    
    switch (msg) {
    case WM_CREATE:
        gfx = new HwndGraphics(hwnd);
        break;
        
    case WM_PAINT: {
        gfx->BeginRender();
        gfx->Clear(Colors::White);
        gfx->FillRect(50, 50, 200, 100, Colors::SkyBlue);
        gfx->DrawString(L"Window Graphics", 75, 75, Colors::Black);
        gfx->EndRender();
        break;
    }
    
    case WM_SIZE:
        gfx->ReSize(LOWORD(lParam), HIWORD(lParam));
        InvalidateRect(hwnd, NULL, FALSE);
        break;
        
    case WM_DESTROY:
        delete gfx;
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
```

---

## 二、Utils 库详解

### 2.1 文件与目录操作

#### File 类
```cpp
// 检查与基本操作
static bool Exists(const std::string path);
static void Create(const std::string path);
static void Delete(const std::string path);
static void Copy(const std::string src, const std::string dest);
static void Move(const std::string src, const std::string dest);

// 读取
static std::string ReadAllText(const std::string path);
static std::vector<uint8_t> ReadAllBytes(const std::string path);
static std::vector<std::string> ReadAllLines(const std::string path);

// 写入
static void WriteAllText(const std::string path, const std::string content);
static void WriteAllBytes(const std::string path, const std::vector<uint8_t> content);
static void WriteAllLines(const std::string path, const std::vector<std::string> content);

// 追加
static void AppendAllText(const std::string path, const std::string content);
static void AppendAllBytes(const std::string path, const std::vector<uint8_t> content);

// 属性
static FileAttributes GetAttributes(const std::string path);
static void SetAttributes(const std::string path, FileAttributes attributes);
static FILETIME GetCreationTime(const std::string path);
static FILETIME GetLastWriteTime(const std::string path);
```

**使用示例：**
```cpp
// 文本文件
File::WriteAllText("config.txt", "Hello World");
std::string text = File::ReadAllText("config.txt");

// 二进制文件
std::vector<uint8_t> data = {0x89, 0x50, 0x4E, 0x47};
File::WriteAllBytes("data.bin", data);
auto bytes = File::ReadAllBytes("data.bin");

// 行读取
auto lines = File::ReadAllLines("log.txt");
for (const auto& line : lines) {
    std::cout << line << std::endl;
}
```

#### Directory 类
```cpp
static void Create(std::string dirPath);
static bool Exists(std::string dirPath);
static void Delete(std::string dirPath, bool recursive = false);
static std::vector<FileInfo> GetFiles(std::string path);
static std::vector<DirectoryInfo> GetDirectories(std::string path);
```

**使用示例：**
```cpp
Directory::Create("temp");
auto files = Directory::GetFiles("C:\\Windows\\System32");
for (const auto& file : files) {
    std::cout << file.Name << " - " << file.Length << " bytes" << std::endl;
}
```

### 2.2 字符串处理

```cpp
class StringHelper {
    // 分割与连接
    static std::vector<std::string> Split(std::string str, std::string separator);
    static std::string Join(std::vector<std::string> strs, std::string separator);
    
    // 大小写转换
    static std::string ToUpper(std::string str);
    static std::string ToLower(std::string str);
    
    // 修剪
    static std::string Trim(std::string str);
    static std::string TrimLeft(std::string str);
    static std::string TrimRight(std::string str);
    
    // 查找
    static int IndexOf(std::string str, std::string substr);
    static int LastIndexOf(std::string str, std::string substr);
    static bool Contains(std::string str, std::string substr);
    
    // 修改
    static std::string Replace(std::string str, std::string oldstr, std::string newstr);
    static std::string Insert(std::string str, int index, std::string substr);
    static std::string Remove(std::string str, int index, int count);
    
    // 格式化
    static std::string Format(const char* fmt, ...);
    static std::wstring Format(const wchar_t* fmt, ...);
    
    // 哈希
    static int GetHashCode(std::string str);
};
```

**使用示例：**
```cpp
// 分割
auto parts = StringHelper::Split("a,b,c,d", ",");  // {"a", "b", "c", "d"}

// 连接
std::string joined = StringHelper::Join({"Hello", "World"}, " ");  // "Hello World"

// 大小写
std::string upper = StringHelper::ToUpper("hello");  // "HELLO"
std::string lower = StringHelper::ToLower("WORLD");  // "world"

// 修剪
std::string trimmed = StringHelper::Trim("  Hello  ");  // "Hello"

// 替换
std::string replaced = StringHelper::Replace("Hello World", "World", "CppUtils");  // "Hello CppUtils"

// 格式化
std::string formatted = StringHelper::Format("Value: %d, Name: %s", 42, "Test");
```

### 2.3 类型转换

```cpp
class Convert {
    // 十六进制
    static std::string ToHex(const void* input, size_t size);
    static std::vector<uint8_t> FromHex(const std::string hex);
    
    // Base64
    static std::string ToBase64(const void* data, size_t size);
    static std::string FromBase64(const std::string input);
    static std::vector<uint8_t> FromBase64ToBytes(const std::string input);
    
    // 编码转换
    static std::wstring AnsiToUnicode(const std::string ansiStr);
    static std::string UnicodeToAnsi(const std::wstring unicodeStr);
    static std::wstring Utf8ToUnicode(const std::string utf8Str);
    static std::string UnicodeToUtf8(const std::wstring unicodeStr);
    static std::wstring string_to_wstring(const std::string str);
    static std::string wstring_to_string(const std::wstring wstr);
    
    // 哈希 (便捷方法)
    static std::string CalcMD5(const void* data, size_t size);
    static std::string CalcSHA256(const void* data, size_t size);
    
    // 数值转换
    static int ToInt32(const std::string input);
    static long long ToInt64(const std::string input);
    static double ToFloat(const std::string input);
};
```

**使用示例：**
```cpp
// 十六进制
std::vector<uint8_t> data = {0xDE, 0xAD, 0xBE, 0xEF};
std::string hex = Convert::ToHex(data.data(), data.size());  // "DEADBEEF"
auto bytes = Convert::FromHex(hex);

// Base64
std::string base64 = Convert::ToBase64("Hello World");
std::string decoded = Convert::FromBase64(base64);

// 编码转换
std::wstring wide = Convert::string_to_wstring("Hello");
std::string narrow = Convert::wstring_to_string(wide);

// 哈希
std::string md5 = Convert::CalcMD5("password");
std::string sha256 = Convert::CalcSHA256("sensitive data");
```

### 2.4 进程管理

```cpp
class Process {
    int Id;
    std::wstring ProcessName;
    
    Process(int _id);
    
    // 静态方法
    static Process CurrentProcess();
    static Process FromWindow(HWND hwnd);
    static std::vector<Process> GetProcessesByName(const std::string _name);
    static std::vector<Process> GetProcesses();
    static Process Start(const std::string fileName, const std::string arguments = "", const std::string workingDirectory = "");
    
    // 实例方法
    HWND MainWindowHandle();
    std::vector<HWND> Forms();
    std::string MainWindowTitle();
    int ExitCode();
    bool HasExited();
    void Kill();
    bool WaitForExit(int milliseconds = INFINITE);
    
    // 内存信息
    long long VirtualMemorySize();
    long long WorkingSet64();
    long long PagedMemorySize();
    
    // 模块信息
    MODULEINFO MainModule();
    std::vector<MODULEENTRY32> Modules();
    std::vector<HANDLE> Threads();
};
```

**使用示例：**
```cpp
// 启动进程
Process proc = Process::Start("notepad.exe", "document.txt", "C:\\");

// 等待退出
proc.WaitForExit(5000);  // 等待 5 秒
if (!proc.HasExited()) {
    proc.Kill();
}

// 枚举进程
auto processes = Process::GetProcesses();
for (const auto& p : processes) {
    std::wcout << p.ProcessName << " (PID: " << p.Id << ")" << std::endl;
}

// 查找进程
auto chromeProcs = Process::GetProcessesByName("chrome.exe");

// 当前进程信息
auto current = Process::CurrentProcess();
std::cout << "Memory: " << current.WorkingSet64() / 1024 / 1024 << " MB" << std::endl;
```

### 2.5 网络编程

#### TCPSocket
```cpp
class TCPSocket {
    TCPSocket();
    bool Connect(const char* ip, int port);
    bool Listen(int port, int backlog = SOMAXCONN);
    TCPSocket* Accept();
    int Send(const char* data, int length);
    int Receive(char* buffer, int length);
    std::string GetRemoteIP();
    int GetRemotePort();
    void Close();
    bool IsConnected();
};
```

**客户端示例：**
```cpp
TCPSocket socket;
if (socket.Connect("127.0.0.1", 8080)) {
    const char* msg = "Hello Server";
    socket.Send(msg, strlen(msg));
    
    char buffer[1024];
    int received = socket.Receive(buffer, sizeof(buffer));
    buffer[received] = '\0';
    std::cout << "Received: " << buffer << std::endl;
    
    socket.Close();
}
```

**服务器示例：**
```cpp
TCPSocket server;
if (server.Listen(8080)) {
    while (true) {
        TCPSocket* client = server.Accept();
        if (client) {
            char buffer[1024];
            int received = client->Receive(buffer, sizeof(buffer));
            buffer[received] = '\0';
            std::cout << "Client: " << buffer << std::endl;
            
            client->Send("Hello Client", 12);
            client->Close();
            delete client;
        }
    }
}
```

#### HttpHelper (仅支持http)
```cpp
class HttpHelper {
    static std::string UrlEncode(std::string str);
    static std::string Get(std::string url, std::string headers = "", std::string cookies = "");
    static std::vector<uint8_t> HttpGetBytes(std::string url);
    static std::string Post(std::string url, std::string body, std::string headers, std::string cookies = "");
    static std::string GetStream(std::string url, std::string headers = "", std::string cookies = "", HTTP_STREAM_CALLBACK callback = NULL);
};
```

**使用示例：**
```cpp
// GET 请求
std::string html = HttpHelper::Get("http://www.example.com");

// POST 请求
std::string response = HttpHelper::Post(
    "http://api.example.com/login",
    "{\"username\":\"user\",\"password\":\"pass\"}",
    "Content-Type: application/json"
);

// 下载文件
auto imageData = HttpHelper::HttpGetBytes("http://example.com/image.png");
File::WriteAllBytes("image.png", imageData);

// 流式下载 (大文件)
HttpHelper::GetStream("http://example.com/large.zip", "", "", 
    [](std::string chunk) {
        File::AppendAllText("download.tmp", chunk);
    }
);
```

### 2.6 数据库操作

```cpp
class SqliteHelper {
    SqliteHelper(const char* _path);
    
    void Open();
    void Close();
    bool IsTableExist(std::string tableName);
    void CreateTable(std::string tableName, std::vector<Tuple<std::string, SqliteType>> columns);
    void DeleteTable(std::string tableName);
    
    // 查询
    List<List<std::string>> Select(std::string sql);
    void Select(std::string sql, SEL_CALLBAC callback);
    
    // 插入
    bool Insert(const std::string tableName, const std::vector<ColumnValue>& columnValues);
    
    // 执行 SQL
    int Excute(std::string sql);
};
```

**使用示例：**
```cpp
SqliteHelper db("users.db");
db.Open();

// 创建表
db.CreateTable("users", {
    {"name", SqliteType::TEXT},
    {"age", SqliteType::INT32}
});

// 插入数据
db.Insert("users", {
    ColumnValue("name", "Alice"),
    ColumnValue("age", 25)
});

// 查询
auto results = db.Select("SELECT * FROM users WHERE age > 20");
for (const auto& row : results) {
    for (const auto& col : row) {
        std::cout << col << " ";
    }
    std::cout << std::endl;
}

// 回调查询
db.Select("SELECT * FROM users", 
    [](int columns, char** colData, char** colNames, sqlite3_stmt* stmt) {
        for (int i = 0; i < columns; i++) {
            std::cout << colNames[i] << ": " << colData[i] << std::endl;
        }
        return true;  // 继续
    }
);

db.Close();
```

### 2.7 时间与日期

```cpp
class DateTime {
    DateTime();
    DateTime(int years, int months, int days, int hours, int minutes, int seconds, int milliseconds);
    
    // 加减操作
    DateTime AddYears(int years) const;
    DateTime AddMonths(int months) const;
    DateTime AddDays(int days) const;
    DateTime AddHours(int hours) const;
    DateTime AddMinutes(int minutes) const;
    DateTime AddSeconds(int seconds) const;
    
    // 属性
    READONLY_PROPERTY(UINT, Year);
    READONLY_PROPERTY(UINT, Month);
    READONLY_PROPERTY(UINT, Day);
    READONLY_PROPERTY(UINT, Hour);
    READONLY_PROPERTY(UINT, Minute);
    READONLY_PROPERTY(UINT, Second);
    READONLY_PROPERTY(UINT, Milliseconds);
    READONLY_PROPERTY(UINT, DayOfWeek);
    
    // 静态方法
    static DateTime Now();
    static bool IsLeapYear(int year);
    static DateTime Parse(const std::string& str);
    
    std::string ToString() const;
};
```

**使用示例：**
```cpp
DateTime now = DateTime::Now();
std::cout << now.ToString() << std::endl;  // "2025-12-21 10:30:45"

DateTime future = now.AddDays(7).AddHours(3);
std::cout << "7 days later: " << future.ToString() << std::endl;

// 比较
if (now < future) {
    std::cout << "Future is later" << std::endl;
}

// 闰年判断
bool isLeap = DateTime::IsLeapYear(2024);  // true
```

### 2.8 注册表操作

```cpp
class RegistryKey {
    RegistryKey CreateSubKey(const std::string& subKey);
    RegistryKey OpenSubKey(const std::string& subKey, bool writable = false);
    std::string GetValue(const std::string& name);
    void SetValue(const std::string& name, const std::string& value);
    void DeleteValue(const std::string& name);
    void DeleteSubKey(const std::string& subKey);
    std::vector<std::string> GetSubKeyNames();
    std::vector<std::string> GetValueNames();
    void Close();
};

class Registry {
    static RegistryKey OpenBaseKey(HKEY hKey, const std::string& subKey, bool writable = false);
};

// 预定义根键
extern RegistryKey ClassesRoot;
extern RegistryKey CurrentConfig;
extern RegistryKey CurrentUser;
extern RegistryKey LocalMachine;
extern RegistryKey Users;
```

**使用示例：**
```cpp
// 读取
auto key = Registry::OpenBaseKey(HKEY_CURRENT_USER, "Software\\MyApp");
std::string value = key.GetValue("Setting1");

// 写入
auto writable = Registry::OpenBaseKey(HKEY_CURRENT_USER, "Software\\MyApp", true);
writable.SetValue("Setting1", "NewValue");

// 枚举子键
auto subKeys = key.GetSubKeyNames();
for (const auto& subKey : subKeys) {
    std::cout << subKey << std::endl;
}
```

### 2.9 其他实用工具

#### Guid - GUID 生成
```cpp
Guid guid = Guid::NewGuid();
std::string str = guid.ToString();  // "{550E8400-E29B-41D4-A716-446655440000}"
```

#### Random - 随机数
```cpp
Random rand;
int value = rand.Next(100);          // 0-99
double dvalue = rand.NextDouble();    // 0.0-1.0
```

#### Clipboard - 剪贴板
```cpp
Clipboard::SetText("Hello World");
std::string text = Clipboard::GetText();
```

#### 压缩 (zlib)
```cpp
// 压缩
std::vector<uint8_t> data = {/* ... */};
std::vector<uint8_t> compressed = GCompress(data);

// 解压
std::vector<uint8_t> decompressed = GDecompress(compressed);
```

#### JSON
```cpp
json j;
j["name"] = "Alice";
j["age"] = 25;
j["skills"] = {"C++", "Python", "JavaScript"};

std::string jsonStr = j.dump();  // 序列化
json parsed = json::parse(jsonStr);  // 反序列化

std::string name = parsed["name"];
int age = parsed["age"];
```

---

## 三、高级技巧

### 3.1 属性宏

库提供了类似 C# 的属性语法：

```cpp
class MyClass {
private:
    int _value;
    
public:
    PROPERTY(int, Value);
    GET(int, Value) { return _value; }
    SET(int, Value) { _value = value; }
};

// 使用
MyClass obj;
obj.Value = 42;           // 调用 SetValue
int v = obj.Value;        // 调用 GetValue
```

### 3.2 模式匹配 (Utils)

```cpp
// 查找内存特征码
void* address = FindPattern("kernel32.dll", "48 8B ? ? ? 00 00", 0);
std::vector<void*> addresses = FindAllPattern("user32.dll", "FF 15 ? ? ? ?", 2);

// 解析特征码
auto pattern = ParserPattern("48 8B ? ? ? 00 00");
```

### 3.3 自定义绘制

```cpp
// 自定义几何路径
ComPtr<ID2D1PathGeometry> geo;
geo.Attach(Factory::CreateGeomtry());

ComPtr<ID2D1GeometrySink> sink;
geo->Open(&sink);
sink->BeginFigure({100, 100}, D2D1_FIGURE_BEGIN_FILLED);
sink->AddLine({200, 100});
sink->AddLine({150, 200});
sink->EndFigure(D2D1_FIGURE_END_CLOSED);
sink->Close();

gfx.FillGeometry(geo.Get(), Colors::Red);
```

---

## 四、架构与配置

### 4.1 项目结构

```
CppUtils/
├── Graphics/               # 图形库源码
│   ├── BitmapSource.h/cpp      # 位图封装
│   ├── Colors.h                # 颜色常量
│   ├── Factory.h/cpp           # Direct2D 工厂
│   ├── Font.h/cpp              # 字体处理
│   ├── Graphics.h/cpp          # 基础图形类
│   └── Graphics1.h/cpp         # 增强图形类
├── Utils/                  # 工具库源码
│   ├── defines.h               # 宏定义
│   ├── Utils.h                 # 主头文件
│   ├── StringHelper.h/cpp      # 字符串工具
│   ├── File.h/cpp              # 文件操作
│   ├── Process.h/cpp           # 进程管理
│   ├── Socket.h/cpp            # 网络编程
│   ├── HttpHelper.h/cpp        # HTTP 客户端
│   ├── SqliteHelper.h/cpp      # SQLite 封装
│   ├── Convert.h/cpp           # 类型转换
│   ├── DateTime.h/cpp          # 日期时间
│   ├── Registry.h/cpp          # 注册表
│   ├── Guid.h/cpp              # GUID 生成
│   ├── Clipboard.h/cpp         # 剪贴板
│   ├── zlib/                   # zlib 库
│   └── sqlite/                 # SQLite 库
├── include/                # 头文件副本（用于安装）
│   ├── Graphics/
│   ├── Utils/
│   └── ...
├── UtilsTest/              # 测试项目
└── build/                  # 输出目录
    ├── Graphics_x64_MT.lib
    ├── Graphics_x64_MTd.lib
    ├── CppUtils_x64_MT.lib
    └── CppUtils_x64_MTd.lib
```

### 4.2 编译配置

#### 支持的配置
- **架构**: x86, x64
- **配置**: Debug, Release
- **运行时**: MT (静态), MD (动态)
- **C++ 标准**: C++17
- **工具集**: Visual Studio 2022 (v143)

#### 依赖项

**Graphics 库:**
- `d2d1.lib` - Direct2D
- `dwrite.lib` - DirectWrite
- `dxgi.lib` - DXGI
- `d3d11.lib` - Direct3D 11
- `windowscodecs.lib` - WIC

**Utils 库:**
- `ws2_32.lib` - Winsock
- `wininet.lib` - WinINet
- `wtsapi32.lib` - WTS API
- `dbghelp.lib` - Debug Help

### 4.3 自动链接机制

两个库都实现了智能自动链接：

```cpp
// Graphics.h 中的自动链接逻辑
#ifndef _LIB
    #if defined(_MT) && !defined(_DLL)  // MT
        #if defined(_M_X64) && defined(_DEBUG)
            #pragma comment(lib, "Graphics_x64_MTd.lib")
        #elif defined(_M_X64) && !defined(_DEBUG)
            #pragma comment(lib, "Graphics_x64_MT.lib")
        // ... x86 配置
    #else  // MD
        #if defined(_M_X64) && defined(_DEBUG)
            #pragma comment(lib, "Graphics_x64_MDd.lib")
        // ... 其他配置
    #endif
#endif
```

---

## 五、注意事项

### 5.1 平台要求
- **操作系统**: Windows 7 或更高版本
- **DirectX**: 需要 DirectX 11 支持 (Graphics 库)
- **运行时**: Visual C++ Redistributable 2015-2022

### 5.2 线程安全
- **Graphics**: 图形对象 **不是** 线程安全的，每个线程应使用独立实例
- **Utils**: 大部分工具类是线程安全的，但文件流等需要注意

### 5.3 性能建议
1. **重用对象**: 重用 Graphics 对象和画刷，避免频繁创建销毁
2. **批量绘制**: 在 BeginRender/EndRender 之间批量绘制操作
3. **位图缓存**: 缓存常用位图，避免重复创建
4. **离屏渲染**: 复杂场景使用 CompatibleGraphics 离屏渲染后再绘制

### 5.4 错误处理
- 大部分函数失败时返回空值或默认值
- 关键操作应检查返回值
- 使用 `GetLastError()` 获取详细错误信息
- Graphics 类在设备丢失时会自动尝试重建

### 5.5 内存管理
- COM 对象需要手动 `Release()`
- 使用智能指针 (`ComPtr`) 管理 COM 对象更安全
- BitmapSource 使用 `std::shared_ptr` 自动管理

---

## 六、常见问题 (FAQ)

### Q1: 为什么包含头文件后链接失败？
**A**: 确保：
1. 编译配置 (x86/x64, Debug/Release, MT/MD) 匹配
2. 库文件在系统路径或项目配置的库路径中
3. 没有定义 `_LIB` 宏 (用于库本身编译)

### Q2: Graphics 渲染黑屏或空白？
**A**: 检查：
1. 是否调用了 `BeginRender()` 和 `EndRender()`
2. 窗口大小是否正确 (至少 1x1)
3. 是否安装了 DirectX 运行时
4. 查看调试输出的 Direct2D 错误信息

### Q3: 中文字符串显示乱码？
**A**: 
- 使用 `std::wstring` 和 L"中文" 字面量
- 或使用 `Convert::Utf8ToUnicode()` 转换

### Q4: 如何处理大文件？
**A**: 
- 使用 `FileStream` 类进行流式读写
- 或使用 `HttpHelper::GetStream()` 分块下载

### Q5: SQLite 操作失败？
**A**: 
- 确保数据库文件路径正确且有写权限
- 检查 SQL 语法是否正确
- 调用 `Open()` 后检查返回值

---

## 七、示例项目

### 完整示例：图像查看器

```cpp
#include <Graphics/Graphics.h>
#include <Utils/Utils.h>

class ImageViewer {
private:
    HwndGraphics* gfx = nullptr;
    ID2D1Bitmap* currentBitmap = nullptr;
    float zoom = 1.0f;
    D2D1_POINT_2F offset = {0, 0};
    
public:
    void Initialize(HWND hwnd) {
        gfx = new HwndGraphics(hwnd);
    }
    
    void LoadImage(const std::wstring& path) {
        if (currentBitmap) {
            currentBitmap->Release();
            currentBitmap = nullptr;
        }
        
        auto bmpSrc = BitmapSource::FromFile(path);
        if (bmpSrc) {
            currentBitmap = gfx->CreateBitmap(bmpSrc);
            zoom = 1.0f;
            offset = {0, 0};
        }
    }
    
    void Render() {
        gfx->BeginRender();
        gfx->Clear(Colors::White);
        
        if (currentBitmap) {
            // 应用缩放和平移
            D2D1_MATRIX_3X2_F transform = 
                D2D1::Matrix3x2F::Scale(zoom, zoom) *
                D2D1::Matrix3x2F::Translation(offset.x, offset.y);
            gfx->SetTransform(transform);
            
            gfx->DrawBitmap(currentBitmap, 0, 0);
            gfx->ClearTransform();
            
            // 显示信息
            auto size = currentBitmap->GetSize();
            Font font(L"Arial", 14);
            std::wstring info = StringHelper::Format(L"Size: %.0fx%.0f  Zoom: %.0f%%", 
                size.width, size.height, zoom * 100);
            gfx->DrawString(info, 10, 10, Colors::Black, &font);
        }
        
        gfx->EndRender();
    }
    
    void Zoom(float delta) {
        zoom = std::max(0.1f, std::min(10.0f, zoom + delta));
    }
    
    ~ImageViewer() {
        if (currentBitmap) currentBitmap->Release();
        delete gfx;
    }
};
```

---

## 八、更新日志

### v1.0 (当前版本)
- ✅ 完整的 Direct2D/Direct3D 图形引擎
- ✅ 双版本图形类 (D2DGraphics 和 D2DGraphics1)
- ✅ 统一位图封装 (BitmapSource)
- ✅ GIF 动画支持
- ✅ 900+ 预定义颜色
- ✅ 完整的文件、网络、数据库工具类
- ✅ 自动链接机制
- ✅ x86/x64 全架构支持
- ✅ MT/MD 运行时支持

---

## 九、许可与贡献

本项目为个人或团队内部使用工具库。

**技术支持**: 遇到问题请检查：
1. 编译配置是否匹配
2. 依赖项是否正确链接
3. DirectX 和 Visual C++ 运行时是否安装
4. 查看项目 issues 或示例代码

**反馈与改进**: 欢迎提出建议和改进意见。

---

## 快速参考

### 常用头文件
```cpp
#include <Graphics/Graphics.h>
#include <Graphics/Graphics1.h>
#include <Graphics/BitmapSource.h>
#include <Graphics/Font.h>
#include <Graphics/Colors.h>

#include <Utils/Utils.h>           // 包含所有 Utils 类
#include <Utils/StringHelper.h>
#include <Utils/File.h>
#include <Utils/Process.h>
#include <Utils/Convert.h>
// ... 或单独包含
```

### 命名空间
```cpp
using json = JsonLib::json;
// Colors 命名空间已自动可用
```

### 最小示例
```cpp
#include <Graphics/Graphics.h>
#include <Utils/Utils.h>

int main() {
    // 图形
    D2DGraphics gfx(400, 300);
    gfx.BeginRender();
    gfx.Clear(Colors::White);
    gfx.FillRect(50, 50, 100, 75, Colors::SkyBlue);
    gfx.EndRender();
    
    // 工具
    File::WriteAllText("test.txt", "Hello!");
    std::string text = File::ReadAllText("test.txt");
    
    return 0;
}
```

---

**项目版本**: 1.0  
**文档更新**: 2025-12-21  
**兼容性**: Windows 7+, Visual Studio 2022, C++17
QQ群 = 522222570
