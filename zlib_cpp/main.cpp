#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cassert>
#include <chrono>
#include "ZlibCpp.h"

using namespace ZlibCpp;

// 辅助函数：打印字节数组
void PrintBytes(const std::vector<uint8_t>& data, const std::string& title) {
    std::cout << title << " (大小: " << data.size() << " 字节): ";
    for (size_t i = 0; i < std::min(data.size(), size_t(20)); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
    }
    if (data.size() > 20) {
        std::cout << "...";
    }
    std::cout << std::dec << std::endl;
}

// 测试基本压缩和解压缩
void TestBasicCompression() {
    std::cout << "\n=== 测试基本压缩和解压缩 ===" << std::endl;
    
    try {
        std::string original = "这是一个测试字符串，用于验证zlib压缩和解压缩功能。Hello World! 1234567890";
        std::cout << "原始字符串: " << original << std::endl;
        std::cout << "原始大小: " << original.size() << " 字节" << std::endl;

        // 压缩
        auto compressed = ZlibUtils::Compress(original);
        PrintBytes(compressed, "压缩后数据");

        // 解压缩
        std::string decompressed = ZlibUtils::DecompressToString(compressed);
        std::cout << "解压缩后: " << decompressed << std::endl;

        // 验证
        assert(original == decompressed);
        std::cout << "✓ 基本压缩解压缩测试通过！" << std::endl;

        // 压缩率
        double ratio = (double)compressed.size() / original.size();
        std::cout << "压缩率: " << std::fixed << std::setprecision(2) << (ratio * 100) << "%" << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "✗ 测试失败: " << e.what() << std::endl;
    }
}

// 测试不同格式
void TestDifferentFormats() {
    std::cout << "\n=== 测试不同数据格式 ===" << std::endl;
    
    try {
        std::string data = "这是一个用于测试不同压缩格式的长字符串。" + std::string(100, 'A');
        
        // 测试Zlib格式
        auto zlib_compressed = ZlibUtils::Compress(data, CompressionLevel::DefaultCompression, DataFormat::Zlib);
        auto zlib_decompressed = ZlibUtils::DecompressToString(zlib_compressed, DataFormat::Zlib);
        assert(data == zlib_decompressed);
        std::cout << "✓ Zlib格式测试通过！压缩大小: " << zlib_compressed.size() << std::endl;

        // 测试Gzip格式
        auto gzip_compressed = ZlibUtils::Compress(data, CompressionLevel::DefaultCompression, DataFormat::Gzip);
        auto gzip_decompressed = ZlibUtils::DecompressToString(gzip_compressed, DataFormat::Gzip);
        assert(data == gzip_decompressed);
        std::cout << "✓ Gzip格式测试通过！压缩大小: " << gzip_compressed.size() << std::endl;

        // 测试Raw格式
        auto raw_compressed = ZlibUtils::Compress(data, CompressionLevel::DefaultCompression, DataFormat::Raw);
        auto raw_decompressed = ZlibUtils::DecompressToString(raw_compressed, DataFormat::Raw);
        assert(data == raw_decompressed);
        std::cout << "✓ Raw格式测试通过！压缩大小: " << raw_compressed.size() << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "✗ 测试失败: " << e.what() << std::endl;
    }
}

// 测试流式压缩
void TestStreamCompression() {
    std::cout << "\n=== 测试流式压缩 ===" << std::endl;
    
    try {
        ZlibCompressor compressor;
        std::vector<uint8_t> result;

        // 分块压缩
        std::string part1 = "第一部分数据：";
        std::string part2 = "第二部分数据：";
        std::string part3 = "第三部分数据结束。";

        auto chunk1 = compressor.Compress(std::vector<uint8_t>(part1.begin(), part1.end()));
        result.insert(result.end(), chunk1.begin(), chunk1.end());

        auto chunk2 = compressor.Compress(std::vector<uint8_t>(part2.begin(), part2.end()));
        result.insert(result.end(), chunk2.begin(), chunk2.end());

        auto chunk3 = compressor.Compress(std::vector<uint8_t>(part3.begin(), part3.end()));
        result.insert(result.end(), chunk3.begin(), chunk3.end());

        auto final_chunk = compressor.Finish();
        result.insert(result.end(), final_chunk.begin(), final_chunk.end());

        // 解压缩
        std::string original = part1 + part2 + part3;
        std::string decompressed = ZlibUtils::DecompressToString(result);
        
        assert(original == decompressed);
        std::cout << "✓ 流式压缩测试通过！" << std::endl;
        std::cout << "原始大小: " << original.size() << ", 压缩大小: " << result.size() << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "✗ 测试失败: " << e.what() << std::endl;
    }
}

