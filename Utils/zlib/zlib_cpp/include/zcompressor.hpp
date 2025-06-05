#ifndef ZCOMPRESSOR_HPP
#define ZCOMPRESSOR_HPP

#include "zstream.hpp"

namespace zlib_cpp {

/**
 * @brief GZip头部信息类
 */
class GZipHeader {
private:
    bool _text = false;
    uint32_t _time = 0;
    int _extraFlags = 0;
    int _os = 0;
    std::vector<uint8_t> _extra;
    std::string _name;
    std::string _comment;
    bool _hcrc = false;
    
public:
    /**
     * @brief 默认构造函数
     */
    GZipHeader() = default;
    
    /**
     * @brief 设置文本标志
     * 
     * @param isText 文本标志
     */
    void setText(bool isText);
    
    /**
     * @brief 获取文本标志
     * 
     * @return 文本标志
     */
    bool isText() const;
    
    /**
     * @brief 设置时间戳
     * 
     * @param time 时间戳
     */
    void setTime(uint32_t time);
    
    /**
     * @brief 获取时间戳
     * 
     * @return 时间戳
     */
    uint32_t getTime() const;
    
    /**
     * @brief 设置额外标志
     * 
     * @param flags 额外标志
     */
    void setExtraFlags(int flags);
    
    /**
     * @brief 获取额外标志
     * 
     * @return 额外标志
     */
    int getExtraFlags() const;
    
    /**
     * @brief 设置操作系统标识
     * 
     * @param os 操作系统标识
     */
    void setOS(int os);
    
    /**
     * @brief 获取操作系统标识
     * 
     * @return 操作系统标识
     */
    int getOS() const;
    
    /**
     * @brief 设置额外数据
     * 
     * @param extra 额外数据
     */
    void setExtra(const std::vector<uint8_t>& extra);
    
    /**
     * @brief 获取额外数据
     * 
     * @return 额外数据
     */
    const std::vector<uint8_t>& getExtra() const;
    
    /**
     * @brief 设置文件名
     * 
     * @param name 文件名
     */
    void setName(const std::string& name);
    
    /**
     * @brief 获取文件名
     * 
     * @return 文件名
     */
    const std::string& getName() const;
    
    /**
     * @brief 设置注释
     * 
     * @param comment 注释
     */
    void setComment(const std::string& comment);
    
    /**
     * @brief 获取注释
     * 
     * @return 注释
     */
    const std::string& getComment() const;
    
    /**
     * @brief 设置HCRC标志
     * 
     * @param hcrc HCRC标志
     */
    void setHCRC(bool hcrc);
    
    /**
     * @brief 获取HCRC标志
     * 
     * @return HCRC标志
     */
    bool getHCRC() const;
};

/**
 * @brief 压缩器类，处理数据压缩功能
 */
class ZCompressor : public ZStream {
private:
    CompressionLevel _level = CompressionLevel::DEFAULT_COMPRESSION;
    CompressionStrategy _strategy = CompressionStrategy::DEFAULT_STRATEGY;
    int _windowBits = 15; // 默认windowBits
    int _memLevel = 8;    // 默认memLevel
    std::unique_ptr<GZipHeader> _gzipHeader;
    bool _isGzip = false;

public:
    /**
     * @brief 默认构造函数
     */
    ZCompressor();
    
    /**
     * @brief 带参数的构造函数
     * 
     * @param level 压缩级别
     * @param strategy 压缩策略
     */
    ZCompressor(CompressionLevel level, CompressionStrategy strategy = CompressionStrategy::DEFAULT_STRATEGY);
    
    /**
     * @brief 构造函数，支持自定义窗口大小和内存级别
     * 
     * @param level 压缩级别
     * @param windowBits 窗口大小，负值表示不使用zlib头/尾
     * @param memLevel 内存使用级别
     * @param strategy 压缩策略
     */
    ZCompressor(CompressionLevel level, int windowBits, int memLevel, 
                CompressionStrategy strategy = CompressionStrategy::DEFAULT_STRATEGY);
    
    /**
     * @brief 析构函数
     */
    virtual ~ZCompressor();
    
    /**
     * @brief 使用默认参数压缩数据
     * 
     * @param data 输入数据
     * @return 压缩后的数据
     */
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    
    /**
     * @brief 压缩数据块
     * 
     * @param flush 刷新模式
     * @return 处理状态
     */
    StatusCode deflate(FlushMode flush = FlushMode::NO_FLUSH);
    
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
     * @brief 重置压缩器
     * 
     * @return 状态码
     */
    virtual StatusCode reset() override;
    
    /**
     * @brief 更改压缩参数
     * 
     * @param level 压缩级别
     * @param strategy 压缩策略
     * @return 状态码
     */
    StatusCode setParams(CompressionLevel level, CompressionStrategy strategy);
    
    /**
     * @brief 微调压缩参数
     * 
     * @param goodLength 良好长度
     * @param maxLazy 最大延迟
     * @param niceLength 理想长度
     * @param maxChain 最大链长
     * @return 状态码
     */
    StatusCode tune(int goodLength, int maxLazy, int niceLength, int maxChain);
    
    /**
     * @brief 设置GZip头信息
     * 
     * @param header GZip头信息
     * @return 状态码
     */
    StatusCode setHeader(const GZipHeader& header);
    
    /**
     * @brief 获取压缩后数据大小的边界估计
     * 
     * @param sourceLen 源数据长度
     * @return 压缩后的最大长度
     */
    static size_t getBound(size_t sourceLen);
    
    /**
     * @brief 启用GZip模式
     * 
     * @param enable 是否启用
     */
    void enableGZip(bool enable = true);
    
    /**
     * @brief 设置压缩级别
     * 
     * @param level 压缩级别
     */
    void setLevel(CompressionLevel level);
    
    /**
     * @brief 获取压缩级别
     * 
     * @return 压缩级别
     */
    CompressionLevel getLevel() const;
    
    /**
     * @brief 设置压缩策略
     * 
     * @param strategy 压缩策略
     */
    void setStrategy(CompressionStrategy strategy);
    
    /**
     * @brief 获取压缩策略
     * 
     * @return 压缩策略
     */
    CompressionStrategy getStrategy() const;

protected:
    /**
     * @brief 初始化压缩器
     * 
     * @return 状态码
     */
    virtual StatusCode init() override;
    
    /**
     * @brief 清理压缩器资源
     * 
     * @return 状态码
     */
    virtual StatusCode end() override;
};

} // namespace zlib_cpp

#endif // ZCOMPRESSOR_HPP
