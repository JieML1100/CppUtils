#pragma once
#include "ZlibCompressor.h"
#include "ZlibDecompressor.h"
#include <vector>
#include <string>
#include <fstream>

namespace ZlibCpp {

    /**
     * @brief zlib工具类，提供便捷的静态方法
     */
    class ZlibUtils {
    public:
        /**
         * @brief 压缩字符串
         * @param data 输入字符串
         * @param level 压缩级别
         * @param format 数据格式
         * @return 压缩后的数据
         */
        static std::vector<uint8_t> Compress(const std::string& data,
                                           CompressionLevel level = CompressionLevel::DefaultCompression,
                                           DataFormat format = DataFormat::Zlib);

        /**
         * @brief 压缩字节数据
         * @param data 输入数据
         * @param level 压缩级别
         * @param format 数据格式
         * @return 压缩后的数据
         */
        static std::vector<uint8_t> Compress(const std::vector<uint8_t>& data,
                                           CompressionLevel level = CompressionLevel::DefaultCompression,
                                           DataFormat format = DataFormat::Zlib);

        /**
         * @brief 解压缩到字符串
         * @param data 压缩数据
         * @param format 数据格式
         * @return 解压缩后的字符串
         */
        static std::string DecompressToString(const std::vector<uint8_t>& data,
                                            DataFormat format = DataFormat::Zlib);

        /**
         * @brief 解压缩到字节数据
         * @param data 压缩数据
         * @param format 数据格式
         * @return 解压缩后的数据
         */
        static std::vector<uint8_t> Decompress(const std::vector<uint8_t>& data,
                                             DataFormat format = DataFormat::Zlib);

        /**
         * @brief 压缩文件
         * @param input_file 输入文件路径
         * @param output_file 输出文件路径
         * @param level 压缩级别
         * @param format 数据格式
         * @return 是否成功
         */
        static bool CompressFile(const std::string& input_file,
                               const std::string& output_file,
                               CompressionLevel level = CompressionLevel::DefaultCompression,
                               DataFormat format = DataFormat::Zlib);

        /**
         * @brief 解压缩文件
         * @param input_file 输入文件路径
         * @param output_file 输出文件路径
         * @param format 数据格式
         * @return 是否成功
         */
        static bool DecompressFile(const std::string& input_file,
                                 const std::string& output_file,
                                 DataFormat format = DataFormat::Zlib);

        /**
         * @brief 计算Adler32校验和
         * @param data 输入数据
         * @return Adler32校验和
         */
        static uint32_t CalculateAdler32(const std::vector<uint8_t>& data);

        /**
         * @brief 计算CRC32校验和
         * @param data 输入数据
         * @return CRC32校验和
         */
        static uint32_t CalculateCRC32(const std::vector<uint8_t>& data);

        /**
         * @brief 获取zlib版本信息
         * @return 版本字符串
         */
        static std::string GetVersion();

        /**
         * @brief 检查数据格式
         * @param data 输入数据
         * @return 数据格式
         */
        static DataFormat DetectFormat(const std::vector<uint8_t>& data);

        /**
         * @brief 创建Gzip压缩器
         * @param level 压缩级别
         * @return Gzip压缩器实例
         */
        static std::unique_ptr<ZlibCompressor> CreateGzipCompressor(
            CompressionLevel level = CompressionLevel::DefaultCompression);

        /**
         * @brief 创建Gzip解压缩器
         * @return Gzip解压缩器实例
         */
        static std::unique_ptr<ZlibDecompressor> CreateGzipDecompressor();

        /**
         * @brief Base64编码
         * @param data 输入数据
         * @return Base64编码字符串
         */
        static std::string Base64Encode(const std::vector<uint8_t>& data);

        /**
         * @brief Base64解码
         * @param encoded Base64编码字符串
         * @return 解码后的数据
         */
        static std::vector<uint8_t> Base64Decode(const std::string& encoded);

        /**
         * @brief 十六进制编码
         * @param data 输入数据
         * @return 十六进制字符串
         */
        static std::string HexEncode(const std::vector<uint8_t>& data);

        /**
         * @brief 十六进制解码
         * @param hex 十六进制字符串
         * @return 解码后的数据
         */
        static std::vector<uint8_t> HexDecode(const std::string& hex);

    private:
        // 工具类，禁用实例化
        ZlibUtils() = delete;
        ~ZlibUtils() = delete;
        ZlibUtils(const ZlibUtils&) = delete;
        ZlibUtils& operator=(const ZlibUtils&) = delete;

        /**
         * @brief 读取文件到内存
         */
        static std::vector<uint8_t> ReadFile(const std::string& filename);

        /**
         * @brief 写入数据到文件
         */
        static bool WriteFile(const std::string& filename, const std::vector<uint8_t>& data);
    };

} // namespace ZlibCpp 