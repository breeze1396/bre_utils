#define BOOST_TEST_MODULE BufferTest
#include <boost/test/included/unit_test.hpp>
#include "breutil/buffer.hpp"
#include <string>

BOOST_AUTO_TEST_SUITE(BufferTestSuite)

BOOST_AUTO_TEST_CASE(test_initial_state) {
    bre::Buffer buffer;
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 0);
    BOOST_CHECK_EQUAL(buffer.WritableBytes(), bre::Buffer::kInitialSize);
    BOOST_CHECK_EQUAL(buffer.PrependableBytes(), bre::Buffer::kPrependSize);
}

BOOST_AUTO_TEST_CASE(test_append_and_retrieve) {
    bre::Buffer buffer;
    std::string data = "Hello, World!";
    buffer.Append(data);
    
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), data.size());
    BOOST_CHECK_EQUAL(std::string(buffer.Peek(), buffer.ReadableBytes()), data);
    
    std::string retrieved = buffer.RetrieveAsString(5);
    BOOST_CHECK_EQUAL(retrieved, "Hello");
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), data.size() - 5);
}

BOOST_AUTO_TEST_CASE(test_retrieve_all) {
    bre::Buffer buffer;
    buffer.Append("Test data");
    
    std::string all = buffer.RetrieveAllAsString();
    BOOST_CHECK_EQUAL(all, "Test data");
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 0);
}

BOOST_AUTO_TEST_CASE(test_find_crlf) {
    bre::Buffer buffer;
    buffer.Append("Line 1\r\nLine 2\r\nLine 3");
    
    const char* crlf = buffer.FindCRLF();
    BOOST_CHECK(crlf != nullptr);
    BOOST_CHECK_EQUAL(std::string(buffer.Peek(), crlf - buffer.Peek()), "Line 1");
}

BOOST_AUTO_TEST_CASE(test_find_eol) {
    bre::Buffer buffer;
    buffer.Append("First line\nSecond line\n");
    
    const char* eol = buffer.FindEOL();
    BOOST_CHECK(eol != nullptr);
    BOOST_CHECK_EQUAL(std::string(buffer.Peek(), eol - buffer.Peek()), "First line");
}

BOOST_AUTO_TEST_CASE(test_ensure_writable_bytes) {
    bre::Buffer buffer(10);
    BOOST_CHECK_EQUAL(buffer.WritableBytes(), 10);
    
    buffer.EnsureWritableBytes(100);
    BOOST_CHECK(buffer.WritableBytes() >= 100);
}

BOOST_AUTO_TEST_CASE(test_prepend) {
    bre::Buffer buffer;
    buffer.Append("World");
    
    const char* hello = "Hello ";
    buffer.Prepend(hello, 6);
    
    BOOST_CHECK_EQUAL(buffer.RetrieveAllAsString(), "Hello World");
}

BOOST_AUTO_TEST_CASE(test_move_constructor) {
    bre::Buffer buffer1;
    buffer1.Append("Test data");
    
    bre::Buffer buffer2(std::move(buffer1));
    BOOST_CHECK_EQUAL(buffer2.ReadableBytes(), 9);
    BOOST_CHECK_EQUAL(buffer2.RetrieveAllAsString(), "Test data");
    BOOST_CHECK_EQUAL(buffer1.ReadableBytes(), 0);
}

BOOST_AUTO_TEST_CASE(test_move_assignment) {
    bre::Buffer buffer1;
    buffer1.Append("Test data");
    
    bre::Buffer buffer2;
    buffer2 = std::move(buffer1);
    
    BOOST_CHECK_EQUAL(buffer2.ReadableBytes(), 9);
    BOOST_CHECK_EQUAL(buffer2.RetrieveAllAsString(), "Test data");
    BOOST_CHECK_EQUAL(buffer1.ReadableBytes(), 0);
}

BOOST_AUTO_TEST_CASE(test_retrieve_until) {
    bre::Buffer buffer;
    buffer.Append("Hello, World!");
    
    const char* comma = buffer.Peek() + 5;
    buffer.RetrieveUntil(comma);
    
    BOOST_CHECK_EQUAL(buffer.RetrieveAllAsString(), ", World!");
}

BOOST_AUTO_TEST_CASE(test_has_written) {
    bre::Buffer buffer;
    char* writable = buffer.BeginWrite();
    std::memcpy(writable, "Test", 4);
    buffer.HasWritten(4);
    
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 4);
    BOOST_CHECK_EQUAL(buffer.RetrieveAllAsString(), "Test");
}

