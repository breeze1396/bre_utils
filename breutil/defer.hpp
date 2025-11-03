#pragma once

#include <functional>
#include <vector>

namespace bre {
class Defer {
public:
    template<typename Func>
    Defer(Func&& func) {
        Add(std::forward<Func>(func));
    }

    template<typename Func>
    void Add(Func&& func) {
        functions.push_back(std::forward<Func>(func));
    }

    ~Defer() {
        for (auto& f : functions) {
            f();
        }
    }

private:
    std::vector<std::function<void()>> functions;
};

} // namespace bre
