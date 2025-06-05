#include "zstream.hpp"
#include <zlib.h>
#include <stdexcept>

namespace zlib_cpp {

// ZStreamImpl 是对z_stream的封装
struct ZStreamImpl {
    z_stream stream;
    
    ZStreamImpl() {
        // 初始化为0
        memset(&stream, 0, sizeof(z_stream));
    }
};

// 默认的内存分配和释放函数
static void* defaultAllocFunc(void* opaque, size_t items, size_t size) {
    (void)opaque; // 不使用opaque参数
    return malloc(items * size);
}

static void defaultFreeFunc(void* opaque, void* address) {
    (void)opaque; // 不使用opaque参数
    free(address);
}

// 包装用户提供的内存分配和释放函数
static voidpf zlibAllocWrapper(voidpf opaque, uInt items, uInt size) {
    ZStream* stream = static_cast<ZStream*>(opaque);
    return stream->_allocFunc(stream->_allocOpaque, items, size);
}

static void zlibFreeWrapper(voidpf opaque, voidpf address) {
    ZStream* stream = static_cast<ZStream*>(opaque);
    stream->_freeFunc(stream->_allocOpaque, address);
}

ZStream::ZStream() 
    : _impl(std::make_unique<ZStreamImpl>()),
      _allocFunc(defaultAllocFunc),
      _freeFunc(defaultFreeFunc) {
    _impl->stream.zalloc = Z_NULL;
    _impl->stream.zfree = Z_NULL;
    _impl->stream.opaque = Z_NULL;
}

ZStream::ZStream(AllocFunc allocFunc, FreeFunc freeFunc, void* opaque)
    : _impl(std::make_unique<ZStreamImpl>()),
      _allocFunc(std::move(allocFunc)),
      _freeFunc(std::move(freeFunc)),
      _allocOpaque(opaque) {
    _impl->stream.zalloc = zlibAllocWrapper;
    _impl->stream.zfree = zlibFreeWrapper;
    _impl->stream.opaque = this;
}

ZStream::~ZStream() {
    if (_isInitialized) {
        end();
    }
}

ZStream::ZStream(ZStream&& other) noexcept
    : _impl(std::move(other._impl)),
      _isInitialized(other._isInitialized),
      _allocFunc(std::move(other._allocFunc)),
      _freeFunc(std::move(other._freeFunc)),
      _allocOpaque(other._allocOpaque) {
    other._isInitialized = false;
    other._allocOpaque = nullptr;
    other._impl = std::make_unique<ZStreamImpl>();
}

ZStream& ZStream::operator=(ZStream&& other) noexcept {
    if (this != &other) {
        if (_isInitialized) {
            end();
        }
        
        _impl = std::move(other._impl);
        _isInitialized = other._isInitialized;
        _allocFunc = std::move(other._allocFunc);
        _freeFunc = std::move(other._freeFunc);
        _allocOpaque = other._allocOpaque;
        
        other._isInitialized = false;
        other._allocOpaque = nullptr;
        other._impl = std::make_unique<ZStreamImpl>();
    }
    return *this;
}

void ZStream::setInput(const uint8_t* data, size_t size) {
    _impl->stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(data));
    _impl->stream.avail_in = static_cast<uInt>(size);
}

void ZStream::setOutput(uint8_t* buffer, size_t size) {
    _impl->stream.next_out = reinterpret_cast<Bytef*>(buffer);
    _impl->stream.avail_out = static_cast<uInt>(size);
}

size_t ZStream::getAvailableInput() const {
    return static_cast<size_t>(_impl->stream.avail_in);
}

size_t ZStream::getAvailableOutput() const {
    return static_cast<size_t>(_impl->stream.avail_out);
}

size_t ZStream::getTotalInput() const {
    return static_cast<size_t>(_impl->stream.total_in);
}

size_t ZStream::getTotalOutput() const {
    return static_cast<size_t>(_impl->stream.total_out);
}

std::string ZStream::getLastErrorMessage() const {
    return _impl->stream.msg ? _impl->stream.msg : "";
}

DataType ZStream::getDataType() const {
    switch (_impl->stream.data_type) {
        case Z_BINARY: return DataType::BINARY;
        case Z_TEXT: return DataType::TEXT;
        default: return DataType::UNKNOWN;
    }
}

void ZStream::setDataType(DataType type) {
    switch (type) {
        case DataType::BINARY:
            _impl->stream.data_type = Z_BINARY;
            break;
        case DataType::TEXT:
            _impl->stream.data_type = Z_TEXT;
            break;
        case DataType::UNKNOWN:
            _impl->stream.data_type = Z_UNKNOWN;
            break;
    }
}

uint32_t ZStream::getAdler() const {
    return static_cast<uint32_t>(_impl->stream.adler);
}

void ZStream::setAdler(uint32_t adler) {
    _impl->stream.adler = adler;
}

} // namespace zlib_cpp
