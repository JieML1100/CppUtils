#pragma once
#include "ZlibBase.h"
#include <vector>
#include <string>

namespace ZlibCpp {

    /**
     * @brief zlib压缩器类
     */
    class ZlibCompressor : public ZlibBase {
    public:
        /**
         * @brief 构造函数
         * @param level 压缩级别
         * @param format 数据格式
         * @param strategy 压缩策略
         */
        ZlibCompressor(CompressionLevel level = CompressionLevel::DefaultCompression,
                      DataFormat format = DataFormat::Zlib,
                      CompressionStrategy strategy = CompressionStrategy::Default);

        /**
         * @brief 析构函数
         */
        ~ZlibCompressor() override;

        /**
         * @brief 初始化压缩器
         * @param level 压缩级别
         * @param format 数据格式
         * @param strategy 压缩策略
         */
        void Initialize(CompressionLevel level = CompressionLevel::DefaultCompression,
                       DataFormat format = DataFormat::Zlib,
                       CompressionStrategy strategy = CompressionStrategy::Default);

        /**
         * @brief 重置压缩器状态
         */
        void Reset() override;

        /**
         * @brief 压缩数据
         * @param input 输入数据
         * @param flush 刷新模式
         * @return 压缩后的数据
         */
        std::vector<uint8_t> Compress(const std::vector<uint8_t>& input, bool flush = false);

        /**
         * @brief 压缩数据
         * @param input 输入数据指针
         * @param input_size 输入数据大小
         * @param flush 刷新模式
         * @return 压缩后的数据
         */
        std::vector<uint8_t> Compress(const uint8_t* input, size_t input_size, bool flush = false);

        /**
         * @brief 完成压缩
         * @return 剩余的压缩数据
         */
        std::vector<uint8_t> Finish();

        /**
         * @brief 一次性压缩数据
         * @param input 输入数据
         * @param level 压缩级别
         * @param format 数据格式
         * @return 压缩后的数据
         */
        static std::vector<uint8_t> CompressData(const std::vector<uint8_t>& input,
                                                CompressionLevel level = CompressionLevel::DefaultCompression,
                                                DataFormat format = DataFormat::Zlib);

        /**
         * @brief 一次性压缩字符串
         * @param input 输入字符串
         * @param level 压缩级别
         * @param format 数据格式
         * @return 压缩后的数据
         */
        static std::vector<uint8_t> CompressString(const std::string& input,
                                                  CompressionLevel level = CompressionLevel::DefaultCompression,
                                                  DataFormat format = DataFormat::Zlib);

        /**
         * @brief 设置字典
         * @param dictionary 字典数据
         */
        void SetDictionary(const std::vector<uint8_t>& dictionary);

        /**
         * @brief 获取压缩参数
         */
        void GetParams(CompressionLevel& level, CompressionStrategy& strategy) const;

        /**
         * @brief 设置压缩参数
         */
        void SetParams(CompressionLevel level, CompressionStrategy strategy);

    protected:
        void Cleanup() override;

    private:
        CompressionLevel level_;
        DataFormat format_;
        CompressionStrategy strategy_;
        bool finished_;

        /**
         * @brief 内部压缩实现
         */
        std::vector<uint8_t> CompressInternal(const uint8_t* input, size_t input_size, int flush_mode);
    };

} // namespace ZlibCpp 