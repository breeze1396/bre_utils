#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>
#include <algorithm>

namespace bre {


class Buffer {
public:
    static constexpr size_t kInitialSize = 1024;
    static constexpr size_t kPrependSize = 8;

    /**
     * @brief 构造函数
     * @param initialSize 初始缓冲区大小
     */
    explicit Buffer(size_t initialSize = kInitialSize);

    ~Buffer() = default;

    // 禁止拷贝
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // 支持移动
    Buffer(Buffer&& other) noexcept;

    Buffer& operator=(Buffer&& other) noexcept;

    /**
     * @brief 可读字节数
     */
    size_t ReadableBytes() const;

    /**
     * @brief 可写字节数
     */
    size_t WritableBytes() const;

    /**
     * @brief 预留空间大小
     */
    size_t PrependableBytes() const;

    /**
     * @brief 获取可读数据的起始指针
     */
    const char* Peek() const;

    /**
     * @brief 获取可写数据的起始指针
     */
    char* BeginWrite();

    const char* BeginWrite() const;

    /**
     * @brief 查找指定字符串
     * @param target 目标字符串
     * @return 找到返回指针，否则返回nullptr
     */
    const char* FindCRLF() const;

    const char* FindCRLF(const char* start) const;

    const char* FindEOL() const;

    /**
     * @brief 取出指定长度的数据
     * @param len 长度
     */
    void Retrieve(size_t len);

    /**
     * @brief 取出数据直到指定位置
     * @param end 结束位置
     */
    void RetrieveUntil(const char* end);

    /**
     * @brief 清空所有数据
     */
    void RetrieveAll();

    /**
     * @brief 取出所有数据作为字符串
     */
    std::string RetrieveAllAsString();

    /**
     * @brief 取出指定长度的数据作为字符串
     * @param len 长度
     */
    std::string RetrieveAsString(size_t len);

    /**
     * @brief 追加数据
     * @param data 数据指针
     * @param len 数据长度
     */
    void Append(const char* data, size_t len);

    void Append(std::string_view str);
    /**
     * @brief 确保有足够的可写空间
     * @param len 需要的空间大小
     */
    void EnsureWritableBytes(size_t len);

    /**
     * @brief 标记已写入的字节数
     * @param len 已写入的字节数
     */
    void HasWritten(size_t len);

    /**
     * @brief 标记已读取的字节数
     * @param len 已读取的字节数
     */
    void HasRead(size_t len);

    /**
     * @brief 转换为字符串（用于调试）
     */
    std::string ToString() const;

    /**
     * @brief 预留空间
     * @param len 预留的字节数
     */
    void Prepend(const void* data, size_t len);

    /**
     * @brief 收缩缓冲区到合适大小
     */
    void Shrink(size_t reserve = 0);

    /**
     * @brief 获取缓冲区总容量
     */
    size_t Capacity() const;

private:
    char* begin();

    const char* begin() const;

    void makeSpace(size_t len);

    std::vector<char> _buffer;   // 缓冲区
    size_t _readIndex;            // 读索引
    size_t _writeIndex;           // 写索引
};


#pragma region inline functions


inline Buffer::Buffer(size_t initialSize)
    : _buffer(kPrependSize + initialSize),
        _readIndex(kPrependSize),
        _writeIndex(kPrependSize) {}



inline Buffer::Buffer(Buffer&& other) noexcept
    : _buffer(std::move(other._buffer)),
        _readIndex(other._readIndex),
        _writeIndex(other._writeIndex) {
    other._readIndex = kPrependSize;
    other._writeIndex = kPrependSize;
}

inline Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        _buffer = std::move(other._buffer);
        _readIndex = other._readIndex;
        _writeIndex = other._writeIndex;
        other._readIndex = kPrependSize;
        other._writeIndex = kPrependSize;
    }
    return *this;
}

/**
 * @brief 可读字节数
 */
inline size_t Buffer::ReadableBytes() const {
    return _writeIndex - _readIndex;
}

/**
 * @brief 可写字节数
 */
inline size_t Buffer::WritableBytes() const {
    return _buffer.size() - _writeIndex;
}

/**
 * @brief 预留空间大小
 */
inline size_t Buffer::PrependableBytes() const {
    return _readIndex;
}

/**
 * @brief 获取可读数据的起始指针
 */
inline const char* Buffer::Peek() const {
    return begin() + _readIndex;
}

/**
 * @brief 获取可写数据的起始指针
 */
inline char* Buffer::BeginWrite() {
    return begin() + _writeIndex;
}

inline const char* Buffer::BeginWrite() const {
    return begin() + _writeIndex;
}

/**
 * @brief 查找指定字符串
 * @param target 目标字符串
 * @return 找到返回指针，否则返回nullptr
 */
