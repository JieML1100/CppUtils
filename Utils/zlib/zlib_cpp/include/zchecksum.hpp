#ifndef ZCHECKSUM_HPP
#define ZCHECKSUM_HPP

#include <cstdint>
#include <vector>

namespace zlib_cpp {

/**
 * @brief Adler-32校验和计算类
 */
class ZAdler32 {
private:
    uint32_t _adler = 1; // 初始Adler-32值为1

public:
    /**
     * @brief 默认构造函数
     */
    ZAdler32() = default;
    
    /**
     * @brief 带初始值的构造函数
     * 
     * @param initial 初始Adler-32值
     */
    explicit ZAdler32(uint32_t initial);
    
    /**
     * @brief 更新校验和
     * 
     * @param data 数据指针
     * @param length 数据长度
     * @return 更新后的校验和
     */
    uint32_t update(const uint8_t* data, size_t length);
    
    /**
     * @brief 更新校验和
     * 
     * @param data 数据向量
     * @return 更新后的校验和
     */
    uint32_t update(const std::vector<uint8_t>& data);
    
    /**
     * @brief 获取当前校验和
     * 
     * @return 当前校验和
     */
    uint32_t getValue() const;
    
    /**
     * @brief 重置校验和
     */
    void reset();
    
    /**
     * @brief 静态计算Adler-32校验和
     * 
     * @param data 数据指针
     * @param length 数据长度
     * @param initial 初始Adler-32值，默认为1
     * @return 计算得到的校验和
     */
    static uint32_t calculate(const uint8_t* data, size_t length, uint32_t initial = 1);
    
    /**
     * @brief 静态计算Adler-32校验和
     * 
     * @param data 数据向量
     * @param initial 初始Adler-32值，默认为1
     * @return 计算得到的校验和
     */
    static uint32_t calculate(const std::vector<uint8_t>& data, uint32_t initial = 1);
    
    /**
     * @brief 组合两个Adler-32校验和
     * 
     * @param adler1 第一个校验和
     * @param adler2 第二个校验和
     * @param len2 第二个数据的长度
     * @return 组合后的校验和
     */
    static uint32_t combine(uint32_t adler1, uint32_t adler2, size_t len2);
};

/**
 * @brief CRC-32校验和计算类
 */
class ZCRC32 {
private:
    uint32_t _crc = 0; // 初始CRC-32值为0

public:
    /**
     * @brief 默认构造函数
     */
    ZCRC32() = default;
    
    /**
     * @brief 带初始值的构造函数
     * 
     * @param initial 初始CRC-32值
     */
    explicit ZCRC32(uint32_t initial);
    
    /**
     * @brief 更新校验和
     * 
     * @param data 数据指针
     * @param length 数据长度
     * @return 更新后的校验和
     */
    uint32_t update(const uint8_t* data, size_t length);
    
    /**
     * @brief 更新校验和
     * 
     * @param data 数据向量
     * @return 更新后的校验和
     */
    uint32_t update(const std::vector<uint8_t>& data);
    
    /**
     * @brief 获取当前校验和
     * 
     * @return 当前校验和
     */
    uint32_t getValue() const;
    
    /**
     * @brief 重置校验和
     */
    void reset();
    
    /**
     * @brief 静态计算CRC-32校验和
     * 
     * @param data 数据指针
     * @param length 数据长度
     * @param initial 初始CRC-32值，默认为0
     * @return 计算得到的校验和
     */
    static uint32_t calculate(const uint8_t* data, size_t length, uint32_t initial = 0);
    
    /**
     * @brief 静态计算CRC-32校验和
     * 
     * @param data 数据向量
     * @param initial 初始CRC-32值，默认为0
     * @return 计算得到的校验和
     */
    static uint32_t calculate(const std::vector<uint8_t>& data, uint32_t initial = 0);
    
    /**
     * @brief 组合两个CRC-32校验和
     * 
     * @param crc1 第一个校验和
     * @param crc2 第二个校验和
     * @param len2 第二个数据的长度
     * @return 组合后的校验和
     */
    static uint32_t combine(uint32_t crc1, uint32_t crc2, size_t len2);
    
    /**
     * @brief 获取CRC-32表
     * 
     * @return CRC-32表
     */
    static const uint32_t* getTable();
};

} // namespace zlib_cpp

#endif // ZCHECKSUM_HPP
