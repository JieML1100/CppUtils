#ifndef ZDECOMPRESSOR_HPP
#define ZDECOMPRESSOR_HPP

#include "zstream.hpp"
#include "zcompressor.hpp"

namespace zlib_cpp {

/**
 * @brief 解压缩器类，处理数据解压功能
 */
class ZDecompressor : public ZStream {
private:
    int _windowBits = 15; // 默认windowBits
    bool _expectGzip = false;

public:
    /**
     * @brief 默认构造函数
     */
    ZDecompressor();
    
    /**
     * @brief 带参数的构造函数
     * 
     * @param windowBits 窗口大小，负值表示不使用zlib头/尾
     */
    ZDecompressor(int windowBits);
    
    /**
     * @brief 析构函数
     */
    virtual ~ZDecompressor();
    
    /**
     * @brief 使用默认参数解压数据
     * 
     * @param data 压缩后的数据
     * @return 解压后的数据
     */
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);
    
    /**
     * @brief 解压数据块
     * 
     * @param flush 刷新模式
     * @return 处理状态
     */
    StatusCode inflate(FlushMode flush = FlushMode::NO_FLUSH);
    
    /**
     * @brief 设置字典
     * 
     * @param dictionary 字典数据
     * @return 状态码
     */
    StatusCode setDictionary(const std::vector<uint8_t>& dictionary);
    
    /**
     * @brief 获取字典
     * 
     * @return 字典数据
     */
    std::vector<uint8_t> getDictionary();
    
    /**
     * @brief 重置解压缩器
     * 
     * @return 状态码
     */
    virtual StatusCode reset() override;
    
    /**
     * @brief 与解压同步
     * 
     * @return 状态码
     */
    StatusCode sync();
    
    /**
     * @brief 获取同步点
     * 
     * @return 是否在同步点
     */
    bool syncPoint();
    
    /**
     * @brief 获取GZip头信息
     * 
     * @return GZip头信息
     */
    GZipHeader getHeader();
    
    /**
     * @brief 预设输入位
     * 
     * @param bits 位数
     * @param value 值
     * @return 状态码
     */
    StatusCode prime(int bits, int value);
    
    /**
     * @brief 获取标记位置
     * 
     * @return 标记位置
     */
    long getMark();
    
    /**
     * @brief 设置是否期望GZip格式
     * 
     * @param expectGzip 是否期望GZip格式
     */
    void setExpectGZip(bool expectGzip = true);
    
    /**
     * @brief 验证
     * 
     * @param check 是否检查
     * @return 状态码
     */
    StatusCode validate(bool check);
    
    /**
     * @brief 获取已使用的代码数
     * 
     * @return 已使用的代码数
     */
    size_t getCodesUsed();

protected:
    /**
     * @brief 初始化解压缩器
     * 
     * @return 状态码
     */
    virtual StatusCode init() override;
    
    /**
     * @brief 清理解压缩器资源
     * 
     * @return 状态码
     */
    virtual StatusCode end() override;
};

} // namespace zlib_cpp

#endif // ZDECOMPRESSOR_HPP