inline const char* Buffer::FindCRLF() const {
    const char* start = Peek();
    const char* end   = BeginWrite();
    for (const char* p = start; p + 1 < end; ++p) {
        if (p[0] == '\r' && p[1] == '\n') {
            return p;
        }
    }
    return nullptr;
}

inline const char* Buffer::FindCRLF(const char* start) const {
    if (start < Peek() || start >= BeginWrite()) {
        return nullptr;
    }
    const char* end = BeginWrite();
    for (const char* p = start; p + 1 < end; ++p) {
        if (p[0] == '\r' && p[1] == '\n') {
            return p;
        }
    }
    return nullptr;
}

inline const char* Buffer::FindEOL() const {
    std::string_view sv(Peek(), ReadableBytes());
    const auto pos = sv.find('\n');
    return pos == std::string_view::npos ? nullptr : (Peek() + pos);
}

/**
 * @brief 取出指定长度的数据
 * @param len 长度
 */
inline void Buffer::Retrieve(size_t len) {
    if (len < ReadableBytes()) {
        _readIndex += len;
    } else {
        RetrieveAll();
    }
}

/**
 * @brief 取出数据直到指定位置
 * @param end 结束位置
 */
inline void Buffer::RetrieveUntil(const char* end) {
    if (end >= Peek() && end <= BeginWrite()) {
        Retrieve(end - Peek());
    }
}

/**
 * @brief 清空所有数据
 */
inline void Buffer::RetrieveAll() {
    _readIndex = kPrependSize;
    _writeIndex = kPrependSize;
}

/**
 * @brief 取出所有数据作为字符串
 */
inline std::string Buffer::RetrieveAllAsString() {
    return RetrieveAsString(ReadableBytes());
}

/**
 * @brief 取出指定长度的数据作为字符串
 * @param len 长度
 */
inline std::string Buffer::RetrieveAsString(size_t len) {
    if (len > ReadableBytes()) {
        len = ReadableBytes();
    }
    std::string result(Peek(), len);
    Retrieve(len);
    return result;
}

/**
 * @brief 追加数据
 * @param data 数据指针
 * @param len 数据长度
 */
inline void Buffer::Append(const char* data, size_t len) {
    EnsureWritableBytes(len);
    std::copy(data, data + len, BeginWrite());
    HasWritten(len);
}

inline void Buffer::Append(std::string_view str) {
    Append(str.data(), str.size());
}

/**
 * @brief 确保有足够的可写空间
 * @param len 需要的空间大小
 */
inline void Buffer::EnsureWritableBytes(size_t len) {
    if (WritableBytes() < len) {
        makeSpace(len);
    }
}

/**
 * @brief 标记已写入的字节数
 * @param len 已写入的字节数
 */
inline void Buffer::HasWritten(size_t len) {
    if (len <= WritableBytes()) {
        _writeIndex += len;
    }
}

/**
 * @brief 标记已读取的字节数
 * @param len 已读取的字节数
 */
inline void Buffer::HasRead(size_t len) {
    Retrieve(len);
}

/**
 * @brief 转换为字符串（用于调试）
 */
inline std::string Buffer::ToString() const {
    return std::string(Peek(), ReadableBytes());
}

/**
 * @brief 预留空间
 * @param len 预留的字节数
 */
inline void Buffer::Prepend(const void* data, size_t len) {
    if (len > PrependableBytes()) {
        throw std::length_error("Buffer::Prepend: not enough space");
    }
    _readIndex -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + _readIndex);
}

/**
 * @brief 收缩缓冲区到合适大小
 */
inline void Buffer::Shrink(size_t reserve) {
    std::vector<char> buf(kPrependSize + ReadableBytes() + reserve);
    const char* start = Peek();
    const char* end = BeginWrite();
    std::copy(start, end, buf.begin() + kPrependSize);
    buf.swap(_buffer);
    _writeIndex = kPrependSize + ReadableBytes();
    _readIndex = kPrependSize;
}

/**
 * @brief 获取缓冲区总容量
 */
inline size_t Buffer::Capacity() const {
    return _buffer.size();
}

inline char* Buffer::begin() {
    return _buffer.data();
}

inline const char* Buffer::begin() const {
    return _buffer.data();
}

inline void Buffer::makeSpace(size_t len) {
    if (WritableBytes() + PrependableBytes() < len + kPrependSize) {
        // 需要扩容
        _buffer.resize(_writeIndex + len);
    } else {
        // 移动数据到前面
        size_t readable = ReadableBytes();
        std::copy(begin() + _readIndex,
                    begin() + _writeIndex,
                    begin() + kPrependSize);
        _readIndex = kPrependSize;
        _writeIndex = _readIndex + readable;
    }
}


#pragma endregion inline functions

} // namespace bre

