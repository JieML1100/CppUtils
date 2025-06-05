#include "zchecksum.hpp"
#include <zlib.h>

namespace zlib_cpp {

// ZAdler32类实现
ZAdler32::ZAdler32(uint32_t initial)
    : _adler(initial) {
}

uint32_t ZAdler32::update(const uint8_t* data, size_t length) {
    if (data && length > 0) {
        _adler = adler32(_adler, reinterpret_cast<const Bytef*>(data), static_cast<uInt>(length));
    }
    return _adler;
}

uint32_t ZAdler32::update(const std::vector<uint8_t>& data) {
    if (!data.empty()) {
        _adler = adler32(_adler, reinterpret_cast<const Bytef*>(data.data()), static_cast<uInt>(data.size()));
    }
    return _adler;
}

uint32_t ZAdler32::getValue() const {
    return _adler;
}

void ZAdler32::reset() {
    _adler = 1; // Adler-32 的初始值为1
}

uint32_t ZAdler32::calculate(const uint8_t* data, size_t length, uint32_t initial) {
    if (!data || length == 0) {
        return initial;
    }
    return adler32(initial, reinterpret_cast<const Bytef*>(data), static_cast<uInt>(length));
}

uint32_t ZAdler32::calculate(const std::vector<uint8_t>& data, uint32_t initial) {
    if (data.empty()) {
        return initial;
    }
    return adler32(initial, reinterpret_cast<const Bytef*>(data.data()), static_cast<uInt>(data.size()));
}

uint32_t ZAdler32::combine(uint32_t adler1, uint32_t adler2, size_t len2) {
    return adler32_combine(adler1, adler2, static_cast<z_off_t>(len2));
}

// ZCRC32类实现
ZCRC32::ZCRC32(uint32_t initial)
    : _crc(initial) {
}

uint32_t ZCRC32::update(const uint8_t* data, size_t length) {
    if (data && length > 0) {
        _crc = crc32(_crc, reinterpret_cast<const Bytef*>(data), static_cast<uInt>(length));
    }
    return _crc;
}

uint32_t ZCRC32::update(const std::vector<uint8_t>& data) {
    if (!data.empty()) {
        _crc = crc32(_crc, reinterpret_cast<const Bytef*>(data.data()), static_cast<uInt>(data.size()));
    }
    return _crc;
}

uint32_t ZCRC32::getValue() const {
    return _crc;
}

void ZCRC32::reset() {
    _crc = 0; // CRC-32 的初始值为0
}

uint32_t ZCRC32::calculate(const uint8_t* data, size_t length, uint32_t initial) {
    if (!data || length == 0) {
        return initial;
    }
    return crc32(initial, reinterpret_cast<const Bytef*>(data), static_cast<uInt>(length));
}

uint32_t ZCRC32::calculate(const std::vector<uint8_t>& data, uint32_t initial) {
    if (data.empty()) {
        return initial;
    }
    return crc32(initial, reinterpret_cast<const Bytef*>(data.data()), static_cast<uInt>(data.size()));
}

uint32_t ZCRC32::combine(uint32_t crc1, uint32_t crc2, size_t len2) {
    return crc32_combine(crc1, crc2, static_cast<z_off_t>(len2));
}

const uint32_t* ZCRC32::getTable() {
    return get_crc_table();
}

} // namespace zlib_cpp
