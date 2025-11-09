#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "../block_queue.hpp"
#include "../easy_test.hpp"

using namespace bre;

// ==================== 基础功能测试 ====================

TEST_CASE(BlockQueue_Constructor) {
    BlockQueue<int> queue1;
    ASSERT_EQ(1024, queue1.Capacity());
    ASSERT_TRUE(queue1.Empty());
    ASSERT_FALSE(queue1.IsClosed());

    BlockQueue<int> queue2(100);
    ASSERT_EQ(100, queue2.Capacity());
    ASSERT_TRUE(queue2.Empty());
}

TEST_CASE(BlockQueue_TryPush_TryPop) {
    BlockQueue<int> queue(5);

    // 测试 TryPush
    ASSERT_TRUE(queue.TryPush(1));
    ASSERT_TRUE(queue.TryPush(2));
    ASSERT_TRUE(queue.TryPush(3));
    ASSERT_EQ(3, queue.Size());
    ASSERT_FALSE(queue.Empty());

    // 测试 TryPop
    auto val = queue.TryPop();
    ASSERT_TRUE(val.has_value());
    ASSERT_EQ(1, val.value());

    val = queue.TryPop();
    ASSERT_TRUE(val.has_value());
    ASSERT_EQ(2, val.value());

    ASSERT_EQ(1, queue.Size());
}

TEST_CASE(BlockQueue_TryPush_Full) {
    BlockQueue<int> queue(3);

    ASSERT_TRUE(queue.TryPush(1));
    ASSERT_TRUE(queue.TryPush(2));
    ASSERT_TRUE(queue.TryPush(3));
    ASSERT_TRUE(queue.Full());

    // 队列满时应该返回 false
    ASSERT_FALSE(queue.TryPush(4));
    ASSERT_EQ(3, queue.Size());
}

TEST_CASE(BlockQueue_TryPop_Empty) {
    BlockQueue<int> queue(5);

    // 空队列 TryPop 应该返回 nullopt
    auto val = queue.TryPop();
    ASSERT_FALSE(val.has_value());
}

TEST_CASE(BlockQueue_Front_Back) {
    BlockQueue<int> queue(5);

    queue.TryPush(10);
    queue.TryPush(20);
    queue.TryPush(30);

    ASSERT_EQ(10, queue.Front());
    ASSERT_EQ(30, queue.Back());
}

TEST_CASE(BlockQueue_Front_Back_Empty_Exception) {
    BlockQueue<int> queue(5);

    // 空队列访问 Front/Back 应该抛出异常
    ASSERT_THROW(queue.Front(), std::runtime_error);
    ASSERT_THROW(queue.Back(), std::runtime_error);
}

TEST_CASE(BlockQueue_Clear) {
    BlockQueue<int> queue(10);

    for (int i = 0; i < 5; ++i) {
        queue.TryPush(i);
    }
    ASSERT_EQ(5, queue.Size());

    queue.Clear();
    ASSERT_TRUE(queue.Empty());
    ASSERT_EQ(0, queue.Size());
}

TEST_CASE(BlockQueue_SetCapacity) {
    BlockQueue<int> queue(3);

    queue.TryPush(1);
    queue.TryPush(2);
    queue.TryPush(3);
    ASSERT_TRUE(queue.Full());

    // 增加容量
    queue.SetCapacity(5);
    ASSERT_FALSE(queue.Full());
    ASSERT_EQ(5, queue.Capacity());
    ASSERT_TRUE(queue.TryPush(4));
    ASSERT_TRUE(queue.TryPush(5));

    // 减少容量
    queue.SetCapacity(3);
    ASSERT_EQ(3, queue.Capacity());
}

// ==================== 阻塞操作测试 ====================

TEST_CASE(BlockQueue_Push_Pop_Blocking) {
    BlockQueue<int> queue(5);

    // 在另一个线程中 Push
    std::thread producer([&queue]() {
        for (int i = 1; i <= 10; ++i) {
            queue.Push(i);
        }
    });

    // 主线程 Pop
    std::vector<int> results;
    for (int i = 0; i < 10; ++i) {
        int val;
        ASSERT_TRUE(queue.Pop(val));
        results.push_back(val);
    }

    producer.join();

    // 验证结果
    ASSERT_EQ(10, results.size());
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(i + 1, results[i]);
    }
}

TEST_CASE(BlockQueue_Pop_With_Timeout) {
    BlockQueue<int> queue(5);

    // 空队列 Pop 超时应该返回 false
    int val;
    auto start = std::chrono::steady_clock::now();
    bool result = queue.Pop(val, 100);
    auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_FALSE(result);
    ASSERT_GE(elapsed.count(), std::chrono::milliseconds(90).count());
}

