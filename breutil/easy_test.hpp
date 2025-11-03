#pragma once

/** easy_test.hpp
 * 一个简单易用的 C++ 测试框架，支持多种断言和测试用例管理。
 * 使用方法：
 * 1. 包含头文件：#include "breUtils/easy_test.hpp"
 * 2. 使用 TEST_CASE 宏定义测试用例。
 * 3. 使用各种 ASSERT_* 宏进行断言。
 * 运行所有测试使用 RUN_ALL_TESTS() 宏。
 */

#include <chrono>
#include <cmath>
#include <format>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "enum.hpp"
#include "ostream_operator.hpp"

namespace bre {

// 检测是否存在全局 ToString(T) 函数
template <typename T>
concept HasGlobalToString = requires(T t) {
    {
        ToString(t)
    } -> std::convertible_to<std::string>;
};

// 检测是否存在成员函数 T.ToString()
template <typename T>
concept HasMemberToString = requires(T t) {
    {
        t.ToString()
    } -> std::convertible_to<std::string>;
};

// 检测是否支持 operator<<
template <typename T>
concept Streamable = requires(std::ostream& os, T t) {
    {
        os << t
    } -> std::convertible_to<std::ostream&>;
};


// 测试用例信息
struct TestCase {
    std::string name;
    std::function<void()> func;
    std::string file;
    int line;
};

class EasyTest {
public:
    static EasyTest& Instance() {
        static EasyTest instance;
        return instance;
    }

    // 注册测试用例
    void registerTest(const std::string& name, std::function<void()> func, const std::string& file,
                      int line) {
        _test_cases.push_back({name, std::move(func), file, line});
    }

