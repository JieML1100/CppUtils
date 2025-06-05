#ifndef ZSTREAM_HPP
#define ZSTREAM_HPP

#include "zlib_cpp.hpp"
#include <cstdint>
#include <vector>
#include <memory>
#include <functional>

namespace zlib_cpp {

// 前向声明
struct ZStreamImpl;

/**
 * @brief ZStream类是zlib流处理的核心类，对应于zlib的z_stream结构
 * 
 * 这个类被设计为ZCompressor和ZDecompressor的基类，
 * 提供基本的数据流管理和状态控制功能。
 */
class ZStream {
public:
    using AllocFunc = std::function<void*(void*, size_t, size_t)>;
    using FreeFunc = std::function<void(void*, void*)>;

protected:
    std::unique_ptr<ZStreamImpl> _impl;
    bool _isInitialized = false;
    
    // 自定义内存分配器和释放器(可选)
    AllocFunc _allocFunc;
    FreeFunc _freeFunc;
    void* _allocOpaque = nullptr;

public:
    /**
     * @brief 构造函数
     */
    ZStream();
    
    /**
     * @brief 带自定义内存分配函数的构造函数
     * 
     * @param allocFunc 内存分配函数
     * @param freeFunc 内存释放函数
     * @param opaque 传递给分配/释放函数的透明指针
     */
    ZStream(AllocFunc allocFunc, FreeFunc freeFunc, void* opaque);
    
    /**
     * @brief 析构函数
     */
    virtual ~ZStream();

    /**
     * @brief 禁止拷贝构造
     */
    ZStream(const ZStream&) = delete;
    
    /**
     * @brief 禁止赋值操作
     */
    ZStream& operator=(const ZStream&) = delete;
    
    /**
     * @brief 移动构造函数
     */
    ZStream(ZStream&& other) noexcept;
    
    /**
     * @brief 移动赋值操作符
     */
    ZStream& operator=(ZStream&& other) noexcept;

    /**
     * @brief 设置输入数据
     * 
     * @param data 输入数据的指针
     * @param size 输入数据的大小
     */
    void setInput(const uint8_t* data, size_t size);
    
    /**
     * @brief 设置输出缓冲区
     * 
     * @param buffer 输出缓冲区指针
     * @param size 输出缓冲区大小
     */
    void setOutput(uint8_t* buffer, size_t size);
    
    /**
     * @brief 获取可用的输入数据大小
     * 
     * @return 可用的输入数据大小
     */
    size_t getAvailableInput() const;
    
    /**
     * @brief 获取可用的输出缓冲区大小
     * 
     * @return 可用的输出缓冲区大小
     */
    size_t getAvailableOutput() const;
    
    /**
     * @brief 获取总输入数据大小
     * 
     * @return 总输入数据大小
     */
    size_t getTotalInput() const;
    
    /**
     * @brief 获取总输出数据大小
     * 
     * @return 总输出数据大小
     */
    size_t getTotalOutput() const;
    
    /**
     * @brief 重置流状态
     * 
     * @return 状态码
     */
    virtual StatusCode reset() = 0;
    
    /**
     * @brief 获取最后的错误消息
     * 
     * @return 错误消息
     */
    std::string getLastErrorMessage() const;
    
    /**
     * @brief 获取数据类型
     * 
     * @return 数据类型
     */
    DataType getDataType() const;
    
    /**
     * @brief 设置数据类型
     * 
     * @param type 数据类型
     */
    void setDataType(DataType type);
    
    /**
     * @brief 获取Adler-32校验和
     * 
     * @return Adler-32校验和
     */
    uint32_t getAdler() const;
    
    /**
     * @brief 设置Adler-32校验和
     * 
     * @param adler Adler-32校验和
     */
    void setAdler(uint32_t adler);

protected:
    /**
     * @brief 初始化流
     * 
     * @return 状态码
     */
    virtual StatusCode init() = 0;
    
    /**
     * @brief 清理流资源
     * 
     * @return 状态码
     */
    virtual StatusCode end() = 0;
};

} // namespace zlib_cpp

#endif // ZSTREAM_HPP
