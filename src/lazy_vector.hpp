#pragma once

#include <memory>
#include <utility>
#include <vector>

namespace ari {

template <typename T>
class LazyVector {
public:
    using Container = std::vector<T>;
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

    LazyVector() = default;
    LazyVector(LazyVector&&) noexcept = default;
    LazyVector& operator=(LazyVector&&) noexcept = default;
    LazyVector(const LazyVector&) = delete;
    LazyVector& operator=(const LazyVector&) = delete;

    LazyVector& operator=(Container values) {
        set(std::move(values));
        return *this;
    }

    bool empty() const { return values_ ? values_->empty() : true; }
    size_type size() const { return values_ ? values_->size() : 0; }

    void reserve(size_type capacity) { ensure().reserve(capacity); }
    void clear() { values_.reset(); }

    template <typename U>
    void push_back(U&& value) {
        ensure().push_back(std::forward<U>(value));
    }

    template <typename... Args>
    decltype(auto) emplace_back(Args&&... args) {
        return ensure().emplace_back(std::forward<Args>(args)...);
    }

    T& operator[](size_type index) { return ensure()[index]; }
    const T& operator[](size_type index) const { return get()[index]; }

    T& front() { return ensure().front(); }
    const T& front() const { return get().front(); }

    iterator begin() { return ensure().begin(); }
    iterator end() { return ensure().end(); }
    const_iterator begin() const { return get().begin(); }
    const_iterator end() const { return get().end(); }
    const_iterator cbegin() const { return get().cbegin(); }
    const_iterator cend() const { return get().cend(); }

    operator Container&() { return ensure(); }
    operator const Container&() const { return get(); }

    Container take() {
        if (!values_) return {};
        Container values = std::move(*values_);
        values_.reset();
        return values;
    }

private:
    const Container& get() const {
        static const Container empty;
        return values_ ? *values_ : empty;
    }

    Container& ensure() {
        if (!values_) values_ = std::make_unique<Container>();
        return *values_;
    }

    void set(Container values) {
        if (values.empty()) {
            values_.reset();
            return;
        }
        ensure() = std::move(values);
    }

    std::unique_ptr<Container> values_;
};

} // namespace ari
