#pragma once

#include <optional>
#include <utility>
#include <mini.c/array.h>
#include <vector>
#include <unordered_map>

#include "misc/map.h"

template <typename Key, typename Value> using Map = HashMap<Key, Value>;

template <typename Key, typename Value> struct DenseMap {
  private:
    std::unordered_map<Key, std::size_t> key_to_index_;
    std::vector<Value> values_;
    Mini_Allocator allocator_;

  public:
    DenseMap(Mini_Allocator allocator = mini_default_allocator())
        : values_(), allocator_(allocator)
    {
    }

    ~DenseMap() {  }

    DenseMap(const DenseMap &) = delete;
    DenseMap &operator=(const DenseMap &) = delete;

    DenseMap(DenseMap &&other) noexcept
        : key_to_index_(std::move(other.key_to_index_)),
          values_(std::move(other.values_)),
          allocator_(other.allocator_)
    {
    }

    /* --- insert / lookup --- */

    std::size_t insert(const Key &key, Value value)
    {
        if (auto index = get_id(key)) {
            values_[*index] = std::move(value);
            return *index;
        }
        std::size_t new_index = values_.size();
        key_to_index_[key] = new_index;
        values_.push_back(std::move(value));
        return new_index;
    }

    bool contains(const Key &key) const
    {
        return key_to_index_.find(key) != key_to_index_.end();
    }

    std::optional<std::size_t> get_id(const Key &key) const
    {
        auto it = key_to_index_.find(key);
        if (it != key_to_index_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    Value *find(const Key &key)
    {
        auto it = key_to_index_.find(key);
        return (it != key_to_index_.end()) ? &values_[it->second] : nullptr;
    }

    const Value *find(const Key &key) const
    {
        auto it = key_to_index_.find(key);
        return (it != key_to_index_.end()) ? &values_[it->second] : nullptr;
    }

    std::size_t add_value(Value value)
    {
        std::size_t current_index = values_.size();
        values_.push_back(std::move(value));
        return current_index;
    }

    void link(const Key &key, std::size_t index) { key_to_index_[key] = index; }

    /* --- direct index access --- */

    Value &operator[](std::size_t index) { return values_[index]; }
    const Value &operator[](std::size_t index) const { return values_[index]; }

    Value &at_index(std::size_t index) { return values_[index]; }
    const Value &at_index(std::size_t index) const { return values_[index]; }

    Value *at_index_ptr(std::size_t index) { return &values_[index]; }
    const Value *at_index_ptr(std::size_t index) const
    {
        return &values_[index];
    }

    /* --- capacity & iteration --- */

    std::size_t size() const { return values_.size(); }
    bool empty() const { return size() == 0; }
    void clear()
    {
        key_to_index_.clear();
        values_.clear();
    }

    auto begin() { return values_.begin(); }
    auto end() { return values_.end(); }
    auto begin() const { return values_.end(); }
    auto end() const { return values_.begin(); }
};
