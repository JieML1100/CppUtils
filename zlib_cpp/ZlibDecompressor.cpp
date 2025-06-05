#include "ZlibDecompressor.h"
#include <algorithm>

namespace ZlibCpp {

    ZlibDecompressor::ZlibDecompressor(DataFormat format)
        : format_(format), finished_(false) {
        Initialize(format);
    }

        ZlibDecompressor::~ZlibDecompressor() {        if (initialized_) {            Cleanup();        }    }

    void ZlibDecompressor::Initialize(DataFormat format) {
        if (initialized_) {
            Cleanup();
        }

        format_ = format;
        finished_ = false;

        // 计算窗口位数
        int windowBits = 15;  // 默认窗口大小
        if (format == DataFormat::Gzip) {
            windowBits += 16;  // Gzip格式
        } else if (format == DataFormat::Raw) {
            windowBits = -windowBits;  // 原始deflate格式
        }

        int result = inflateInit2(&stream_, windowBits);
        CheckResult(result, "inflateInit2");
        initialized_ = true;
    }

    void ZlibDecompressor::Reset() {
        if (!initialized_) {
            throw ZlibStreamException("解压缩器未初始化");
        }

        int result = inflateReset(&stream_);
        CheckResult(result, "inflateReset");
        finished_ = false;
    }

    std::vector<uint8_t> ZlibDecompressor::Decompress(const std::vector<uint8_t>& input, bool flush) {
        return Decompress(input.data(), input.size(), flush);
    }

    std::vector<uint8_t> ZlibDecompressor::Decompress(const uint8_t* input, size_t input_size, bool flush) {
        if (finished_) {
            throw ZlibStreamException("解压缩器已完成，无法继续解压缩");
        }

        int flush_mode = flush ? Z_SYNC_FLUSH : Z_NO_FLUSH;
        return DecompressInternal(input, input_size, flush_mode);
    }

    std::vector<uint8_t> ZlibDecompressor::Finish() {
        if (finished_) {
            return {};
        }

        finished_ = true;
        return DecompressInternal(nullptr, 0, Z_FINISH);
    }

    std::vector<uint8_t> ZlibDecompressor::DecompressInternal(const uint8_t* input, size_t input_size, int flush_mode) {
        if (!initialized_) {
            throw ZlibStreamException("解压缩器未初始化");
        }

        std::vector<uint8_t> output;
        const size_t buffer_size = 8192;  // 8KB缓冲区
        uint8_t buffer[buffer_size];

        stream_.next_in = const_cast<Bytef*>(input);
        stream_.avail_in = static_cast<uInt>(input_size);

        do {
            stream_.next_out = buffer;
            stream_.avail_out = buffer_size;

            int result = inflate(&stream_, flush_mode);

            // 处理需要字典的情况
            if (result == Z_NEED_DICT) {
                throw ZlibDataException("需要字典进行解压缩");
            }

            if (result != Z_OK && result != Z_STREAM_END && result != Z_BUF_ERROR) {
                CheckResult(result, "inflate");
            }

            size_t produced = buffer_size - stream_.avail_out;
            if (produced > 0) {
                size_t old_size = output.size();
                output.resize(old_size + produced);
                std::copy(buffer, buffer + produced, output.begin() + old_size);
            }

            // 如果返回Z_STREAM_END，表示解压缩完成
            if (result == Z_STREAM_END) {
                finished_ = true;
                break;
            }

        } while (stream_.avail_out == 0);  // 当输出缓冲区被填满时继续

        return output;
    }

    std::vector<uint8_t> ZlibDecompressor::DecompressData(const std::vector<uint8_t>& input, DataFormat format) {
        ZlibDecompressor decompressor(format);
        auto result = decompressor.Decompress(input);
        auto final = decompressor.Finish();
        
        result.insert(result.end(), final.begin(), final.end());
        return result;
    }

    std::string ZlibDecompressor::DecompressString(const std::vector<uint8_t>& input, DataFormat format) {
        auto data = DecompressData(input, format);
        return std::string(data.begin(), data.end());
    }

    void ZlibDecompressor::SetDictionary(const std::vector<uint8_t>& dictionary) {
        if (!initialized_) {
            throw ZlibStreamException("解压缩器未初始化");
        }

        int result = inflateSetDictionary(&stream_, dictionary.data(), 
                                         static_cast<uInt>(dictionary.size()));
        CheckResult(result, "inflateSetDictionary");
    }

    bool ZlibDecompressor::IsFinished() const {
        return finished_;
    }

    DataFormat ZlibDecompressor::GetFormat() const {
        return format_;
    }

    void ZlibDecompressor::Cleanup() {
        if (initialized_) {
            inflateEnd(&stream_);
            initialized_ = false;
        }
    }

} // namespace ZlibCpp 