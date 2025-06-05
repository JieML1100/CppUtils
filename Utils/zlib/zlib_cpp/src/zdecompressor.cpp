#include "zdecompressor.hpp"
#include <zlib.h>
#include <stdexcept>
#include <cstring>

namespace zlib_cpp {

ZDecompressor::ZDecompressor()
    : ZStream() {
    init();
}

ZDecompressor::ZDecompressor(int windowBits)
    : ZStream(),
      _windowBits(windowBits) {
    init();
}

ZDecompressor::~ZDecompressor() {
    if (_isInitialized) {
        end();
    }
}

std::vector<uint8_t> ZDecompressor::decompress(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return {};
    }
    
    // 预估解压后大小为压缩数据的4倍，如果不够会自动扩容
    size_t bufferSize = data.size() * 4;
    std::vector<uint8_t> result(bufferSize);
    
    setInput(data.data(), data.size());
    setOutput(result.data(), result.size());
    
    StatusCode status = inflate(FlushMode::FINISH);
    
    // 如果输出缓冲区不足，扩容并继续解压
    while (status == StatusCode::BUF_ERROR) {
        // 已解压的数据大小
        size_t decompressedSize = getTotalOutput();
        
        // 扩容结果缓冲区
        bufferSize *= 2;
        result.resize(bufferSize);
        
        // 设置新的输出缓冲区
        setOutput(result.data() + decompressedSize, result.size() - decompressedSize);
        
        // 继续解压
        status = inflate(FlushMode::FINISH);
    }
    
    if (status != StatusCode::STREAM_END) {
        throw ZException(status, "Failed to decompress data: " + getLastErrorMessage());
    }
    
    // 调整结果大小
    result.resize(getTotalOutput());
    return result;
}

StatusCode ZDecompressor::inflate(FlushMode flush) {
    if (!_isInitialized) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = ::inflate(&_impl->stream, static_cast<int>(flush));
    return static_cast<StatusCode>(result);
}

StatusCode ZDecompressor::setDictionary(const std::vector<uint8_t>& dictionary) {
    if (!_isInitialized || dictionary.empty()) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = inflateSetDictionary(&_impl->stream, 
                                     reinterpret_cast<const Bytef*>(dictionary.data()),
                                     static_cast<uInt>(dictionary.size()));
    return static_cast<StatusCode>(result);
}

std::vector<uint8_t> ZDecompressor::getDictionary() {
    if (!_isInitialized) {
        return {};
    }
    
    std::vector<uint8_t> dictionary(32768); // 最大字典大小
    uInt dictLength = static_cast<uInt>(dictionary.size());
    
    int result = inflateGetDictionary(&_impl->stream, 
                                     reinterpret_cast<Bytef*>(dictionary.data()),
                                     &dictLength);
    
    if (result == Z_OK) {
        dictionary.resize(dictLength);
        return dictionary;
    }
    
    return {};
}

StatusCode ZDecompressor::reset() {
    if (!_isInitialized) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = inflateReset(&_impl->stream);
    return static_cast<StatusCode>(result);
}

StatusCode ZDecompressor::sync() {
    if (!_isInitialized) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = inflateSync(&_impl->stream);
    return static_cast<StatusCode>(result);
}

bool ZDecompressor::syncPoint() {
    if (!_isInitialized) {
        return false;
    }
    
    return inflateSyncPoint(&_impl->stream) == 1;
}

GZipHeader ZDecompressor::getHeader() {
    if (!_isInitialized) {
        return {};
    }
    
    gz_header gzHeader;
    memset(&gzHeader, 0, sizeof(gz_header));
    
    int result = inflateGetHeader(&_impl->stream, &gzHeader);
    if (result != Z_OK) {
        return {};
    }
    
    GZipHeader header;
    
    if (gzHeader.done) {
        header.setText(gzHeader.text != 0);
        header.setTime(gzHeader.time);
        header.setExtraFlags(gzHeader.xflags);
        header.setOS(gzHeader.os);
        
        if (gzHeader.extra && gzHeader.extra_len > 0) {
            std::vector<uint8_t> extra(gzHeader.extra, gzHeader.extra + gzHeader.extra_len);
            header.setExtra(extra);
        }
        
        if (gzHeader.name) {
            header.setName(reinterpret_cast<char*>(gzHeader.name));
        }
        
        if (gzHeader.comment) {
            header.setComment(reinterpret_cast<char*>(gzHeader.comment));
        }
        
        header.setHCRC(gzHeader.hcrc != 0);
    }
    
    return header;
}

StatusCode ZDecompressor::prime(int bits, int value) {
    if (!_isInitialized) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = inflatePrime(&_impl->stream, bits, value);
    return static_cast<StatusCode>(result);
}

long ZDecompressor::getMark() {
    if (!_isInitialized) {
        return -1;
    }
    
    return inflateMark(&_impl->stream);
}

void ZDecompressor::setExpectGZip(bool expectGzip) {
    _expectGzip = expectGzip;
    if (_isInitialized) {
        // 如果已初始化，需要重新初始化以应用更改
        end();
        init();
    }
}

StatusCode ZDecompressor::validate(bool check) {
    if (!_isInitialized) {
        return StatusCode::STREAM_ERROR;
    }
    
    int result = inflateValidate(&_impl->stream, check ? 1 : 0);
    return static_cast<StatusCode>(result);
}

size_t ZDecompressor::getCodesUsed() {
    if (!_isInitialized) {
        return 0;
    }
    
    return static_cast<size_t>(inflateCodesUsed(&_impl->stream));
}

StatusCode ZDecompressor::init() {
    if (_isInitialized) {
        end();
    }
    
    int windowBits = _windowBits;
    if (_expectGzip) {
        // 自动检测gzip头，添加32
        windowBits += 32;
    }
    
    int result = inflateInit2(&_impl->stream, windowBits);
    
    if (result == Z_OK) {
        _isInitialized = true;
    }
    
    return static_cast<StatusCode>(result);
}

StatusCode ZDecompressor::end() {
    if (!_isInitialized) {
        return StatusCode::OK;
    }
    
    int result = inflateEnd(&_impl->stream);
    if (result == Z_OK) {
        _isInitialized = false;
    }
    
    return static_cast<StatusCode>(result);
}

} // namespace zlib_cpp
