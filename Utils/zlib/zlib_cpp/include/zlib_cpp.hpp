#ifndef ZLIB_CPP_HPP
#define ZLIB_CPP_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace zlib_cpp {

// 前向声明
class ZStream;
class ZCompressor;
class ZDecompressor;
class ZError;

// 常量定义 (对应zlib中的定义)
enum class FlushMode {
    NO_FLUSH = 0,
    PARTIAL_FLUSH = 1,
    SYNC_FLUSH = 2,
    FULL_FLUSH = 3,
    FINISH = 4,
    BLOCK = 5,
    TREES = 6
};

enum class CompressionLevel {
    NO_COMPRESSION = 0,
    BEST_SPEED = 1,
    BEST_COMPRESSION = 9,
    DEFAULT_COMPRESSION = -1
};

enum class CompressionStrategy {
    DEFAULT_STRATEGY = 0,
    FILTERED = 1,
    HUFFMAN_ONLY = 2,
    RLE = 3,
    FIXED = 4
};

enum class StatusCode {
    OK = 0,
    STREAM_END = 1,
    NEED_DICT = 2,
    ERRNO = -1,
    STREAM_ERROR = -2,
    DATA_ERROR = -3,
    MEM_ERROR = -4,
    BUF_ERROR = -5,
    VERSION_ERROR = -6
};

enum class DataType {
    BINARY = 0,
    TEXT = 1,
    UNKNOWN = 2
};

// 版本信息
class Version {
public:
    static std::string getVersionString();
    static int getMajor();
    static int getMinor();
    static int getRevision();
    static int getSubRevision();
};

// 异常类
class ZException : public std::exception {
private:
    StatusCode _code;
    std::string _message;

public:
    ZException(StatusCode code, const std::string& message);
    virtual ~ZException() noexcept = default;

    virtual const char* what() const noexcept override;
    StatusCode getCode() const noexcept;
};

// 简便函数
std::vector<uint8_t> compress(const std::vector<uint8_t>& data, 
                             CompressionLevel level = CompressionLevel::DEFAULT_COMPRESSION);
std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);

uint32_t adler32(const std::vector<uint8_t>& data);
uint32_t crc32(const std::vector<uint8_t>& data);

// 辅助函数
uint32_t adler32Combine(uint32_t adler1, uint32_t adler2, size_t len2);
uint32_t crc32Combine(uint32_t crc1, uint32_t crc2, size_t len2);

} // namespace zlib_cpp

#endif // ZLIB_CPP_HPP
