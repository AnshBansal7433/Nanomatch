#include "FlatHashMap.h"

#include <cstring>

FlatHashMap::FlatHashMap(size_t initialCapacity)
{
    capacity = 1;

    while(capacity < initialCapacity)
        capacity <<= 1;

    table = new Bucket[capacity];

    sz = 0;
}

FlatHashMap::~FlatHashMap()
{
    delete[] table;
}

size_t FlatHashMap::hash(uint64_t x) const
{
    x += 0x9e3779b97f4a7c15ULL;

    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;

    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;

    x ^= (x >> 31);

    return x & (capacity - 1);
}

size_t FlatHashMap::size() const
{
    return sz;
}

bool FlatHashMap::empty() const
{
    return sz == 0;
}

void FlatHashMap::clear()
{
    delete[] table;

    table = new Bucket[capacity];

    sz = 0;
}

int* FlatHashMap::find(uint64_t key)
{
    size_t idx = hash(key);

    while(table[idx].occupied)
    {
        if(!table[idx].deleted && table[idx].key == key)
            return &table[idx].value;

        idx = (idx + 1) & (capacity - 1);
    }

    return nullptr;
}

bool FlatHashMap::contains(uint64_t key)
{
    return find(key) != nullptr;
}

bool FlatHashMap::insert(uint64_t key,int value)
{
    if((double)(sz + 1) / capacity > MAX_LOAD)
        rehash();

    size_t idx = hash(key);

    while(table[idx].occupied)
    {
        if(!table[idx].deleted && table[idx].key == key)
        {
            table[idx].value = value;
            return true;
        }

        idx = (idx + 1) & (capacity - 1);
    }

    table[idx].occupied = true;
    table[idx].deleted = false;
    table[idx].key = key;
    table[idx].value = value;

    sz++;

    return true;
}

bool FlatHashMap::erase(uint64_t key)
{
    size_t idx = hash(key);

    while(table[idx].occupied)
    {
        if(!table[idx].deleted && table[idx].key == key)
        {
            table[idx].deleted = true;

            sz--;

            return true;
        }

        idx = (idx + 1) & (capacity - 1);
    }

    return false;
}

void FlatHashMap::rehash()
{
    Bucket* oldTable = table;

    size_t oldCapacity = capacity;

    capacity <<= 1;

    table = new Bucket[capacity];

    sz = 0;

    for(size_t i = 0; i < oldCapacity; i++)
    {
        if(oldTable[i].occupied && !oldTable[i].deleted)
        {
            insert(oldTable[i].key, oldTable[i].value);
        }
    }

    delete[] oldTable;
}

