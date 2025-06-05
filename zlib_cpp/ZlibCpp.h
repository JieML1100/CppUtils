#pragma once

/**
 * @file ZlibCpp.h
 * @brief zlib C++面向对象封装库主头文件
 * @version 1.0
 * @author ZlibCpp Team
 * 
 * 这个库提供了zlib的C++面向对象封装，包括：
 * - ZlibCompressor: 压缩器类
 * - ZlibDecompressor: 解压缩器类
 * - ZlibUtils: 便捷工具类
 * - ZlibException: 异常处理类
 * 
 * 支持的数据格式：
 * - Zlib: 标准zlib格式
 * - Gzip: Gzip格式
 * - Raw: 原始deflate格式
 */

// 核心类
#include "ZlibException.h"
#include "ZlibBase.h"
#include "ZlibCompressor.h"
#include "ZlibDecompressor.h"
#include "ZlibUtils.h"

/**
 * @namespace ZlibCpp
 * @brief zlib C++封装库的命名空间
 */
namespace ZlibCpp {

    /**
     * @brief 库版本信息
     */
    constexpr const char* ZLIB_CPP_VERSION = "1.0.0";

    /**
     * @brief 获取库版本
     * @return 版本字符串
     */
    inline std::string GetLibraryVersion() {
        return ZLIB_CPP_VERSION;
    }

    /**
     * @brief 获取底层zlib版本
     * @return zlib版本字符串
     */
    inline std::string GetZlibVersion() {
        return ZlibUtils::GetVersion();
    }

} // namespace ZlibCpp 