TEST_CASE(BlockQueue_Push_With_Timeout) {
    BlockQueue<int> queue(3);

    // 填满队列
    queue.TryPush(1);
    queue.TryPush(2);
    queue.TryPush(3);
    ASSERT_TRUE(queue.Full());

    // Push 超时应该返回 false
    auto start = std::chrono::steady_clock::now();
    bool result = queue.Push(4, std::chrono::milliseconds(100));
    auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_FALSE(result);
    ASSERT_GE(elapsed.count(), std::chrono::milliseconds(90).count());
}

TEST_CASE(BlockQueue_Peek) {
    BlockQueue<int> queue(5);

    // 在另一个线程中延迟 Push
    std::thread producer([&queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.TryPush(42);
    });

    // Peek 应该等待并返回值，但不移除
    int val;
    bool result = queue.Peek(val, 200);
    ASSERT_TRUE(result);
    ASSERT_EQ(42, val);
    ASSERT_EQ(1, queue.Size());  // 元素仍在队列中

    producer.join();
}

// ==================== 批量操作测试 ====================

TEST_CASE(BlockQueue_PushBatch) {
    BlockQueue<int> queue(10);

    std::vector<int> data = {1, 2, 3, 4, 5};
    ASSERT_TRUE(queue.Push(data.begin(), data.end()));
    ASSERT_EQ(5, queue.Size());

    // 验证元素顺序
    for (int i = 1; i <= 5; ++i) {
        auto val = queue.TryPop();
        ASSERT_TRUE(val.has_value());
        ASSERT_EQ(i, val.value());
    }
}

TEST_CASE(BlockQueue_PushBatch_Capacity_Limit) {
    BlockQueue<int> queue(3);

    // 在另一个线程中延迟消费
    std::atomic<bool> consumed{false};
    std::thread consumer([&queue, &consumed]() {
        for (int i = 0; i < 3; ++i) {
            int data;
            queue.Pop(data);
        }
        consumed = true;
    });

    // 批量 Push 超过容量应该等待
    std::vector<int> data = {1, 2, 3, 4, 5};
    size_t pushedCount = queue.Push(data.begin(), data.end());

    // 等待消费者消费完
    while (!consumed) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_EQ(3, pushedCount);
    ASSERT_EQ(0, queue.Size());

    consumer.join();
}

TEST_CASE(BlockQueue_PopBatch) {
    BlockQueue<int> queue(10);

    // Push 一些数据
    for (int i = 1; i <= 7; ++i) {
        queue.TryPush(i);
    }

    // 批量 Pop
    std::vector<int> result(5);
    size_t count = queue.Pop(result.begin(), 5);

    ASSERT_EQ(5, count);
    ASSERT_EQ(2, queue.Size());

    for (int i = 0; i < 5; ++i) {
        ASSERT_EQ(i + 1, result[i]);
    }
}

TEST_CASE(BlockQueue_PopBatch_Less_Than_Request) {
    BlockQueue<int> queue(10);

    // 只 Push 3 个元素
    queue.TryPush(1);
    queue.TryPush(2);
    queue.TryPush(3);

    // 请求 Pop 5 个
    std::vector<int> result(5);
    size_t count = queue.Pop(result.begin(), 5);

    // 应该只返回 3 个
    ASSERT_EQ(3, count);
    ASSERT_TRUE(queue.Empty());
}

// ==================== 多线程测试 ====================

TEST_CASE(BlockQueue_MultiProducer_MultiConsumer) {
    BlockQueue<int> queue(20);
    const int items_per_producer = 50;
    const int num_producers = 3;
    const int num_consumers = 2;
    std::atomic<int> total_consumed{0};

    // 启动多个生产者
    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&queue, p]() {
            for (int i = 0; i < items_per_producer; ++i) {
                queue.Push(p * 1000 + i);
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }

    // 启动多个消费者
    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&queue, &total_consumed]() {
            int expected_items = (items_per_producer * num_producers) / 2;
            for (int i = 0; i < expected_items; ++i) {
                int val;
                if (queue.Pop(val, 2000)) {
                    total_consumed++;
                }
            }
        });
    }

    // 等待所有线程完成
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    ASSERT_EQ(items_per_producer * num_producers, total_consumed);
}

TEST_CASE(BlockQueue_ProducerConsumer_Pattern) {
    BlockQueue<int> queue(10);
    const int total_items = 100;
    std::atomic<int> sum{0};

    // 生产者线程
    std::thread producer([&queue]() {
        for (int i = 1; i <= total_items; ++i) {
            queue.Push(i);
        }
    });

    // 消费者线程
    std::thread consumer([&queue, &sum]() {
        for (int i = 0; i < total_items; ++i) {
            int val;
            if (queue.Pop(val, 2000)) {
                sum += val;
            }
        }
    });

    producer.join();
    consumer.join();

    // 验证总和：1 + 2 + ... + 100 = 5050
    int expected = total_items * (total_items + 1) / 2;
    ASSERT_EQ(expected, sum);
}

