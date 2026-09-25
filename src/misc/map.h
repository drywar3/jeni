#pragma once

#include <cstddef>          // std::size_t, std::nullptr_t
#include <cstdint>          // std::uintptr_t, std::uint64_t
#include <utility>          // std::move, std::forward, std::swap, std::pair
#include <initializer_list> // std::initializer_list

// ============================================================================
// General Purpose FNV-1a Hash Functor Template
// ============================================================================

template <typename T>
struct Hash {
    std::size_t operator()(const T& key) const {
        // FNV-1a 64-bit hash over raw byte representation of key
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&key);
        std::size_t hash = 14695981039346656037ULL;
        for (std::size_t i = 0; i < sizeof(T); ++i) {
            hash ^= bytes[i];
            hash *= 1099511628211ULL;
        }
        return hash;
    }
};

// Pointer Specialization
template <typename T>
struct Hash<T*> {
    std::size_t operator()(T* ptr) const {
        std::uintptr_t val = reinterpret_cast<std::uintptr_t>(ptr);
        return Hash<std::uintptr_t>{}(val);
    }
};

// C-String Specialization
template <>
struct Hash<const char*> {
    std::size_t operator()(const char* str) const {
        if (!str) return 0;
        std::size_t hash = 14695981039346656037ULL;
        while (*str) {
            hash ^= static_cast<unsigned char>(*str++);
            hash *= 1099511628211ULL;
        }
        return hash;
    }
};

// ============================================================================
// HashMap Class (Separate Chaining with Dynamic Rehash & Full Iterator Support)
// ============================================================================

template <typename Key, typename Value, typename HashFunc = Hash<Key>>
class HashMap {
public:
    struct Node {
        Key key;
        Value value;
        Node* next;

        template <typename K, typename V>
        Node(K&& k, V&& v, Node* n = nullptr)
            : key(std::forward<K>(k)), value(std::forward<V>(v)), next(n) {}
    };

    // Forward Iterator
    class Iterator {
    private:
        const HashMap* map_;
        std::size_t bucket_;
        Node* node_;

        void advance_to_valid() {
            while (node_ == nullptr && ++bucket_ < map_->capacity_) {
                node_ = map_->buckets_[bucket_];
            }
        }

    public:
        Iterator(const HashMap* map, std::size_t bucket, Node* node)
            : map_(map), bucket_(bucket), node_(node) {
            if (node_ == nullptr && map_ && bucket_ < map_->capacity_) {
                advance_to_valid();
            }
        }

        Node& operator*() const { return *node_; }
        Node* operator->() const { return node_; }

        Iterator& operator++() {
            if (node_) {
                node_ = node_->next;
                if (!node_) {
                    advance_to_valid();
                }
            }
            return *this;
        }

        bool operator==(const Iterator& other) const {
            return node_ == other.node_ && bucket_ == other.bucket_ && map_ == other.map_;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    };

private:
    Node** buckets_;
    std::size_t capacity_;
    std::size_t size_;
    float max_load_factor_;
    HashFunc hasher_;

    std::size_t get_bucket_index(const Key& key, std::size_t cap) const {
        return hasher_(key) % cap;
    }

    void check_and_rehash() {
        if (static_cast<float>(size_ + 1) / capacity_ > max_load_factor_) {
            rehash(capacity_ * 2);
        }
    }

public:
    explicit HashMap(std::size_t initial_capacity = 16, float max_load_factor = 0.75f)
        : capacity_(initial_capacity < 4 ? 4 : initial_capacity),
          size_(0),
          max_load_factor_(max_load_factor),
          hasher_(HashFunc()) {
        buckets_ = new Node*[capacity_](); // zero-initialized
    }

    HashMap(std::initializer_list<std::pair<Key, Value>> list)
        : HashMap(list.size() * 2) {
        for (const auto& item : list) {
            insert(item.first, item.second);
        }
    }

    ~HashMap() {
        clear();
        delete[] buckets_;
    }

    // Copy Semantics
    HashMap(const HashMap& other)
        : capacity_(other.capacity_), size_(0), max_load_factor_(other.max_load_factor_), hasher_(other.hasher_) {
        buckets_ = new Node*[capacity_]();
        for (std::size_t i = 0; i < other.capacity_; ++i) {
            Node* current = other.buckets_[i];
            while (current) {
                insert(current->key, current->value);
                current = current->next;
            }
        }
    }

    HashMap& operator=(const HashMap& other) {
        if (this != &other) {
            HashMap temp(other);
            swap(temp);
        }
        return *this;
    }

