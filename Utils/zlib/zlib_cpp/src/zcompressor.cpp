#include "zcompressor.hpp"
#include <zlib.h>
#include <stdexcept>
#include <cstring>

namespace zlib_cpp {

// GZipHeader类的实现
void GZipHeader::setText(bool isText) {
    _text = isText;
}

bool GZipHeader::isText() const {
    return _text;
}

void GZipHeader::setTime(uint32_t time) {
    _time = time;
}

uint32_t GZipHeader::getTime() const {
    return _time;
}

void GZipHeader::setExtraFlags(int flags) {
    _extraFlags = flags;
}

int GZipHeader::getExtraFlags() const {
    return _extraFlags;
}

void GZipHeader::setOS(int os) {
    _os = os;
}

int GZipHeader::getOS() const {
    return _os;
}

void GZipHeader::setExtra(const std::vector<uint8_t>& extra) {
    _extra = extra;
}

const std::vector<uint8_t>& GZipHeader::getExtra() const {
    return _extra;
}

void GZipHeader::setName(const std::string& name) {
    _name = name;
}

const std::string& GZipHeader::getName() const {
    return _name;
}

void GZipHeader::setComment(const std::string& comment) {
    _comment = comment;
}

const std::string& GZipHeader::getComment() const {
    return _comment;
}

void GZipHeader::setHCRC(bool hcrc) {
    _hcrc = hcrc;
}

bool GZipHeader::getHCRC() const {
    return _hcrc;
}

// ZCompressor类的实现
ZCompressor::ZCompressor()
    : ZStream() {
    init();
}

ZCompressor::ZCompressor(CompressionLevel level, CompressionStrategy strategy)
    : ZStream(),
      _level(level),
      _strategy(strategy) {
    init();
}

ZCompressor::ZCompressor(CompressionLevel level, int windowBits, int memLevel, CompressionStrategy strategy)
    : ZStream(),
      _level(level),
      _strategy(strategy),
      _windowBits(windowBits),
      _memLevel(memLevel) {
    init();
}

ZCompressor::~ZCompressor() {
    if (_isInitialized) {
        end();
    }
}

std::vector<uint8_t> ZCompressor::compress(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return {};
    }
    
    size_t bufferSize = getBound(data.size());
    std::vector<uint8_t> result(bufferSize);
    
    setInput(data.data(), data.size());
    setOutput(result.data(), result.size());
    
    StatusCode status = deflate(FlushMode::FINISH);
    if (status != StatusCode::STREAM_END) {
        throw ZException(status, "Failed to compress data: " + getLastErrorMessage());
    }
    
    // 调整结果大小
    result.resize(getTotalOutput());
    return result;
}

StatusCode ZCompressor::deflate(FlushMode flush) {
    if (!_isInitialized) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = ::deflate(&_impl->stream, static_cast<int>(flush));
    return static_cast<StatusCode>(result);
}

StatusCode ZCompressor::setDictionary(const std::vector<uint8_t>& dictionary) {
    if (!_isInitialized || dictionary.empty()) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = deflateSetDictionary(&_impl->stream, 
                                     reinterpret_cast<const Bytef*>(dictionary.data()),
                                     static_cast<uInt>(dictionary.size()));
    return static_cast<StatusCode>(result);
}

std::vector<uint8_t> ZCompressor::getDictionary() {
    if (!_isInitialized) {
        return {};
    }
    
    std::vector<uint8_t> dictionary(32768); // 最大字典大小
    uInt dictLength = static_cast<uInt>(dictionary.size());
    
    int result = deflateGetDictionary(&_impl->stream, 
                                     reinterpret_cast<Bytef*>(dictionary.data()),
                                     &dictLength);
    
    if (result == Z_OK) {
        dictionary.resize(dictLength);
        return dictionary;
    }
    
    return {};
}

StatusCode ZCompressor::reset() {
    if (!_isInitialized) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = deflateReset(&_impl->stream);
    return static_cast<StatusCode>(result);
}

