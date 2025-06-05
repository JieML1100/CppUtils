#pragma once
#include <vector>
#include <memory>
#include "ZlibException.h"

// 引入原始zlib头文件
extern "C" {
#include "../zlib.h"
}

namespace ZlibCpp {

    /**
     * @brief 压缩级别枚举
     */
    enum class CompressionLevel {
        NoCompression = Z_NO_COMPRESSION,      // 不压缩
        BestSpeed = Z_BEST_SPEED,              // 最快速度
        BestCompression = Z_BEST_COMPRESSION,   // 最佳压缩
        DefaultCompression = Z_DEFAULT_COMPRESSION // 默认压缩
    };

    /**
     * @brief 压缩策略枚举
     */
    enum class CompressionStrategy {
        Default = Z_DEFAULT_STRATEGY,   // 默认策略
        Filtered = Z_FILTERED,          // 过滤策略
        HuffmanOnly = Z_HUFFMAN_ONLY,   // 仅Huffman编码
        Rle = Z_RLE,                    // RLE策略
        Fixed = Z_FIXED                 // 固定策略
    };

    /**
     * @brief 数据格式枚举
     */
    enum class DataFormat {
        Zlib = 0,        // 标准zlib格式
        Gzip = 16,       // Gzip格式
        Raw = -1         // 原始deflate格式
    };

    /**
     * @brief zlib操作的基础抽象类
     */
    class ZlibBase {
    public:
        ZlibBase();
        virtual ~ZlibBase();

        // 禁用拷贝构造和赋值
        ZlibBase(const ZlibBase&) = delete;
        ZlibBase& operator=(const ZlibBase&) = delete;

        // 启用移动构造和赋值
        ZlibBase(ZlibBase&& other) noexcept;
        ZlibBase& operator=(ZlibBase&& other) noexcept;

        /**
         * @brief 重置流状态
         */
        virtual void Reset() = 0;

        /**
         * @brief 获取总输入字节数
         */
        size_t GetTotalInput() const;

        /**
         * @brief 获取总输出字节数
         */
        size_t GetTotalOutput() const;

        /**
         * @brief 获取Adler32校验和
         */
        uint32_t GetAdler32() const;

        /**
         * @brief 检查流是否已初始化
         */
        bool IsInitialized() const;

    protected:
        z_stream stream_;
        bool initialized_;

        /**
         * @brief 检查zlib返回码并抛出相应异常
         */
        void CheckResult(int result, const std::string& operation) const;

        /**
         * @brief 初始化内存分配函数
         */
        void InitializeAllocators();

                /**         * @brief 清理资源         */        virtual void Cleanup() {}

    private:
        static voidpf AllocFunction(voidpf opaque, uInt items, uInt size);
        static void FreeFunction(voidpf opaque, voidpf address);
    };

} // namespace ZlibCpp 