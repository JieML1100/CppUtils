#pragma once
#include <stdexcept>
#include <string>

namespace ZlibCpp {

    /**
     * @brief zlib相关异常的基类
     */
    class ZlibException : public std::runtime_error {
    public:
        explicit ZlibException(const std::string& message, int error_code = 0)
            : std::runtime_error(message), error_code_(error_code) {}

        int GetErrorCode() const noexcept { return error_code_; }

    private:
        int error_code_;
    };

    /**
     * @brief 内存不足异常
     */
    class ZlibMemoryException : public ZlibException {
    public:
        explicit ZlibMemoryException(const std::string& message = "内存不足")
            : ZlibException(message, -4) {}
    };

    /**
     * @brief 数据错误异常
     */
    class ZlibDataException : public ZlibException {
    public:
        explicit ZlibDataException(const std::string& message = "数据错误")
            : ZlibException(message, -3) {}
    };

    /**
     * @brief 流错误异常
     */
    class ZlibStreamException : public ZlibException {
    public:
        explicit ZlibStreamException(const std::string& message = "流错误")
            : ZlibException(message, -2) {}
    };

    /**
     * @brief 缓冲区错误异常
     */
    class ZlibBufferException : public ZlibException {
    public:
        explicit ZlibBufferException(const std::string& message = "缓冲区错误")
            : ZlibException(message, -5) {}
    };

    /**
     * @brief 版本错误异常
     */
    class ZlibVersionException : public ZlibException {
    public:
        explicit ZlibVersionException(const std::string& message = "版本错误")
            : ZlibException(message, -6) {}
    };

} // namespace ZlibCpp 