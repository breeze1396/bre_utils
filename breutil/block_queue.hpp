#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace bre {

template <class T>
class BlockQueue {
public:
    explicit BlockQueue(size_t MaxCapacity = 1024) : _capacity(MaxCapacity), _isClose(false) {}

    ~BlockQueue() { Close(); }

    // 禁止拷贝
    BlockQueue(const BlockQueue &) = delete;
    BlockQueue &operator=(const BlockQueue &) = delete;

    // 支持移动
    BlockQueue(BlockQueue &&) noexcept = default;
    BlockQueue &operator=(BlockQueue &&) noexcept = default;

    void Clear() {
        std::lock_guard<std::mutex> locker(_mtx);
        _queue = std::queue<T>();
        _condProducer.notify_all();
    }

    bool Empty() const {
        std::lock_guard<std::mutex> locker(_mtx);
        return _queue.empty();
    }

    bool Full() const {
        std::lock_guard<std::mutex> locker(_mtx);
        return _queue.size() >= _capacity;
    }

    void Close() {
        {
            std::lock_guard<std::mutex> locker(_mtx);
            _isClose = true;
        }
        _condProducer.notify_all();
        _condConsumer.notify_all();
    }

    bool IsClosed() const {
        std::lock_guard<std::mutex> locker(_mtx);
        return _isClose;
    }

    size_t Size() const {
        std::lock_guard<std::mutex> locker(_mtx);
        return _queue.size();
    }

    size_t Capacity() const {
        std::lock_guard<std::mutex> locker(_mtx);
        return _capacity;
    }

    // 动态调整容量
    void SetCapacity(size_t newCapacity) {
        std::lock_guard<std::mutex> locker(_mtx);
        _capacity = newCapacity;
        if (_queue.size() < _capacity) {
            _condProducer.notify_all();  // 容量增加，通知等待的生产者
        }
    }

    T Front() const {  // 新增：获取队首元素
        std::lock_guard<std::mutex> locker(_mtx);
        if (_queue.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return _queue.front();
    }

    T Back() const {  // 添加 const
        std::lock_guard<std::mutex> locker(_mtx);
        if (_queue.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return _queue.back();
    }

    // 非阻塞 Push，如果队列满则返回 false
    bool TryPush(const T &item) {
        std::lock_guard<std::mutex> locker(_mtx);
        if (_isClose || _queue.size() >= _capacity) {
            return false;
        }
        _queue.push(item);
        _condConsumer.notify_one();
        return true;
    }

    bool TryPush(T &&item) {
        std::lock_guard<std::mutex> locker(_mtx);
        if (_isClose || _queue.size() >= _capacity) {
            return false;
        }
        _queue.push(std::move(item));
        _condConsumer.notify_one();
        return true;
    }

    void Push(const T &item) {
        std::unique_lock<std::mutex> locker(_mtx);
        _condProducer.wait(locker, [this] {
            return _isClose || _queue.size() < _capacity;
        });
        if (_isClose) {
            throw std::runtime_error("Queue is closed");
        }
        _queue.push(item);
        _condConsumer.notify_one();
    }

    void Push(T &&item) {
        std::unique_lock<std::mutex> locker(_mtx);
        _condProducer.wait(locker, [this] {
            return _isClose || _queue.size() < _capacity;
        });
        if (_isClose) {
            throw std::runtime_error("Queue is closed");
        }
        _queue.push(std::move(item));
        _condConsumer.notify_one();
    }

    // 支持超时的 Push
    template <typename Rep, typename Period>
    bool Push(const T &item, const std::chrono::duration<Rep, Period> &timeout) {
        std::unique_lock<std::mutex> locker(_mtx);
        if (!_condProducer.wait_for(locker, timeout, [this] {
                return _isClose || _queue.size() < _capacity;
            })) {
            return false;
        }
        if (_isClose) {
            return false;
        }
        _queue.push(item);
        _condConsumer.notify_one();
        return true;
    }

    template <typename Rep, typename Period>
    bool Push(T &&item, const std::chrono::duration<Rep, Period> &timeout) {
        std::unique_lock<std::mutex> locker(_mtx);
        if (!_condProducer.wait_for(locker, timeout, [this] {
                return _isClose || _queue.size() < _capacity;
            })) {
            return false;
        }
        if (_isClose) {
            return false;
        }
        _queue.push(std::move(item));
        _condConsumer.notify_one();
        return true;
    }

    // 非阻塞
    std::optional<T> TryPop() {
        std::lock_guard<std::mutex> locker(_mtx);
        if (_queue.empty()) {
            return std::nullopt;
        }
        T item = std::move(_queue.front());
        _queue.pop();
        _condProducer.notify_one();
        return item;
    }

    // 从队列拿走一个元素
    bool Pop(T &item) {
        std::unique_lock<std::mutex> locker(_mtx);
        _condConsumer.wait(locker, [this] {
            return _isClose || !_queue.empty();
        });
        if (_isClose && _queue.empty()) {
            return false;
        }
        item = std::move(_queue.front());
        _queue.pop();
        _condProducer.notify_one();
        return true;
    }

    // 从队列查看第一个元素，不取出
    bool Peek(T &item, int timeout_ms) {
        std::unique_lock<std::mutex> locker(_mtx);
        if (!_condConsumer.wait_for(locker, std::chrono::milliseconds(timeout_ms), [this] {
                return _isClose || !_queue.empty();
            })) {
            return false;
        }
        if (_isClose && _queue.empty()) {
            return false;
        }
        item = _queue.front();
        return true;
    }

    bool Pop(T &item, int timeout_ms) {
        std::unique_lock<std::mutex> locker(_mtx);
        if (!_condConsumer.wait_for(locker, std::chrono::milliseconds(timeout_ms), [this] {
                return _isClose || !_queue.empty();
            })) {
            return false;
        }
        if (_isClose && _queue.empty()) {
            return false;
        }
        item = std::move(_queue.front());
        _queue.pop();
        _condProducer.notify_one();
        return true;
    }

    // 批量操作：一次性 Push 多个元素
    template <typename InputIt>
    bool Push(InputIt first, InputIt last) {
        // 判断个数是否满足全部放入，否则一个一个放入
        int count = std::distance(first, last);
        {
            std::lock_guard<std::mutex> locker(_mtx);
            if (!_isClose && (_queue.size() + count <= _capacity)) {
                for (auto it = first; it != last; ++it) {
                    _queue.push(*it);
                }
                _condConsumer.notify_all();
                return true;
            }
        }

        bool result;
        for (auto it = first; it != last; ++it) {
            result = Push(*it);
            if (!result) {
                return false;
            }
        }
        return true;
    }

    // 批量操作：一次性 Pop 多个元素
    template <typename OutputIt>
    size_t Pop(OutputIt dest, size_t maxCount) {
        std::unique_lock<std::mutex> locker(_mtx);
        _condConsumer.wait(locker, [this] {
            return _isClose || !_queue.empty();
        });

        size_t count = 0;
        while (!_queue.empty() && count < maxCount) {
            *dest++ = std::move(_queue.front());
            _queue.pop();
            ++count;
        }
        if (count > 0) {
            _condProducer.notify_all();
        }
        return count;
    }

    // 让读取加速
    void Flush() { _condConsumer.notify_one(); }

    // 唤醒所有等待的线程
    void NotifyAll() {
        _condConsumer.notify_all();
        _condProducer.notify_all();
    }

private:
    size_t _capacity;
    bool _isClose;
    std::queue<T> _queue;
    mutable std::mutex _mtx;
    std::condition_variable _condConsumer;
    std::condition_variable _condProducer;
};


}  // namespace bre
