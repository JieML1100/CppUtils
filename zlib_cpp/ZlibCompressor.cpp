#include "ZlibCompressor.h"
#include <algorithm>

namespace ZlibCpp {

    ZlibCompressor::ZlibCompressor(CompressionLevel level, DataFormat format, CompressionStrategy strategy)
        : level_(level), format_(format), strategy_(strategy), finished_(false) {
        Initialize(level, format, strategy);
    }

        ZlibCompressor::~ZlibCompressor() {        if (initialized_) {            Cleanup();        }    }

    void ZlibCompressor::Initialize(CompressionLevel level, DataFormat format, CompressionStrategy strategy) {
        if (initialized_) {
            Cleanup();
        }

        level_ = level;
        format_ = format;
        strategy_ = strategy;
        finished_ = false;

        // 计算窗口位数
        int windowBits = 15;  // 默认窗口大小
        if (format == DataFormat::Gzip) {
            windowBits += 16;  // Gzip格式
        } else if (format == DataFormat::Raw) {
            windowBits = -windowBits;  // 原始deflate格式
        }

        int result = deflateInit2(&stream_,
                                 static_cast<int>(level),
                                 Z_DEFLATED,
                                 windowBits,
                                 8,  // memLevel
                                 static_cast<int>(strategy));

        CheckResult(result, "deflateInit2");
        initialized_ = true;
    }

    void ZlibCompressor::Reset() {
        if (!initialized_) {
            throw ZlibStreamException("压缩器未初始化");
        }

        int result = deflateReset(&stream_);
        CheckResult(result, "deflateReset");
        finished_ = false;
    }

    std::vector<uint8_t> ZlibCompressor::Compress(const std::vector<uint8_t>& input, bool flush) {
        return Compress(input.data(), input.size(), flush);
    }

    std::vector<uint8_t> ZlibCompressor::Compress(const uint8_t* input, size_t input_size, bool flush) {
        if (finished_) {
            throw ZlibStreamException("压缩器已完成，无法继续压缩");
        }

        int flush_mode = flush ? Z_SYNC_FLUSH : Z_NO_FLUSH;
        return CompressInternal(input, input_size, flush_mode);
    }

    std::vector<uint8_t> ZlibCompressor::Finish() {
        if (finished_) {
            return {};
        }

        finished_ = true;
        return CompressInternal(nullptr, 0, Z_FINISH);
    }

    std::vector<uint8_t> ZlibCompressor::CompressInternal(const uint8_t* input, size_t input_size, int flush_mode) {
        if (!initialized_) {
            throw ZlibStreamException("压缩器未初始化");
        }

        std::vector<uint8_t> output;
        const size_t buffer_size = 8192;  // 8KB缓冲区
        uint8_t buffer[buffer_size];

        stream_.next_in = const_cast<Bytef*>(input);
        stream_.avail_in = static_cast<uInt>(input_size);

        do {
            stream_.next_out = buffer;
            stream_.avail_out = buffer_size;

            int result = deflate(&stream_, flush_mode);
            
            if (result != Z_OK && result != Z_STREAM_END && result != Z_BUF_ERROR) {
                CheckResult(result, "deflate");
            }

            size_t produced = buffer_size - stream_.avail_out;
            if (produced > 0) {
                size_t old_size = output.size();
                output.resize(old_size + produced);
                std::copy(buffer, buffer + produced, output.begin() + old_size);
            }

            // 如果是Z_FINISH且返回Z_STREAM_END，表示压缩完成
            if (flush_mode == Z_FINISH && result == Z_STREAM_END) {
                break;
            }

        } while (stream_.avail_out == 0);  // 当输出缓冲区被填满时继续

        return output;
    }

    std::vector<uint8_t> ZlibCompressor::CompressData(const std::vector<uint8_t>& input,
                                                     CompressionLevel level,
                                                     DataFormat format) {
        ZlibCompressor compressor(level, format);
        auto result = compressor.Compress(input);
        auto final = compressor.Finish();
        
        result.insert(result.end(), final.begin(), final.end());
        return result;
    }

    std::vector<uint8_t> ZlibCompressor::CompressString(const std::string& input,
                                                       CompressionLevel level,
                                                       DataFormat format) {
        std::vector<uint8_t> data(input.begin(), input.end());
        return CompressData(data, level, format);
    }

    void ZlibCompressor::SetDictionary(const std::vector<uint8_t>& dictionary) {
        if (!initialized_) {
            throw ZlibStreamException("压缩器未初始化");
        }

        int result = deflateSetDictionary(&stream_, dictionary.data(), 
                                         static_cast<uInt>(dictionary.size()));
        CheckResult(result, "deflateSetDictionary");
    }

    void ZlibCompressor::GetParams(CompressionLevel& level, CompressionStrategy& strategy) const {
        level = level_;
        strategy = strategy_;
    }

    void ZlibCompressor::SetParams(CompressionLevel level, CompressionStrategy strategy) {
        if (!initialized_) {
            throw ZlibStreamException("压缩器未初始化");
        }

        int result = deflateParams(&stream_, static_cast<int>(level), static_cast<int>(strategy));
        CheckResult(result, "deflateParams");
        
        level_ = level;
        strategy_ = strategy;
    }

    void ZlibCompressor::Cleanup() {
        if (initialized_) {
            deflateEnd(&stream_);
            initialized_ = false;
        }
    }

} // namespace ZlibCpp 