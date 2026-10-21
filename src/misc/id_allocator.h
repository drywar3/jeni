#pragma once

#include <mini.cc/array.h>

template<typename T, typename Id = usize>
struct Id_Allocator {
    mini::Array<T> pool;

    Id_Allocator() : pool() {}
    Id_Allocator(Mini_Allocator allocator) : pool(allocator) {}

    Id allocate(T value)
    {
        usize current = pool.count();
        pool.append(value);
        return Id(current);
    }

    T &get(Id id)
    {
        return pool[usize(id)];
    }

    const T &get(Id id) const
    {
        return pool[usize(id)];
    }

    T *get_ptr(Id id)
    {
        return &pool[usize(id)];
    }

    const T *get_ptr(Id id) const
    {
        return &pool[usize(id)];
    }
};
