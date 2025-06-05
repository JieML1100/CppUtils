#pragma once
#include "ZlibBase.h"
#include <vector>
#include <string>

namespace ZlibCpp {

    /**
     * @brief zlib解压缩器类
     */
    class ZlibDecompressor : public ZlibBase {
    public:
        /**
         * @brief 构造函数
         * @param format 数据格式
         */
        explicit ZlibDecompressor(DataFormat format = DataFormat::Zlib);

        /**
         * @brief 析构函数
         */
        ~ZlibDecompressor() override;

        /**
         * @brief 初始化解压缩器
         * @param format 数据格式
         */
        void Initialize(DataFormat format = DataFormat::Zlib);

        /**
         * @brief 重置解压缩器状态
         */
        void Reset() override;

        /**
         * @brief 解压缩数据
         * @param input 输入数据
         * @param flush 刷新模式
         * @return 解压缩后的数据
         */
        std::vector<uint8_t> Decompress(const std::vector<uint8_t>& input, bool flush = false);

        /**
         * @brief 解压缩数据
         * @param input 输入数据指针
         * @param input_size 输入数据大小
         * @param flush 刷新模式
         * @return 解压缩后的数据
         */
        std::vector<uint8_t> Decompress(const uint8_t* input, size_t input_size, bool flush = false);

        /**
         * @brief 完成解压缩
         * @return 剩余的解压缩数据
         */
        std::vector<uint8_t> Finish();

        /**
         * @brief 一次性解压缩数据
         * @param input 输入数据
         * @param format 数据格式
         * @return 解压缩后的数据
         */
        static std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& input,
                                                  DataFormat format = DataFormat::Zlib);

        /**
         * @brief 一次性解压缩到字符串
         * @param input 输入数据
         * @param format 数据格式
         * @return 解压缩后的字符串
         */
        static std::string DecompressString(const std::vector<uint8_t>& input,
                                           DataFormat format = DataFormat::Zlib);

        /**
         * @brief 设置字典
         * @param dictionary 字典数据
         */
        void SetDictionary(const std::vector<uint8_t>& dictionary);

        /**
         * @brief 检查是否完成解压缩
         */
        bool IsFinished() const;

        /**
         * @brief 获取数据格式
         */
        DataFormat GetFormat() const;

    protected:
        void Cleanup() override;

    private:
        DataFormat format_;
        bool finished_;

        /**
         * @brief 内部解压缩实现
         */
        std::vector<uint8_t> DecompressInternal(const uint8_t* input, size_t input_size, int flush_mode);
    };

} // namespace ZlibCpp 