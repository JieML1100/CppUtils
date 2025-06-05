#include "ZlibBase.h"
#include <cstring>
#include <cstdlib>

namespace ZlibCpp {

    ZlibBase::ZlibBase() : initialized_(false) {
        std::memset(&stream_, 0, sizeof(z_stream));
        InitializeAllocators();
    }

    ZlibBase::~ZlibBase() {
        // 不在基类析构函数中调用纯虚函数
        // 派生类应该在自己的析构函数中调用Cleanup()
    }

    ZlibBase::ZlibBase(ZlibBase&& other) noexcept
        : stream_(other.stream_), initialized_(other.initialized_) {
        // 重置other对象的状态
        std::memset(&other.stream_, 0, sizeof(z_stream));
        other.initialized_ = false;
    }

        ZlibBase& ZlibBase::operator=(ZlibBase&& other) noexcept {       
            if (this != &other) {            // 清理当前对象           
                if (initialized_) {               
                    Cleanup();         
                }            // 移动other的状态  
                stream_ = other.stream_;        
                initialized_ = other.initialized_;            // 重置other对象的状态   
                std::memset(&other.stream_, 0, sizeof(z_stream));  
                other.initialized_ = false;      
            }        
            return *this;   
        }

    size_t ZlibBase::GetTotalInput() const {
        return stream_.total_in;
    }

    size_t ZlibBase::GetTotalOutput() const {
        return stream_.total_out;
    }

    uint32_t ZlibBase::GetAdler32() const {
        return stream_.adler;
    }

    bool ZlibBase::IsInitialized() const {
        return initialized_;
    }

    void ZlibBase::CheckResult(int result, const std::string& operation) const {
        switch (result) {
        case Z_OK:
        case Z_STREAM_END:
        case Z_NEED_DICT:
            // 这些都是正常或可处理的状态
            return;

        case Z_ERRNO:
            throw ZlibException("系统错误: " + operation, result);

        case Z_STREAM_ERROR:
            throw ZlibStreamException("流错误: " + operation);

        case Z_DATA_ERROR:
            throw ZlibDataException("数据错误: " + operation);

        case Z_MEM_ERROR:
            throw ZlibMemoryException("内存不足: " + operation);

        case Z_BUF_ERROR:
            throw ZlibBufferException("缓冲区错误: " + operation);

        case Z_VERSION_ERROR:
            throw ZlibVersionException("版本错误: " + operation);

        default:
            throw ZlibException("未知错误: " + operation + " (错误码: " + std::to_string(result) + ")", result);
        }
    }

    void ZlibBase::InitializeAllocators() {
        stream_.zalloc = AllocFunction;
        stream_.zfree = FreeFunction;
        stream_.opaque = nullptr;
    }

    voidpf ZlibBase::AllocFunction(voidpf opaque, uInt items, uInt size) {
        (void)opaque; // 未使用的参数
        return std::malloc(items * size);
    }

    void ZlibBase::FreeFunction(voidpf opaque, voidpf address) {
        (void)opaque; // 未使用的参数
        std::free(address);
    }

} // namespace ZlibCpp 