    // 运行所有测试
    int runAllTests() {
        std::cout << Color::CYAN
                  << "==================== Running Tests ====================" << Color::RESET
                  << std::endl;

        auto start = std::chrono::high_resolution_clock::now();

        for (const auto& test : _test_cases) {
            _current_test_name = test.name;
            std::cout << Color::BLUE << "[ RUN      ] " << Color::RESET << test.name << std::endl;

            auto test_start = std::chrono::high_resolution_clock::now();
            try {
                test.func();
                auto test_end = std::chrono::high_resolution_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(test_end - test_start)
                        .count();

                std::cout << Color::GREEN << "[       OK ] " << Color::RESET << test.name << " ("
                          << duration << " ms)" << std::endl;
            } catch (const std::exception& e) {
                _failed_tests.push_back(test);

                auto test_end = std::chrono::high_resolution_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(test_end - test_start)
                        .count();

                std::cout << Color::RED << "[  FAILED  ] " << Color::RESET << test.name << " ("
                          << duration << " ms)" << std::endl;
                std::cout << Color::RED << "Exception: " << e.what() << Color::RESET << std::endl;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto total_duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        showResults(total_duration);
        return _failed_tests.size() > 0 ? 1 : 0;
    }

    // 断言：真值
    void assertTrue(bool expression, const std::string& expr_str, const std::string& file,
                    int line) {
        tests_run_++;
        if (!expression) {
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expected: " << expr_str << " is true" << std::endl
                      << "  Actual: false" << std::endl;
        }
    }

    // 断言：假值
    void assertFalse(bool expression, const std::string& expr_str, const std::string& file,
                     int line) {
        tests_run_++;
        if (expression) {
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expected: " << expr_str << " is false" << std::endl
                      << "  Actual: true" << std::endl;
        }
    }

    // 断言：相等
    template <typename T1, typename T2>
    void assertEqual(const T1& expected, const T2& actual, const std::string& expr_str,
                     const std::string& file, int line) {
        tests_run_++;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
        if (!(expected == actual)) {
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expression: " << expr_str << std::endl
                      << "  Expected: " << toString(expected) << std::endl
                      << "  Actual: " << toString(actual) << std::endl;
        }
    }

    // 断言：不相等
    template <typename T1, typename T2>
    void assertNotEqual(const T1& expected, const T2& actual, const std::string& expr_str,
                        const std::string& file, int line) {
        tests_run_++;
        if (expected == actual) {
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expression: " << expr_str << std::endl
                      << "  Expected: not equal to " << toString(expected) << std::endl
                      << "  Actual: " << toString(actual) << std::endl;
        }
    }

    // 断言：小于
    template <typename T1, typename T2>
    void assertLess(const T1& left, const T2& right, const std::string& expr_str,
                    const std::string& file, int line) {
        tests_run_++;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
        if (!(left < right)) {
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expression: " << expr_str << std::endl
                      << "  Expected: " << toString(left) << " < " << toString(right) << std::endl
                      << "  Actual: " << toString(left) << " >= " << toString(right) << std::endl;
        }
    }

    // 断言：小于等于
    template <typename T1, typename T2>
    void assertLessEqual(const T1& left, const T2& right, const std::string& expr_str,
                         const std::string& file, int line) {
        tests_run_++;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
        if (!(left <= right)) {
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expression: " << expr_str << std::endl
                      << "  Expected: " << toString(left) << " <= " << toString(right) << std::endl
                      << "  Actual: " << toString(left) << " > " << toString(right) << std::endl;
        }
    }

    // 断言：大于
    template <typename T1, typename T2>
    void assertGreater(const T1& left, const T2& right, const std::string& expr_str,
                       const std::string& file, int line) {
        tests_run_++;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
        if (!(left > right)) {
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expression: " << expr_str << std::endl
                      << "  Expected: " << toString(left) << " > " << toString(right) << std::endl
                      << "  Actual: " << toString(left) << " <= " << toString(right) << std::endl;
        }
    }

    // 断言：大于等于
    template <typename T1, typename T2>
    void assertGreaterEqual(const T1& left, const T2& right, const std::string& expr_str,
                            const std::string& file, int line) {
        tests_run_++;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
        if (!(left >= right)) {
#pragma clang diagnostic pop
#pragma GCC diagnostic pop
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expression: " << expr_str << std::endl
                      << "  Expected: " << toString(left) << " >= " << toString(right) << std::endl
                      << "  Actual: " << toString(left) << " < " << toString(right) << std::endl;
        }
    }

    // 断言：浮点数近似相等
    template <typename T>
    void assertNear(T expected, T actual, T epsilon, const std::string& expr_str,
                    const std::string& file, int line) {
        static_assert(std::is_floating_point_v<T>,
                      "assertNear only works with floating point types");
        tests_run_++;
        if (std::abs(expected - actual) > epsilon) {
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expression: " << expr_str << std::endl
                      << "  Expected: " << expected << " (±" << epsilon << ")" << std::endl
                      << "  Actual: " << actual << std::endl
                      << "  Diff: " << std::abs(expected - actual) << std::endl;
        }
    }

    // 断言：指针为空
    template <typename T>
    void assertNull(T* ptr, const std::string& expr_str, const std::string& file, int line) {
        tests_run_++;
        if (ptr != nullptr) {
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expected: " << expr_str << " is nullptr" << std::endl
                      << "  Actual: " << static_cast<void*>(ptr) << std::endl;
        }
    }

    // 断言：指针非空
    template <typename T>
    void assertNotNull(T* ptr, const std::string& expr_str, const std::string& file, int line) {
        tests_run_++;
        if (ptr == nullptr) {
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expected: " << expr_str << " is not nullptr" << std::endl
                      << "  Actual: nullptr" << std::endl;
        }
    }

    // 断言：抛出异常
    template <typename ExceptionType>
    void assertThrows(std::function<void()> test_func, const std::string& expr_str,
                      const std::string& file, int line) {
        tests_run_++;
        try {
            test_func();
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expected: " << expr_str << " throws exception" << std::endl
                      << "  Actual: no exception thrown" << std::endl;
        } catch (const ExceptionType&) {
            // 预期的异常
        } catch (...) {
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expected: " << expr_str << " throws specific exception" << std::endl
                      << "  Actual: different exception thrown" << std::endl;
        }
    }

    // 断言：不抛出异常
    void assertNoThrow(std::function<void()> test_func, const std::string& expr_str,
                       const std::string& file, int line) {
        tests_run_++;
        try {
            test_func();
        } catch (const std::exception& e) {
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expected: " << expr_str << " does not throw" << std::endl
                      << "  Actual: exception thrown: " << e.what() << std::endl;
        } catch (...) {
            _failed_tests.push_back({_current_test_name, nullptr, file, line});
            std::cerr << Color::RED << "[  FAILED  ] " << Color::RESET << file << ":" << line
                      << std::endl
                      << "  Expected: " << expr_str << " does not throw" << std::endl
                      << "  Actual: unknown exception thrown" << std::endl;
        }
    }

    // 显示测试结果
    void showResults(long long duration_ms = 0) {
        std::cout << Color::CYAN
                  << "=======================================================" << Color::RESET
                  << std::endl;

        int tests_passed = tests_run_ - _failed_tests.size();
        std::cout << "Total tests: " << tests_run_ << std::endl;
        std::cout << Color::GREEN << "Passed: " << tests_passed << Color::RESET << std::endl;

        if (_failed_tests.size() > 0) {
            std::cout << Color::RED << "Failed: " << _failed_tests.size() << Color::RESET
                      << std::endl;
        }

        if (duration_ms > 0) {
            std::cout << "Time: " << duration_ms << " ms" << std::endl;
        }

        if (_failed_tests.empty()) {
            std::cout << Color::GREEN << "\n✓ All tests passed!" << Color::RESET << std::endl;
        } else {
            std::cout << Color::RED << "\n✗ Some tests failed." << Color::RESET << std::endl;
            std::cout << Color::RED << "\nFailed Tests:" << Color::RESET << std::endl;
            for (const auto& test : _failed_tests) {
                std::cout << " - " << test.name << " (" << test.file << ":" << test.line << ")"
                          << std::endl;
            }
        }
        std::cout << Color::CYAN
                  << "=======================================================" << Color::RESET
                  << std::endl;
    }

    // 重置计数器
    void RESET() {
        tests_run_ = 0;
        _failed_tests.clear();
        _test_cases.clear();
        _current_test_name.clear();
    }

private:
    int tests_run_ = 0;
    std::vector<TestCase> _test_cases;
    std::vector<TestCase> _failed_tests;
    std::string _current_test_name;

    EasyTest() = default;
    EasyTest(const EasyTest&) = delete;
    EasyTest& operator=(const EasyTest&) = delete;

    // 特化：字符串类型
    static std::string toString(const std::string& value) { return "\"" + value + "\""; }

    static std::string toString(const char* value) { return std::string("\"") + value + "\""; }

    // 特化：布尔类型
    static std::string toString(bool value) { return value ? "true" : "false"; }

    // 特化：enum 类型
    template <typename E>
        requires std::is_enum_v<E>
    static std::string toString(E value) {
        // 优先使用全局 ToString(E) 函数
        if constexpr (HasGlobalToString<E>) {
            return ToString(value);
        } else {
            // 回退到打印底层整数值
            using Underlying = std::underlying_type_t<E>;
            std::ostringstream oss;
            oss << static_cast<Underlying>(value);
            return oss.str();
        }
    }

    // 特化：vector 类型
    template <typename T>
    static std::string toString(const std::vector<T>& vec) {
        std::ostringstream oss;
        oss << "[";
        size_t max_size = std::min(vec.size(), static_cast<size_t>(8));
        for (size_t i = 0; i < max_size; ++i) {
            oss << toString(vec[i]);
            if (i != max_size - 1) {
                oss << ", ";
            }
        }
        if (vec.size() > 8) {
            oss << ", ...";
        }
        oss << "]";
        return oss.str();
    }

    // 通用 toString：按优先级尝试不同的转换方法
    template <typename T>
        requires(!std::is_enum_v<T> && !std::is_same_v<std::remove_cvref_t<T>, std::string> &&
                 !std::is_same_v<std::remove_cvref_t<T>, char*> &&
                 !std::is_same_v<std::remove_cvref_t<T>, const char*> &&
                 !std::is_same_v<std::remove_cvref_t<T>, bool>)
    static std::string toString(const T& value) {
        // 1. 优先尝试全局 ToString(T) 函数
        if constexpr (HasGlobalToString<T>) {
            return ToString(value);
        }
        // 2. 尝试成员函数 T.ToString()
        else if constexpr (HasMemberToString<T>) {
            return value.ToString();
        }
        // 3. 尝试 operator<< 流操作符
        else if constexpr (Streamable<T>) {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
        // 4. 如果都不支持，返回类型信息和错误提示
        else {
            return std::string("<unprintable type: ") + typeid(T).name() + ">";
        }
    }
};

// ==================== 简单易用的宏定义 ====================

// 基本断言
#define ASSERT(expr) bre::EasyTest::Instance().assertTrue((expr), #expr, __FILE__, __LINE__)

#define ASSERT_TRUE(expr) bre::EasyTest::Instance().assertTrue((expr), #expr, __FILE__, __LINE__)

#define ASSERT_FALSE(expr) bre::EasyTest::Instance().assertFalse((expr), #expr, __FILE__, __LINE__)

// 比较断言
#define ASSERT_EQ(expected, actual) \
    bre::EasyTest::Instance().assertEqual((expected), (actual), #actual, __FILE__, __LINE__)

#define ASSERT_NE(expected, actual) \
    bre::EasyTest::Instance().assertNotEqual((expected), (actual), #actual, __FILE__, __LINE__)

#define ASSERT_LT(left, right) \
    bre::EasyTest::Instance().assertLess((left), (right), #left " < " #right, __FILE__, __LINE__)

#define ASSERT_LE(left, right)                                                                \
    bre::EasyTest::Instance().assertLessEqual((left), (right), #left " <= " #right, __FILE__, \
                                              __LINE__)

#define ASSERT_GT(left, right) \
    bre::EasyTest::Instance().assertGreater((left), (right), #left " > " #right, __FILE__, __LINE__)

#define ASSERT_GE(left, right)                                                                   \
    bre::EasyTest::Instance().assertGreaterEqual((left), (right), #left " >= " #right, __FILE__, \
                                                 __LINE__)

// 浮点数断言
#define ASSERT_NEAR(expected, actual, epsilon)                                               \
    bre::EasyTest::Instance().assertNear((expected), (actual), (epsilon), #actual, __FILE__, \
                                         __LINE__)

// 指针断言
#define ASSERT_NULL(ptr) bre::EasyTest::Instance().assertNull((ptr), #ptr, __FILE__, __LINE__)

#define ASSERT_NOT_NULL(ptr) \
    bre::EasyTest::Instance().assertNotNull((ptr), #ptr, __FILE__, __LINE__)

// 异常断言
#define ASSERT_THROW(expr, exception_type)                  \
    bre::EasyTest::Instance().assertThrows<exception_type>( \
        [&]() {                                             \
            expr;                                           \
        },                                                  \
        #expr, __FILE__, __LINE__)

#define ASSERT_NO_THROW(expr)                \
    bre::EasyTest::Instance().assertNoThrow( \
        [&]() {                              \
            expr;                            \
        },                                   \
        #expr, __FILE__, __LINE__)

// 测试用例定义
#define TEST_CASE(name)                                                                     \
    void test_##name();                                                                     \
    namespace {                                                                             \
    struct TestRegistrar_##name {                                                           \
        TestRegistrar_##name() {                                                            \
            bre::EasyTest::Instance().registerTest(#name, test_##name, __FILE__, __LINE__); \
        }                                                                                   \
    } registrar_##name;                                                                     \
    }                                                                                       \
    void test_##name()

// 运行所有测试
#define RUN_ALL_TESTS() bre::EasyTest::Instance().runAllTests()

// 显示结果（用于不使用 TEST_CASE 的情况）
#define SHOW_TEST_RESULTS() bre::EasyTest::Instance().showResults()

}  // namespace bre
