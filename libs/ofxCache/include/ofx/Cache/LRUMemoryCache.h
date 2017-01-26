//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofEvents.h"
#include "ofx/LRUCache.h"
#include "ofx/Cache/BaseCache.h"


namespace ofx {
namespace Cache {


/// \brief A thread-safe LRU memory cache.
///
/// An LRU (Least Recently Used) cache discards the least recently used elements
/// first. Elements that are accessed frequently are kept in the cache.
///
/// \sa https://en.wikipedia.org/wiki/Cache_algorithms#Overview
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
template<typename KeyType, typename ValueType>
class LRUMemoryCache: public BaseCache<KeyType, ValueType>
{
public:
    /// \brief Create an LRUCache with the given size.
    /// \param size The size of the LRU cache.
    LRUMemoryCache(std::size_t size = DEFAULT_CACHE_SIZE);

    /// \brief Destroy the memory cache.
    virtual ~LRUMemoryCache();

    enum
    {
        /// \brief The default number of elements stored in the LRU cache.
        DEFAULT_CACHE_SIZE = 2048
    };

protected:
    bool doHas(const KeyType& key) const override;
    std::shared_ptr<ValueType> doGet(const KeyType& key) override;
    void doAdd(const KeyType& key, std::shared_ptr<ValueType> entry) override;
    void doUpdate(const KeyType& key, std::shared_ptr<ValueType> entry) override;
    void doRemove(const KeyType& key) override;
    std::size_t doSize() override;
    void doClear() override;

    /// \brief The underlying LRU cache implementation.
    LRUCache<KeyType, ValueType> _LRUCache;

};


template<typename KeyType, typename ValueType>
LRUMemoryCache<KeyType, ValueType>::LRUMemoryCache(std::size_t size):
    _LRUCache(size)
{
}


template<typename KeyType, typename ValueType>
LRUMemoryCache<KeyType, ValueType>::~LRUMemoryCache()
{
}


template<typename KeyType, typename ValueType>
bool LRUMemoryCache<KeyType, ValueType>::doHas(const KeyType& key) const
{
    return _LRUCache.has(key);
}


template<typename KeyType, typename ValueType>
std::shared_ptr<ValueType> LRUMemoryCache<KeyType, ValueType>::doGet(const KeyType& key)
{
    return _LRUCache.get(key);
}


template<typename KeyType, typename ValueType>
void LRUMemoryCache<KeyType, ValueType>::doAdd(const KeyType& key, std::shared_ptr<ValueType> entry)
{
    _LRUCache.add(key, entry);
}


template<typename KeyType, typename ValueType>
void LRUMemoryCache<KeyType, ValueType>::doUpdate(const KeyType& key, std::shared_ptr<ValueType> entry)
{
    _LRUCache.update(key, entry);
}


template<typename KeyType, typename ValueType>
void LRUMemoryCache<KeyType, ValueType>::doRemove(const KeyType& key)
{
    _LRUCache.remove(key);
}


template<typename KeyType, typename ValueType>
std::size_t LRUMemoryCache<KeyType, ValueType>::doSize()
{
    return _LRUCache.size();
}


template<typename KeyType, typename ValueType>
void LRUMemoryCache<KeyType, ValueType>::doClear()
{
    _LRUCache.clear();
}


} } // namespace ofx::Cache
