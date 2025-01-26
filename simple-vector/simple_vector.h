#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include "array_ptr.h"

class ReserveProxyObj {
public:
    explicit ReserveProxyObj(size_t capacity) : capacity_to_reserve_(capacity) {}
    size_t GetCapacityToReserve() { return capacity_to_reserve_; }

private:
    size_t capacity_to_reserve_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(ReserveProxyObj reserve)
        : size_(0),
          capacity_(reserve.GetCapacityToReserve()),
          items_(capacity_ > 0 ? ArrayPtr<Type>(capacity_) : ArrayPtr<Type>()) {}

    explicit SimpleVector(size_t size)
        : size_(size),
          capacity_(size),
          items_(size > 0 ? ArrayPtr<Type>(size) : ArrayPtr<Type>()) {
        if (size > 0) {
            std::fill(items_.Get(), items_.Get() + size_, Type{});
        }
    }

    SimpleVector(SimpleVector&& other) noexcept
        : size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)),
          items_(std::move(other.items_)) {}

    SimpleVector(const SimpleVector& other)
        : size_(other.size_),
          capacity_(other.capacity_),
          items_(ArrayPtr<Type>(capacity_)) {
        std::copy(other.items_.Get(), other.items_.Get() + size_, items_.Get());
    }

    SimpleVector(size_t size, const Type& value)
        : size_(size),
          capacity_(size),
          items_(size > 0 ? ArrayPtr<Type>(size) : ArrayPtr<Type>()) {
        for (size_t i = 0; i < size_; ++i) {
            items_[i] = value;
        }
    }

    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()),
          capacity_(size_),
          items_(ArrayPtr<Type>(capacity_)) {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            if (rhs.IsEmpty()) {
                Clear();
                return *this;
            }
            items_ = std::move(rhs.items_);
            size_ = std::exchange(rhs.size_, 0);
            capacity_ = std::exchange(rhs.capacity_, 0);
        }
        return *this;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            if (rhs.IsEmpty()) {
                Clear();
                return *this;
            }
            SimpleVector<Type> temp(rhs);
            swap(temp);
        }
        return *this;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_items(new_capacity);
            std::move(items_.Get(), items_.Get() + size_, new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            Reserve(capacity_ ? capacity_ * 2 : 1);
        }
        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            Reserve(capacity_ ? capacity_ * 2 : 1);
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = pos - begin();

        if (size_ == capacity_) {
            Reserve(capacity_ ? capacity_ * 2 : 1);
        }

        for (size_t i = size_; i > index; --i) {
            items_[i] = std::move(items_[i - 1]);
        }

        items_[index] = value;
        ++size_;
        return begin() + index;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = pos - begin();

        if (size_ == capacity_) {
            Reserve(capacity_ ? capacity_ * 2 : 1);
        }

        for (size_t i = size_; i > index; --i) {
            items_[i] = std::move(items_[i - 1]);
        }

        items_[index] = std::move(value);
        ++size_;
        return begin() + index;
    }

    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end());
        size_t index = pos - begin();
        for (size_t i = index; i < size_ - 1; ++i) {
            items_[i] = std::move(items_[i + 1]);
        }
        --size_;
        return begin() + index;
    }

    void swap(SimpleVector& other) noexcept {
        using std::swap;
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
        swap(items_, other.items_);
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        } else if (new_size > size_) {
            if (new_size > capacity_) {
                Reserve(new_size);
            }
            for (size_t i = size_; i < new_size; ++i) {
                items_[i] = Type();
            }
            size_ = new_size;
        }
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return begin();
    }

    ConstIterator cend() const noexcept {
        return end();
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> items_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) return false;
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