    // Move Semantics
    HashMap(HashMap&& other) noexcept
        : buckets_(other.buckets_), capacity_(other.capacity_), size_(other.size_),
          max_load_factor_(other.max_load_factor_), hasher_(std::move(other.hasher_)) {
        other.buckets_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
    }

    HashMap& operator=(HashMap&& other) noexcept {
        if (this != &other) {
            clear();
            delete[] buckets_;

            buckets_ = other.buckets_;
            capacity_ = other.capacity_;
            size_ = other.size_;
            max_load_factor_ = other.max_load_factor_;
            hasher_ = std::move(other.hasher_);

            other.buckets_ = nullptr;
            other.capacity_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    void swap(HashMap& other) noexcept {
        std::swap(buckets_, other.buckets_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(max_load_factor_, other.max_load_factor_);
        std::swap(hasher_, other.hasher_);
    }

    // Lookup & Access
    Value* find(const Key& key) {
        if (capacity_ == 0) return nullptr;
        std::size_t idx = get_bucket_index(key, capacity_);
        Node* current = buckets_[idx];

        while (current) {
            if (current->key == key) { // Uses operator==
                return &current->value;
            }
            current = current->next;
        }
        return nullptr;
    }

    const Value* find(const Key& key) const {
        if (capacity_ == 0) return nullptr;
        std::size_t idx = get_bucket_index(key, capacity_);
        Node* current = buckets_[idx];

        while (current) {
            if (current->key == key) { // Uses operator==
                return &current->value;
            }
            current = current->next;
        }
        return nullptr;
    }

    bool contains(const Key& key) const {
        return find(key) != nullptr;
    }

    Value& operator[](const Key& key) {
        Value* val = find(key);
        if (val) return *val;

        emplace(key, Value());
        return *find(key);
    }

    Value& operator[](Key&& key) {
        Value* val = find(key);
        if (val) return *val;

        Key k_copy = key;
        emplace(std::move(key), Value());
        return *find(k_copy);
    }

    // Insertion & Modification
    template <typename K, typename V>
    bool emplace(K&& key, V&& value) {
        std::size_t idx = get_bucket_index(key, capacity_);
        Node* current = buckets_[idx];

        while (current) {
            if (current->key == key) { // Uses operator==
                current->value = std::forward<V>(value);
                return false; // Key already existed, updated in-place
            }
            current = current->next;
        }

        check_and_rehash();
        idx = get_bucket_index(key, capacity_);

        Node* new_node = new Node(std::forward<K>(key), std::forward<V>(value), buckets_[idx]);
        buckets_[idx] = new_node;
        ++size_;
        return true;
    }

    bool insert(const Key& key, const Value& value) {
        return emplace(key, value);
    }

    bool insert(Key&& key, Value&& value) {
        return emplace(std::move(key), std::move(value));
    }

    bool erase(const Key& key) {
        if (capacity_ == 0) return false;
        std::size_t idx = get_bucket_index(key, capacity_);
        Node* current = buckets_[idx];
        Node* prev = nullptr;

        while (current) {
            if (current->key == key) { // Uses operator==
                if (prev) {
                    prev->next = current->next;
                } else {
                    buckets_[idx] = current->next;
                }
                delete current;
                --size_;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    void clear() {
        if (!buckets_) return;
        for (std::size_t i = 0; i < capacity_; ++i) {
            Node* current = buckets_[i];
            while (current) {
                Node* next = current->next;
                delete current;
                current = next;
            }
            buckets_[i] = nullptr;
        }
        size_ = 0;
    }

    void rehash(std::size_t new_capacity) {
        if (new_capacity < size_) new_capacity = size_;
        if (new_capacity == 0) new_capacity = 4;

        Node** new_buckets = new Node*[new_capacity]();

        for (std::size_t i = 0; i < capacity_; ++i) {
            Node* current = buckets_[i];
            while (current) {
                Node* next = current->next;
                std::size_t new_idx = hasher_(current->key) % new_capacity;

                current->next = new_buckets[new_idx];
                new_buckets[new_idx] = current;

                current = next;
            }
        }

        delete[] buckets_;
        buckets_ = new_buckets;
        capacity_ = new_capacity;
    }

    // Capacity Readouts
    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    // Iterators
    Iterator begin() {
        if (size_ == 0) return end();
        for (std::size_t i = 0; i < capacity_; ++i) {
            if (buckets_[i]) return Iterator(this, i, buckets_[i]);
        }
        return end();
    }

    Iterator end() { return Iterator(this, capacity_, nullptr); }
    const Iterator begin() const { return const_cast<HashMap*>(this)->begin(); }
    const Iterator end() const { return const_cast<HashMap*>(this)->end(); }
};