BOOST_AUTO_TEST_CASE(test_shrink) {
    bre::Buffer buffer(1000);
    buffer.Append("Small data");
    
    size_t oldCapacity = buffer.Capacity();
    buffer.Shrink();
    
    BOOST_CHECK(buffer.Capacity() < oldCapacity);
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 10);
    BOOST_CHECK_EQUAL(buffer.RetrieveAllAsString(), "Small data");
}

BOOST_AUTO_TEST_CASE(test_large_data) {
    bre::Buffer buffer;
    std::string largeData(10000, 'A');
    buffer.Append(largeData);
    
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 10000);
    BOOST_CHECK_EQUAL(buffer.RetrieveAllAsString(), largeData);
}

BOOST_AUTO_TEST_CASE(test_multiple_append_retrieve) {
    bre::Buffer buffer;
    
    buffer.Append("First");
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 5);
    
    buffer.Append(" Second");
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 12);
    
    std::string first = buffer.RetrieveAsString(5);
    BOOST_CHECK_EQUAL(first, "First");
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 7);
    
    std::string second = buffer.RetrieveAllAsString();
    BOOST_CHECK_EQUAL(second, " Second");
}

BOOST_AUTO_TEST_CASE(test_comprehensive_buffer_operations) {
    bre::Buffer buffer;
    
    // 测试初始状态
    BOOST_CHECK_EQUAL(buffer.WritableBytes(), bre::Buffer::kInitialSize);
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 0);
    
    // 添加数据
    buffer.Append("Hello, ");
    char str[] = "World!";
    buffer.Append(str, 6);
    BOOST_CHECK_EQUAL(buffer.ToString(), "Hello, World!");
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 13);
    
    // 取出部分数据
    std::string part1 = buffer.RetrieveAsString(7);
    BOOST_CHECK_EQUAL(part1, "Hello, ");
    BOOST_CHECK_EQUAL(buffer.ToString(), "World!");
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 6);
    
    // 继续添加和取出
    buffer.Append(str, 6);
    buffer.Retrieve(6);
    BOOST_CHECK_EQUAL(buffer.ToString(), "World!");
}

BOOST_AUTO_TEST_CASE(test_large_volume_append_retrieve) {
    bre::Buffer buffer;
    char str[] = "World!";
    
    // 写入超过1024个字符，测试自动扩容
    for(int i = 0; i < 1000; ++i) {
        buffer.Append(str, 6);
    }
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 6000);
    
    // 取出大部分数据
    buffer.Retrieve(5990);
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 10);
    // "World!" 重复 1000 次，第 5991-6000 字节是 "rld!World!"
    BOOST_CHECK_EQUAL(buffer.ToString(), "rld!World!");
    
    // 获取所有字符
    std::string all = buffer.RetrieveAllAsString();
    BOOST_CHECK_EQUAL(all, "rld!World!");
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 0);
}

BOOST_AUTO_TEST_CASE(test_performance_repeated_operations) {
    bre::Buffer buffer;
    char str[] = "world!";
    
    // 测试重复的添加和取出操作
    for(int i = 0; i < 10000; ++i) {
        buffer.Append(str, 6);
        buffer.Retrieve(6);
    }
    
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 0);
    // 确保缓冲区在重复操作后仍然正常工作
    buffer.Append("Test");
    BOOST_CHECK_EQUAL(buffer.ToString(), "Test");
}

BOOST_AUTO_TEST_CASE(test_retrieve_until_with_pointer) {
    bre::Buffer buffer;
    buffer.Append("Hello, World! How are you?");
    
    // 找到第一个空格并取出到该位置
    const char* space = static_cast<const char*>(
        std::memchr(buffer.Peek(), ' ', buffer.ReadableBytes())
    );
    BOOST_REQUIRE(space != nullptr);
    
    buffer.RetrieveUntil(space);
    BOOST_CHECK_EQUAL(buffer.ToString(), " World! How are you?");
}

BOOST_AUTO_TEST_CASE(test_has_read) {
    bre::Buffer buffer;
    buffer.Append("Test data for HasRead");
    
    size_t originalSize = buffer.ReadableBytes();
    buffer.HasRead(5);
    
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), originalSize - 5);
    BOOST_CHECK_EQUAL(buffer.ToString(), "data for HasRead");
}

