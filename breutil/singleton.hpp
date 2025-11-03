#pragma once
#include <iostream>
#include <memory>
#include <mutex>

/*
使用方式：
class A : public Singleton<A> {}
使用A::Instance()获得指针A的智能指针


*/


template <typename T> class Singleton {
public:
    using Shared = std::shared_ptr<T>;
    virtual const char* getClassName() const { return ""; }

    static Shared Instance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            _instance = std::shared_ptr<T>(new T(), [](T* ptr) {
                delete ptr;
            });
            s_className = _instance->getClassName();
        });

        return _instance;
    }

    void PrintAddress() { std::cout << _instance.get() << std::endl; }

    virtual ~Singleton() {
        if (s_className == "") {
            s_className = typeid(*this).name();
        }
#ifndef NDEBUG
        std::cout << s_className << " as singleton destruct" << std::endl;
#endif
    }

protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>& st) = delete;

    inline static Shared _instance = nullptr;


private:
    inline static std::string s_className = "";
};
