// =============================================================================
//
// Copyright (c) 2010-2016 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


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
    std::shared_ptr<ValueType> doGet(const KeyType& key) const override;
    void doPut(const KeyType& key, std::shared_ptr<ValueType> entry) override;
    void doRemove(const KeyType& key) override;
    std::size_t doSize() const override;
    void doClear() override;

    /// \brief The underlying LRU cache implementation.
    mutable LRUCache<KeyType, ValueType> _LRUCache;

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
std::shared_ptr<ValueType> LRUMemoryCache<KeyType, ValueType>::doGet(const KeyType& key) const
{
    return _LRUCache.get(key);
}


template<typename KeyType, typename ValueType>
void LRUMemoryCache<KeyType, ValueType>::doPut(const KeyType& key, std::shared_ptr<ValueType> entry)
{
    _LRUCache.update(key, entry);
}


template<typename KeyType, typename ValueType>
void LRUMemoryCache<KeyType, ValueType>::doRemove(const KeyType& key)
{
    _LRUCache.remove(key);
}


template<typename KeyType, typename ValueType>
std::size_t LRUMemoryCache<KeyType, ValueType>::doSize() const
{
    return _LRUCache.size();
}


template<typename KeyType, typename ValueType>
void LRUMemoryCache<KeyType, ValueType>::doClear()
{
    _LRUCache.clear();
}


} } // namespace ofx::Cache
