//
//

#ifndef AARI_DATA_STRUCTURES_H
#define AARI_DATA_STRUCTURES_H

#include <string>
#include <exception>
#include <vector>
#include <entt/entt.hpp>

template<typename T>
class Stack {
    /** Simple stack implementation using a std::vector
     * and with clear and reserve methods
     */
public:
    Stack() = default;

    ~Stack() = default;

    void push(const T &item) {
        _items.push_back(item);
    }

    T pop() {
        auto a = _items.back();
        _items.pop_back();
        return std::move(a);
    }

    T top() {
        return _items.back();
    }

    void clear() {
        _items.clear();
    }

    void reserve(size_t size) {
        _items.reserve(size);
    }

    [[nodiscard]] size_t size() {
        return _items.size();
    }

    [[nodiscard]] bool empty() {
        return _items.empty();
    }

    std::vector<T> _items;
};

template<size_t N, typename... Args>
auto fill_with_null(Args... args) {
    std::array<entt::entity, N> arr = {args...};
    for (size_t i = sizeof...(args); i < N; ++i) {
        arr[i] = entt::null;
    }
    return arr;
}

#endif //AARI_DATA_STRUCTURES_H
