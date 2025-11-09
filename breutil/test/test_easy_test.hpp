#pragma once
// EasyTest 框架使用示例
#include <stdexcept>
#include <vector>

#include "../easy_test.hpp"

// ==================== 方式一：使用 TEST_CASE 宏自动注册测试 ====================

TEST_CASE(BasicAssertions) {
    // 基本断言
    ASSERT(1 + 1 == 2);
    ASSERT_TRUE(true);
    ASSERT_FALSE(false);
}

TEST_CASE(EqualityAssertions) {
    // 相等性断言
    ASSERT_EQ(42, 42);
    ASSERT_NE(42, 43);

    std::string str1 = "hello";
    std::string str2 = "hello";
    ASSERT_EQ(str1, str2);
}

TEST_CASE(ComparisonAssertions) {
    // 比较断言
    ASSERT_LT(1, 2);
    ASSERT_LE(2, 2);
    ASSERT_GT(3, 2);
    ASSERT_GE(3, 3);
}

TEST_CASE(FloatingPointAssertions) {
    // 浮点数断言
    double pi = 3.14159;
    ASSERT_NEAR(pi, 3.14, 0.01);
    ASSERT_NEAR(0.1 + 0.2, 0.3, 1e-10);
}

TEST_CASE(PointerAssertions) {
    // 指针断言
    int* null_ptr = nullptr;
    int value = 42;
    int* valid_ptr = &value;

    ASSERT_NULL(null_ptr);
    ASSERT_NOT_NULL(valid_ptr);
}

TEST_CASE(ExceptionAssertions) {
    // 异常断言
    ASSERT_THROW(throw std::runtime_error("error"), std::runtime_error);
    ASSERT_NO_THROW([]() {
        int x = 1 + 1;
        return x;
    }());
}

TEST_CASE(ContainerOperations) {
    // 容器测试
    std::vector<int> vec = {1, 2, 3, 4, 5};

    ASSERT_EQ(vec.size(), 5u);
    ASSERT_FALSE(vec.empty());
    ASSERT_EQ(vec[0], 1);
    ASSERT_EQ(vec.back(), 5);
}

// 故意失败的测试示例
TEST_CASE(FailingTest) {
    ASSERT_EQ(1, 2);  // 这会失败
}

// 测试类 .ToString()
class SampleClass1 {
public:
    std::string ToString() const { return "SampleClass instance"; }
    bool operator==(const SampleClass1&) const { return false; }
};

class SampleClass2 {
public:
    bool operator==(const SampleClass2&) const { return false; }
};

inline std::string ToString(const SampleClass2&) {
    return "SampleClass2 instance via global ToString";
}


TEST_CASE(CustomTypeAssertion) {
    SampleClass1 obj1;
    SampleClass1 obj2;
    ASSERT_EQ(obj1, obj2);

    SampleClass2 obj3;
    SampleClass2 obj4;
    ASSERT_EQ(obj3, obj4);
}

// ==================== 方式二：手动使用断言（不自动注册） ====================

void manual_test_example() {
    std::cout << "\n--- Manual Test Example ---\n";

    ASSERT_TRUE(1 == 1);
    ASSERT_EQ(10, 5 + 5);
    ASSERT_LT(1, 10);

    // 显示结果
    SHOW_TEST_RESULTS();
}

void test_easy_test() {
    int result = RUN_ALL_TESTS();

    // 运行单独的测试
    // manual_test_example();
    std::cout << "result: " << result << std::endl;
    std::cout << "\nAll EasyTest tests completed with result code: " << result << std::endl;
}
