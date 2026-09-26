#pragma once

#include <optional>
#include <mini.c/array.h>

#include "misc/map.h"

template<typename Key, typename Value>
using Map = HashMap<Key, Value>;

template <typename Key, typename Value>
struct DenseMap {
private:
    Map<Key, std::size_t> key_to_index_;
    MINI_ARRAY(Value) values_;
    Mini_Allocator allocator_;

public:
    DenseMap(Mini_Allocator allocator = mini_default_allocator()) : allocator_(allocator) {
        values_ = MINI_ARRAY_INIT(allocator, Value);
    }

    ~DenseMap() {
        mini_array_destroy(values_);
    }

    /* --- insert / lookup --- */

    /*
     * inserts or updates a key-value pair.
     * returns the dense index ([std::size_t]) assigned to [key].
     */
    std::size_t insert(const Key& key, Value value) {
        if (!key_to_index_.emplace(key, mini_array_count(values_))) {
            /* [key] already exists; update its value in [values_] */
            auto index = get_id(key);
            values_[*index] = std::move(value);
            return *index;
        }
        mini_array_append(values_, value);
        return mini_array_count(values_) - 1;
    }

    /* check if [key] exists in [key_to_index_] */
    bool contains(const Key& key) const {
        return key_to_index_.find(key) != nullptr;
    }

    /* get dense index corresponding to [key] (returns [std::nullopt] if missing) */
    std::optional<std::size_t> get_id(const Key& key) const {
        auto it = key_to_index_.find(key);
        if (it != nullptr) {
            return *it;
        }
        return std::nullopt;
    }

    /* access pointer to [Value] by [key] (returns [nullptr] if not found) */
    Value* find(const Key& key) {
        auto it = key_to_index_.find(key);
        return (it != key_to_index_.end()) ? &values_[it->second] : nullptr;
    }

    const Value* find(const Key& key) const {
        auto it = key_to_index_.find(key);
        return (it != key_to_index_.end()) ? &values_[it->second] : nullptr;
    }

    /* --- direct index access --- */

    Value& operator[](std::size_t index) { return values_[index]; }
    const Value& operator[](std::size_t index) const { return values_[index]; }

    Value& at_index(std::size_t index) { return values_[index]; }
    const Value& at_index(std::size_t index) const { return values_[index]; }

    /* --- capacity & iteration --- */

    std::size_t size() const { return mini_array_count(values_); }
    bool empty() const { return size() == 0; }
    void clear() {
        key_to_index_.clear();
        mini_array_clear(values_);
    }

    /* direct iteration over contiguous [values_] vector */
    auto begin() { return values_; }
    auto end() { return values_ + size(); }
    auto begin() const { return values_; }
    auto end() const { return values_ + size(); }

    /* direct access to raw [values_] dynamic vector storage */
    const Value* values() const { return values_; }
    Value* values() { return values_; }
};
