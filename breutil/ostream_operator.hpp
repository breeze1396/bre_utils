#pragma once

/**
 * 当前文件用于breUtils命名空间下的各种类型重载输出流操作符（operator<<）。
 * 一般用于内部调试和日志记录，方便将自定义类型直接输出到标准输出流或文件流中。
 */

#include <iostream>

#include "enum.hpp"

namespace bre {

inline std::ostream& operator<<(std::ostream& os, Color color) {
    os << ColorToAnsi(color);
    return os;
}

}  // namespace bre