StatusCode ZCompressor::setParams(CompressionLevel level, CompressionStrategy strategy) {
    if (!_isInitialized) {
        return StatusCode::STREAM_ERROR;
    }
    
    _level = level;
    _strategy = strategy;
    
    int result = deflateParams(&_impl->stream, 
                              static_cast<int>(_level), 
                              static_cast<int>(_strategy));
    return static_cast<StatusCode>(result);
}

StatusCode ZCompressor::tune(int goodLength, int maxLazy, int niceLength, int maxChain) {
    if (!_isInitialized) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = deflateTune(&_impl->stream, goodLength, maxLazy, niceLength, maxChain);
    return static_cast<StatusCode>(result);
}

StatusCode ZCompressor::setHeader(const GZipHeader& header) {
    if (!_isInitialized || !_isGzip) {
        return StatusCode::STREAM_ERROR;
    }
    
    gz_header gzHeader;
    memset(&gzHeader, 0, sizeof(gz_header));
    
    gzHeader.text = header.isText() ? 1 : 0;
    gzHeader.time = header.getTime();
    gzHeader.xflags = header.getExtraFlags();
    gzHeader.os = header.getOS();
    
    const auto& extra = header.getExtra();
    if (!extra.empty()) {
        gzHeader.extra = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(extra.data()));
        gzHeader.extra_len = static_cast<uInt>(extra.size());
        gzHeader.extra_max = static_cast<uInt>(extra.size());
    }
    
    const auto& name = header.getName();
    if (!name.empty()) {
        gzHeader.name = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(name.c_str()));
        gzHeader.name_max = static_cast<uInt>(name.size() + 1); // 包含null终止符
    }
    
    const auto& comment = header.getComment();
    if (!comment.empty()) {
        gzHeader.comment = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(comment.c_str()));
        gzHeader.comm_max = static_cast<uInt>(comment.size() + 1); // 包含null终止符
    }
    
    gzHeader.hcrc = header.getHCRC() ? 1 : 0;
    
    int result = deflateSetHeader(&_impl->stream, &gzHeader);
    return static_cast<StatusCode>(result);
}

size_t ZCompressor::getBound(size_t sourceLen) {
    return static_cast<size_t>(deflateBound(nullptr, static_cast<uLong>(sourceLen)));
}

void ZCompressor::enableGZip(bool enable) {
    _isGzip = enable;
    if (_isInitialized) {
        // 如果已初始化，需要重新初始化以应用更改
        end();
        init();
    }
}

void ZCompressor::setLevel(CompressionLevel level) {
    _level = level;
    if (_isInitialized) {
        setParams(_level, _strategy);
    }
}

CompressionLevel ZCompressor::getLevel() const {
    return _level;
}

void ZCompressor::setStrategy(CompressionStrategy strategy) {
    _strategy = strategy;
    if (_isInitialized) {
        setParams(_level, _strategy);
    }
}

CompressionStrategy ZCompressor::getStrategy() const {
    return _strategy;
}

StatusCode ZCompressor::init() {
    if (_isInitialized) {
        end();
    }
    
    int windowBits = _windowBits;
    if (_isGzip) {
        // 为GZip格式添加16
        windowBits += 16;
    }
    
    int result = deflateInit2(&_impl->stream, 
                             static_cast<int>(_level),
                             Z_DEFLATED,
                             windowBits,
                             _memLevel,
                             static_cast<int>(_strategy));
    
    if (result == Z_OK) {
        _isInitialized = true;
    }
    
    return static_cast<StatusCode>(result);
}

StatusCode ZCompressor::end() {
    if (!_isInitialized) {
        return StatusCode::OK;
    }
    
    int result = deflateEnd(&_impl->stream);
    if (result == Z_OK) {
        _isInitialized = false;
    }
    
    return static_cast<StatusCode>(result);
}

} // namespace zlib_cpp