// 测试不同压缩级别
void TestCompressionLevels() {
    std::cout << "\n=== 测试不同压缩级别 ===" << std::endl;
    
    try {
        std::string data = "这是一个重复的测试字符串。" + std::string(1000, 'X') + "结束标记。";
        
        struct LevelTest {
            CompressionLevel level;
            std::string name;
        };

        std::vector<LevelTest> levels = {
            {CompressionLevel::NoCompression, "无压缩"},
            {CompressionLevel::BestSpeed, "最快速度"},
            {CompressionLevel::DefaultCompression, "默认压缩"},
            {CompressionLevel::BestCompression, "最佳压缩"}
        };

        for (const auto& test : levels) {
            auto compressed = ZlibUtils::Compress(data, test.level);
            auto decompressed = ZlibUtils::DecompressToString(compressed);
            
            assert(data == decompressed);
            double ratio = (double)compressed.size() / data.size();
            std::cout << "✓ " << test.name << ": 压缩大小=" << compressed.size() 
                      << ", 压缩率=" << std::fixed << std::setprecision(1) << (ratio * 100) << "%" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "✗ 测试失败: " << e.what() << std::endl;
    }
}

// 测试工具函数
void TestUtilityFunctions() {
    std::cout << "\n=== 测试工具函数 ===" << std::endl;
    
    try {
        std::vector<uint8_t> test_data = {0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD, 0xEF};
        
        // 测试Adler32
        uint32_t adler32 = ZlibUtils::CalculateAdler32(test_data);
        std::cout << "Adler32校验和: 0x" << std::hex << adler32 << std::dec << std::endl;

        // 测试CRC32
        uint32_t crc32 = ZlibUtils::CalculateCRC32(test_data);
        std::cout << "CRC32校验和: 0x" << std::hex << crc32 << std::dec << std::endl;

        // 测试十六进制编码
        std::string hex = ZlibUtils::HexEncode(test_data);
        auto hex_decoded = ZlibUtils::HexDecode(hex);
        assert(test_data == hex_decoded);
        std::cout << "✓ 十六进制编码/解码测试通过！" << std::endl;

        // 测试Base64编码
        std::string base64 = ZlibUtils::Base64Encode(test_data);
        auto base64_decoded = ZlibUtils::Base64Decode(base64);
        assert(test_data == base64_decoded);
        std::cout << "✓ Base64编码/解码测试通过！" << std::endl;

        // 测试格式检测
        auto zlib_data = ZlibUtils::Compress(test_data, CompressionLevel::DefaultCompression, DataFormat::Zlib);
        auto gzip_data = ZlibUtils::Compress(test_data, CompressionLevel::DefaultCompression, DataFormat::Gzip);
        
        assert(ZlibUtils::DetectFormat(zlib_data) == DataFormat::Zlib);
        assert(ZlibUtils::DetectFormat(gzip_data) == DataFormat::Gzip);
        std::cout << "✓ 格式检测测试通过！" << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "✗ 测试失败: " << e.what() << std::endl;
    }
}

// 测试异常处理
void TestExceptionHandling() {
    std::cout << "\n=== 测试异常处理 ===" << std::endl;
    
    try {
        // 测试无效数据解压缩
        std::vector<uint8_t> invalid_data = {0x00, 0x01, 0x02, 0x03};
        try {
            ZlibUtils::DecompressToString(invalid_data);
            std::cout << "✗ 应该抛出异常但没有抛出" << std::endl;
        }
        catch (const ZlibDataException& e) {
            std::cout << "✓ 正确捕获数据错误异常: " << e.what() << std::endl;
        }

        // 测试重复初始化
        try {
            ZlibCompressor compressor;
            compressor.Initialize();  // 应该可以重复初始化
            std::cout << "✓ 重复初始化测试通过" << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "✗ 重复初始化测试失败: " << e.what() << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "✗ 异常处理测试失败: " << e.what() << std::endl;
    }
}

// 性能测试
void TestPerformance() {
    std::cout << "\n=== 性能测试 ===" << std::endl;
    
    try {
        // 生成测试数据
        std::vector<uint8_t> large_data;
        large_data.reserve(1024 * 1024);  // 1MB
        for (int i = 0; i < 1024 * 1024; ++i) {
            large_data.push_back(static_cast<uint8_t>(i % 256));
        }

        std::cout << "测试数据大小: " << large_data.size() << " 字节" << std::endl;

        // 压缩测试
        auto start = std::chrono::high_resolution_clock::now();
        auto compressed = ZlibUtils::Compress(large_data);
        auto compress_end = std::chrono::high_resolution_clock::now();

        // 解压缩测试
        auto decompressed = ZlibUtils::Decompress(compressed);
        auto decompress_end = std::chrono::high_resolution_clock::now();

        // 验证
        assert(large_data == decompressed);

        auto compress_time = std::chrono::duration_cast<std::chrono::milliseconds>(compress_end - start).count();
        auto decompress_time = std::chrono::duration_cast<std::chrono::milliseconds>(decompress_end - compress_end).count();

        double ratio = (double)compressed.size() / large_data.size();
        double compress_speed = (double)large_data.size() / compress_time / 1024;  // KB/ms
        double decompress_speed = (double)decompressed.size() / decompress_time / 1024;  // KB/ms

        std::cout << "压缩时间: " << compress_time << " ms" << std::endl;
        std::cout << "解压缩时间: " << decompress_time << " ms" << std::endl;
        std::cout << "压缩率: " << std::fixed << std::setprecision(1) << (ratio * 100) << "%" << std::endl;
        std::cout << "压缩速度: " << std::fixed << std::setprecision(1) << compress_speed << " KB/ms" << std::endl;
        std::cout << "解压缩速度: " << std::fixed << std::setprecision(1) << decompress_speed << " KB/ms" << std::endl;
        std::cout << "✓ 性能测试通过！" << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "✗ 性能测试失败: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== ZlibCpp 测试程序 ===" << std::endl;
    std::cout << "库版本: " << ZlibCpp::GetLibraryVersion() << std::endl;
    std::cout << "底层zlib版本: " << ZlibCpp::GetZlibVersion() << std::endl;

    // 运行所有测试
    TestBasicCompression();
    TestDifferentFormats();
    TestStreamCompression();
    TestCompressionLevels();
    TestUtilityFunctions();
    TestExceptionHandling();
    TestPerformance();

    std::cout << "\n=== 所有测试完成 ===" << std::endl;
    
    // 暂停等待用户输入
    std::cout << "\n按Enter键退出...";
    std::cin.get();
    
    return 0;
} 