// ==================== 关闭功能测试 ====================

TEST_CASE(BlockQueue_Close_Basic) {
    BlockQueue<int> queue(5);

    queue.TryPush(1);
    queue.TryPush(2);
    ASSERT_FALSE(queue.IsClosed());

    queue.Close();
    ASSERT_TRUE(queue.IsClosed());

    // 关闭后不能 Push
    ASSERT_FALSE(queue.TryPush(3));
}

TEST_CASE(BlockQueue_Close_Wakes_Waiting_Threads) {
    BlockQueue<int> queue(5);

    // 启动等待 Pop 的线程
    std::atomic<bool> pop_returned{false};
    std::thread consumer([&queue, &pop_returned]() {
        int val;
        bool result = queue.Pop(val, 5000);
        ASSERT_FALSE(result);  // 应该返回 false
        pop_returned = true;
    });

    // 等待线程开始等待
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 关闭队列
    queue.Close();

    // 等待线程应该很快返回
    consumer.join();
    ASSERT_TRUE(pop_returned);
}

TEST_CASE(BlockQueue_Close_With_Remaining_Items) {
    BlockQueue<int> queue(5);

    queue.TryPush(1);
    queue.TryPush(2);
    queue.TryPush(3);

    queue.Close();

    // 关闭后仍可以 Pop 剩余元素
    auto val = queue.TryPop();
    ASSERT_TRUE(val.has_value());
    ASSERT_EQ(1, val.value());
}

TEST_CASE(BlockQueue_Push_After_Close_Throws) {
    BlockQueue<int> queue(5);
    queue.Close();

    // 阻塞式 Push 应该抛出异常
    ASSERT_THROW(queue.Push(42), std::runtime_error);
}

// ==================== 边界条件测试 ====================

TEST_CASE(BlockQueue_Capacity_One) {
    BlockQueue<int> queue(1);

    ASSERT_TRUE(queue.TryPush(42));
    ASSERT_TRUE(queue.Full());
    ASSERT_FALSE(queue.TryPush(43));

    auto val = queue.TryPop();
    ASSERT_TRUE(val.has_value());
    ASSERT_EQ(42, val.value());
    ASSERT_TRUE(queue.Empty());
}

TEST_CASE(BlockQueue_Large_Capacity) {
    BlockQueue<int> queue(10000);

    // 快速 Push 大量数据
    for (int i = 0; i < 5000; ++i) {
        ASSERT_TRUE(queue.TryPush(i));
    }

    ASSERT_EQ(5000, queue.Size());
    ASSERT_FALSE(queue.Full());
}

TEST_CASE(BlockQueue_MoveSemantics) {
    BlockQueue<std::string> queue(5);

    std::string str = "Hello, World!";
    queue.TryPush(std::move(str));

    auto result = queue.TryPop();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ("Hello, World!", result.value());
}

TEST_CASE(BlockQueue_Complex_Type) {
    struct Data {
        int id;
        std::string name;

        bool operator==(const Data& other) const { return id == other.id && name == other.name; }
    };

    BlockQueue<Data> queue(5);

    Data d1{1, "Alice"};
    Data d2{2, "Bob"};

    queue.TryPush(d1);
    queue.TryPush(d2);

    auto result = queue.TryPop();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(1, result.value().id);
    ASSERT_EQ("Alice", result.value().name);
}

// ==================== 通知功能测试 ====================

TEST_CASE(BlockQueue_Flush) {
    BlockQueue<int> queue(5);

    std::atomic<bool> woke_up{false};
    std::thread consumer([&queue, &woke_up]() {
        int val;
        // 等待会被 Flush 唤醒
        [[maybe_unused]] bool result = queue.Pop(val, 100);
        woke_up = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.Flush();

    consumer.join();
    ASSERT_TRUE(woke_up);
}

TEST_CASE(BlockQueue_NotifyAll) {
    BlockQueue<int> queue(2);

    queue.TryPush(1);
    queue.TryPush(2);

    std::atomic<int> woken_count{0};

    // 启动多个等待的生产者
    std::vector<std::thread> producers;
    for (int i = 0; i < 3; ++i) {
        producers.emplace_back([&queue, &woken_count]() {
            int val = 42;
            [[maybe_unused]] bool result = queue.Push(val, std::chrono::milliseconds(50));
            woken_count++;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Pop 一些元素并通知所有等待者
    queue.TryPop();
    queue.TryPop();
    queue.NotifyAll();

    for (auto& t : producers) t.join();

    ASSERT_GE(woken_count, 1);
}

void test_block_queue() { RUN_ALL_TESTS(); }
