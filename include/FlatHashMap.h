#ifndef FLAT_HASH_MAP_H
#define FLAT_HASH_MAP_H

#include <cstdint>
#include <cstddef>

class FlatHashMap
{
private:

    struct Bucket
    {
        uint64_t key;
        int value;

        bool occupied;
        bool deleted;

        Bucket()
        {
            occupied = false;
            deleted = false;
            key = 0;
            value = 0;
        }
    };

    Bucket* table;

    size_t capacity;
    size_t sz;

    static constexpr double MAX_LOAD = 0.7;

    size_t hash(uint64_t key) const;

    void rehash();

public:

    explicit FlatHashMap(size_t initialCapacity = 1024);

    ~FlatHashMap();

    bool insert(uint64_t key, int value);

    int* find(uint64_t key);

    bool erase(uint64_t key);

    bool contains(uint64_t key);

    size_t size() const;

    bool empty() const;

    void clear();
};

#endif