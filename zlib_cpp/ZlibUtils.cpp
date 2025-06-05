#include "ZlibUtils.h"
#include <sstream>
#include <iomanip>

// 引入原始zlib头文件用于CRC32和Adler32计算
extern "C" {
#include "../zlib.h"
}

namespace ZlibCpp {

    std::vector<uint8_t> ZlibUtils::Compress(const std::string& data, 
                                           CompressionLevel level, 
                                           DataFormat format) {
        return ZlibCompressor::CompressString(data, level, format);
    }

    std::vector<uint8_t> ZlibUtils::Compress(const std::vector<uint8_t>& data, 
                                           CompressionLevel level, 
                                           DataFormat format) {
        return ZlibCompressor::CompressData(data, level, format);
    }

    std::string ZlibUtils::DecompressToString(const std::vector<uint8_t>& data, 
                                            DataFormat format) {
        return ZlibDecompressor::DecompressString(data, format);
    }

    std::vector<uint8_t> ZlibUtils::Decompress(const std::vector<uint8_t>& data, 
                                             DataFormat format) {
        return ZlibDecompressor::DecompressData(data, format);
    }

    bool ZlibUtils::CompressFile(const std::string& input_file, 
                               const std::string& output_file,
                               CompressionLevel level, 
                               DataFormat format) {
        try {
            auto input_data = ReadFile(input_file);
            auto compressed_data = Compress(input_data, level, format);
            return WriteFile(output_file, compressed_data);
        }
        catch (const std::exception&) {
            return false;
        }
    }

    bool ZlibUtils::DecompressFile(const std::string& input_file, 
                                 const std::string& output_file,
                                 DataFormat format) {
        try {
            auto input_data = ReadFile(input_file);
            auto decompressed_data = Decompress(input_data, format);
            return WriteFile(output_file, decompressed_data);
        }
        catch (const std::exception&) {
            return false;
        }
    }

    uint32_t ZlibUtils::CalculateAdler32(const std::vector<uint8_t>& data) {
        return static_cast<uint32_t>(adler32(1L, data.data(), static_cast<uInt>(data.size())));
    }

    uint32_t ZlibUtils::CalculateCRC32(const std::vector<uint8_t>& data) {
        return static_cast<uint32_t>(crc32(0L, data.data(), static_cast<uInt>(data.size())));
    }

    std::string ZlibUtils::GetVersion() {
        return std::string(zlibVersion());
    }

    DataFormat ZlibUtils::DetectFormat(const std::vector<uint8_t>& data) {
        if (data.size() < 2) {
            return DataFormat::Raw;
        }

        // 检查Gzip魔数 (0x1f, 0x8b)
        if (data[0] == 0x1f && data[1] == 0x8b) {
            return DataFormat::Gzip;
        }

        // 检查zlib格式 (CMF = 0x78, FLG的前5位应该是有效的)
        if (data[0] == 0x78 && (data[1] & 0x01) == 0) {
            // 简单的zlib格式检查
            uint16_t header = (data[0] << 8) | data[1];
            if (header % 31 == 0) {
                return DataFormat::Zlib;
            }
        }

        return DataFormat::Raw;
    }

    std::unique_ptr<ZlibCompressor> ZlibUtils::CreateGzipCompressor(CompressionLevel level) {
        return std::make_unique<ZlibCompressor>(level, DataFormat::Gzip);
    }

    std::unique_ptr<ZlibDecompressor> ZlibUtils::CreateGzipDecompressor() {
        return std::make_unique<ZlibDecompressor>(DataFormat::Gzip);
    }

    std::string ZlibUtils::Base64Encode(const std::vector<uint8_t>& data) {
        static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0;
        int valb = -6;

        for (uint8_t c : data) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                result.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }

        if (valb > -6) {
            result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }

        while (result.size() % 4) {
            result.push_back('=');
        }

        return result;
    }

    std::vector<uint8_t> ZlibUtils::Base64Decode(const std::string& encoded) {
        static const int T[128] = {
            -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
            52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
            -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
            15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
            -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
            41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
        };

        std::vector<uint8_t> result;
        int val = 0;
        int valb = -8;

        for (char c : encoded) {
            if (T[c & 0x7F] == -1) break;
            val = (val << 6) + T[c & 0x7F];
            valb += 6;
            if (valb >= 0) {
                result.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }

        return result;
    }

    std::string ZlibUtils::HexEncode(const std::vector<uint8_t>& data) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (uint8_t byte : data) {
            oss << std::setw(2) << static_cast<int>(byte);
        }
        return oss.str();
    }

    std::vector<uint8_t> ZlibUtils::HexDecode(const std::string& hex) {
        std::vector<uint8_t> result;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::strtol(byteString.c_str(), nullptr, 16));
            result.push_back(byte);
        }
        return result;
    }

    std::vector<uint8_t> ZlibUtils::ReadFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("无法打开文件: " + filename);
        }

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        
        if (!file) {
            throw std::runtime_error("读取文件失败: " + filename);
        }

        return buffer;
    }

    bool ZlibUtils::WriteFile(const std::string& filename, const std::vector<uint8_t>& data) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return file.good();
    }

} // namespace ZlibCpp 