# ZlibCpp - zlib C++面向对象封装库

## 概述

ZlibCpp是对经典zlib压缩库的C++面向对象封装，提供了现代C++风格的API，支持多种数据格式和压缩策略。

## 特性

- 🔧 **面向对象设计**: 使用现代C++设计模式
- 📦 **多格式支持**: 支持Zlib、Gzip、Raw Deflate格式
- 🚀 **高性能**: 基于原始zlib库，保持高效性能
- 🛡️ **异常安全**: 完整的异常处理机制
- 📝 **易于使用**: 提供简单的静态方法和流式API
- 🔍 **丰富的工具**: 包含校验和计算、格式检测等工具

## 核心类结构

### ZlibException 异常类层次
```cpp
ZlibException (基类)
├── ZlibMemoryException (内存错误)
├── ZlibDataException (数据错误)
├── ZlibStreamException (流错误)
├── ZlibBufferException (缓冲区错误)
└── ZlibVersionException (版本错误)
```

### 主要类
- **ZlibBase**: 基础抽象类
- **ZlibCompressor**: 压缩器类
- **ZlibDecompressor**: 解压缩器类
- **ZlibUtils**: 工具类，提供便捷的静态方法

## 快速开始

### 包含头文件
```cpp
#include "ZlibCpp.h"
using namespace ZlibCpp;
```

### 基本用法

#### 1. 简单压缩/解压缩
```cpp
// 压缩字符串
std::string data = "Hello, World!";
auto compressed = ZlibUtils::Compress(data);

// 解压缩
std::string decompressed = ZlibUtils::DecompressToString(compressed);
```

#### 2. 指定压缩级别和格式
```cpp
auto compressed = ZlibUtils::Compress(data, 
    CompressionLevel::BestCompression, 
    DataFormat::Gzip);
```

#### 3. 流式压缩
```cpp
ZlibCompressor compressor(CompressionLevel::DefaultCompression, DataFormat::Zlib);

// 分块压缩
auto chunk1 = compressor.Compress(data1);
auto chunk2 = compressor.Compress(data2);
auto final = compressor.Finish();

// 合并结果
std::vector<uint8_t> result;
result.insert(result.end(), chunk1.begin(), chunk1.end());
result.insert(result.end(), chunk2.begin(), chunk2.end());
result.insert(result.end(), final.begin(), final.end());
```

### 压缩级别

```cpp
enum class CompressionLevel {
    NoCompression = 0,        // 不压缩
    BestSpeed = 1,           // 最快速度
    BestCompression = 9,     // 最佳压缩
    DefaultCompression = -1  // 默认压缩
};
```

### 数据格式

```cpp
enum class DataFormat {
    Zlib = 0,    // 标准zlib格式 (RFC 1950)
    Gzip = 16,   // Gzip格式 (RFC 1952)
    Raw = -1     // 原始deflate格式 (RFC 1951)
};
```

## 高级用法

### 文件压缩
```cpp
// 压缩文件
bool success = ZlibUtils::CompressFile("input.txt", "output.zlib", 
    CompressionLevel::BestCompression);

// 解压缩文件
bool success = ZlibUtils::DecompressFile("output.zlib", "restored.txt");
```

### 校验和计算
```cpp
std::vector<uint8_t> data = {0x12, 0x34, 0x56, 0x78};

uint32_t adler32 = ZlibUtils::CalculateAdler32(data);
uint32_t crc32 = ZlibUtils::CalculateCRC32(data);
```

### 格式检测
```cpp
auto format = ZlibUtils::DetectFormat(compressed_data);
switch (format) {
    case DataFormat::Zlib:
        std::cout << "Zlib格式" << std::endl;
        break;
    case DataFormat::Gzip:
        std::cout << "Gzip格式" << std::endl;
        break;
    case DataFormat::Raw:
        std::cout << "Raw deflate格式" << std::endl;
        break;
}
```

### 编码工具
```cpp
// Base64编码
std::string base64 = ZlibUtils::Base64Encode(data);
auto decoded = ZlibUtils::Base64Decode(base64);

// 十六进制编码
std::string hex = ZlibUtils::HexEncode(data);
auto hex_decoded = ZlibUtils::HexDecode(hex);
```

## 异常处理

```cpp
try {
    auto compressed = ZlibUtils::Compress(data);
    auto decompressed = ZlibUtils::DecompressToString(compressed);
}
catch (const ZlibMemoryException& e) {
    std::cout << "内存不足: " << e.what() << std::endl;
}
catch (const ZlibDataException& e) {
    std::cout << "数据错误: " << e.what() << std::endl;
}
catch (const ZlibException& e) {
    std::cout << "zlib错误: " << e.what() << std::endl;
}
```

## 性能特点

- **高效内存管理**: 使用内存池减少内存分配开销
- **流式处理**: 支持大文件的流式压缩，避免内存溢出
- **缓冲区优化**: 使用8KB缓冲区平衡内存使用和性能
- **异常安全**: RAII设计确保资源正确释放

## 构建要求

- C++11或更高版本
- Visual Studio 2017或更新版本 (Windows)
- 或支持C++11的GCC/Clang (Linux/macOS)

## 版本信息

```cpp
std::cout << "库版本: " << ZlibCpp::GetLibraryVersion() << std::endl;
std::cout << "zlib版本: " << ZlibCpp::GetZlibVersion() << std::endl;
```

## 示例程序

运行`main.cpp`中的测试程序可以看到完整的功能演示：

```bash
# 编译并运行
# Visual Studio: 直接构建项目
# 或使用命令行编译所有源文件
```

## 许可证

本项目基于zlib许可证，与原始zlib库保持一致。

## 贡献

欢迎提交Issue和Pull Request来改进这个库。

---

**注意**: 这个库是对原始zlib的封装，保持了zlib的所有性能特性，同时提供了更友好的C++接口。 