BOOST_AUTO_TEST_CASE(test_peek_and_beginwrite) {
    bre::Buffer buffer;
    buffer.Append("Hello");
    
    // 测试 Peek
    const char* peek = buffer.Peek();
    BOOST_CHECK_EQUAL(std::string(peek, 5), "Hello");
    
    // 测试 BeginWrite
    char* write = buffer.BeginWrite();
    std::memcpy(write, " World", 6);
    buffer.HasWritten(6);
    
    BOOST_CHECK_EQUAL(buffer.ToString(), "Hello World");
}

BOOST_AUTO_TEST_CASE(test_find_crlf_from_start) {
    bre::Buffer buffer;
    buffer.Append("First\r\nSecond\r\nThird");
    
    // 找到第一个 CRLF
    const char* crlf1 = buffer.FindCRLF();
    BOOST_REQUIRE(crlf1 != nullptr);
    BOOST_CHECK_EQUAL(std::string(buffer.Peek(), crlf1 - buffer.Peek()), "First");
    
    // 从第一个 CRLF 之后继续找
    const char* crlf2 = buffer.FindCRLF(crlf1 + 2);
    BOOST_REQUIRE(crlf2 != nullptr);
    BOOST_CHECK_EQUAL(std::string(crlf1 + 2, crlf2 - (crlf1 + 2)), "Second");
}

BOOST_AUTO_TEST_CASE(test_capacity_and_shrink) {
    bre::Buffer buffer(2048);
    BOOST_CHECK(buffer.Capacity() >= 2048 + bre::Buffer::kPrependSize);
    
    buffer.Append("Small");
    size_t oldCapacity = buffer.Capacity();
    
    buffer.Shrink(100);
    BOOST_CHECK(buffer.Capacity() < oldCapacity);
    BOOST_CHECK_EQUAL(buffer.ToString(), "Small");
}

BOOST_AUTO_TEST_CASE(test_prepend_boundary) {
    bre::Buffer buffer;
    buffer.Append("World");
    
    // 测试正常的 Prepend
    buffer.Prepend("Hello ", 6);
    BOOST_CHECK_EQUAL(buffer.RetrieveAllAsString(), "Hello World");
    
    // 测试 Prepend 空间不足的情况
    bre::Buffer buffer2;
    buffer2.Append("Data");
    
    // 尝试 Prepend 超过 kPrependSize 的数据
    char largeData[20] = "TooLargeForPrepend";
    BOOST_CHECK_THROW(buffer2.Prepend(largeData, 19), std::length_error);
}

BOOST_AUTO_TEST_CASE(test_writable_bytes_after_operations) {
    bre::Buffer buffer(100);
    size_t initialWritable = buffer.WritableBytes();
    BOOST_CHECK_EQUAL(initialWritable, 100);
    
    buffer.Append("Test");
    BOOST_CHECK_EQUAL(buffer.WritableBytes(), initialWritable - 4);
    
    buffer.Retrieve(2);
    // Retrieve 不会增加 WritableBytes (除非触发内部重排)
    BOOST_CHECK(buffer.WritableBytes() <= initialWritable - 4);
}

BOOST_AUTO_TEST_CASE(test_empty_operations) {
    bre::Buffer buffer;
    
    // 测试在空缓冲区上的操作
    BOOST_CHECK_EQUAL(buffer.ToString(), "");
    BOOST_CHECK_EQUAL(buffer.RetrieveAllAsString(), "");
    BOOST_CHECK(buffer.FindCRLF() == nullptr);
    BOOST_CHECK(buffer.FindEOL() == nullptr);
    
    // 空 Retrieve 不应崩溃
    buffer.Retrieve(0);
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 0);
}

BOOST_AUTO_TEST_CASE(test_boundary_retrieve) {
    bre::Buffer buffer;
    buffer.Append("Hello");
    
    // Retrieve 超过可读字节数应该清空缓冲区
    buffer.Retrieve(1000);
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), 0);
}

BOOST_AUTO_TEST_CASE(test_append_string_view) {
    bre::Buffer buffer;
    std::string_view sv = "String View Test";
    buffer.Append(sv);
    
    BOOST_CHECK_EQUAL(buffer.ToString(), "String View Test");
    BOOST_CHECK_EQUAL(buffer.ReadableBytes(), sv.size());
}

BOOST_AUTO_TEST_CASE(test_continuous_prepend_append) {
    bre::Buffer buffer;
    buffer.Append("Middle");
    buffer.Prepend("Start", 5);
    buffer.Append("End");
    
    BOOST_CHECK_EQUAL(buffer.RetrieveAllAsString(), "StartMiddleEnd");
}

BOOST_AUTO_TEST_SUITE_END()
