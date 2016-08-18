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


#include "ofEvent.h"
#include "ofx/Cache/BaseStore.h"


namespace ofx {
namespace Cache {


/// \brief A thread-safe cascading cache node.
///
/// Caches can be chained in order to have several layers of caching, e.g.
///
///     memory cache -> disk cache -> internet source
///
/// Caches are templated on KeyTypes and ValueTypes. Since there may be several
/// intemediate data types to cache (e.g. a raw buffer cached on disk vs. a
/// ofPixels object cached in memory), adapter caches are available translate
/// between cache layers that differ in KeyType or ValueType.
///
/// Subclasses _must_ protected their own data to allow multi-threaded access.
///
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
template<typename KeyType, typename ValueType>
class BaseCache: public BaseWritableStore<KeyType, ValueType>
{
public:
    typedef BaseReadableStore<KeyType, ValueType> ChildStore;

    /// \brief A collection of possible request statuses.
    enum class CacheStatus
    {
        /// \brief An unknown or undetermined status.
        NONE,
        /// \brief A response was generated from the cache with no requests sent upstream.
        CACHE_HIT,
        /// \brief The response came from an upstream server.
        CACHE_MISS,
        /// \brief The response was generated directly by the caching module.
        CACHE_MODULE_RESPONSE,
        /// \brief The response was generated from the cache after validating the entry with the origin server.
        VALIDATED,

    };

    /// \brief Create a default BaseCache.
    BaseCache(std::unique_ptr<ChildStore> childStore = nullptr);

    /// \brief Destroy the BaseCache.
    virtual ~BaseCache();

    /// \brief Recursively get a value by its key.
    ///
    /// This function will recursively call child cache nodes and will cache
    /// any child node values that are not currently available in this cache
    /// node.
    ///
    /// This method is synchronous and will block until the get operation is
    /// complete.
    ///
    /// \param key The key to get.
    /// \returns std::shared_ptr<ValueType> or nullptr if the cache missed.
    std::shared_ptr<ValueType> get(const KeyType& key) override;

    /// \returns the number of elements in this cache node cache.
    std::size_t size();

    /// \brief Clear all values in this cache node.
    void clear();

    /// \brief Take ownership of the passed std::unique_ptr<StoreType>.
    ///
    /// This this is "sink" meaning that any child passed to this will be
    /// owned by this Node.
    ///
    /// \param element the rvalue reference to the child node.
    /// \returns A pointer to the added StoreType. The parent StoreType retains
    /// ownership of the pointer via a std::unique_ptr.
    /// \tparam StoreType The StoreType.
    template<typename StoreType>
    StoreType* setChild(std::unique_ptr<StoreType> store);

    /// \brief Create a child using a templated StoreType.
    ///
    /// To create a child StoreType you can use this method like:
    ///
    /// StoreType* element = parentCache->setChild<StoreType>(arguments ...);
    ///
    /// \param args The variable constructor arguments for the StoreType.
    /// \returns A pointer to the added Cache. The parent Cache retains
    /// ownership of the pointer via a std::unique_ptr.
    /// \tparam StoreType The Cache Type.
    /// \tparam Args the StoreType constructor arguments.
    template<typename StoreType, typename... Args>
    StoreType* setChild(Args&&... args);

    /// \brief Release ownership of a child Cache.
    /// \param element The cache to release.
    /// \returns a std::unique_ptr<BaseCache<KeyType, ValueType>> to the child or nullptr if none.
    std::unique_ptr<ChildStore> removeChild();

    /// \brief An event called when the cache is cleared.
    ofEvent<void> onClear;
    
protected:
    virtual std::size_t doSize() = 0;
    virtual void doClear() = 0;

private:
    std::unique_ptr<ChildStore> _childStore = nullptr;

};


template<typename KeyType, typename ValueType>
BaseCache<KeyType, ValueType>::BaseCache(std::unique_ptr<ChildStore> childStore)
{
    setChild(std::move(childStore));
}


template<typename KeyType, typename ValueType>
BaseCache<KeyType, ValueType>::~BaseCache()
{
}


template<typename KeyType, typename ValueType>
std::shared_ptr<ValueType> BaseCache<KeyType, ValueType>::get(const KeyType& key)
{
    auto result = this->doGet(key);

    if (result != nullptr)
    {
        return result;
    }
    else if (_childStore != nullptr)
    {
        result = _childStore->get(key);

        if (result != nullptr)
        {
            this->onAdd.notify(this, std::make_pair(key, result));
            this->doAdd(key, result);
        }

        return result;
    }
    else return nullptr;
}


template<typename KeyType, typename ValueType>
std::size_t BaseCache<KeyType, ValueType>::size()
{
    return doSize();
}


template<typename KeyType, typename ValueType>
void BaseCache<KeyType, ValueType>::clear()
{
    onClear.notify(this);
    doClear();
}


template<typename KeyType, typename ValueType>
template<typename StoreType, typename... Args>
StoreType* BaseCache<KeyType, ValueType>::setChild(Args&&... args)
{
    return addChild(std::make_unique<StoreType>(std::forward<Args>(args)...));
}


template<typename KeyType, typename ValueType>
template<typename StoreType>
StoreType* BaseCache<KeyType, ValueType>::setChild(std::unique_ptr<StoreType> cache)
{

    static_assert(std::is_base_of<ChildStore, StoreType>(), "StoreType must be derived from BaseStore.");
    // Get a raw pointer to the node for later.
    StoreType* pNode = cache.get();

    // Take ownership of the node.
    _childStore = std::move(cache);

    return pNode;
}


template<typename KeyType, typename ValueType>
std::unique_ptr<typename BaseCache<KeyType, ValueType>::ChildStore> BaseCache<KeyType, ValueType>::removeChild()
{
    return std::move(_childStore);
}



///// \brief A simple base cache type.
/////
///// Subclasses must implement all of the protected do* methods. The request()
///// method defaults to the doGet() method.  Event though some cache
///// implementations may be thread safe on get() or put(), care should be given
///// to when various events are notified.
/////
///// All functions are synchronous and expected to block to complete their tasks.
///// Thus, for caches that involve disk or network operations, etc (e.g. sqlite,
///// file system, etc) the user might consider using an asynchronous cache.
/////
///// \tparam KeyType The key type.
///// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
//template<typename KeyType, typename ValueType>
//class BaseCache
//{
//public:
//    /// \brief A collection of possible request statuses.
//    enum class CacheStatus
//    {
//        /// \brief An unknown or undetermined status.
//        NONE,
//        /// \brief A response was generated from the cache with no requests sent upstream.
//        CACHE_HIT,
//        /// \brief The response came from an upstream server.
//        CACHE_MISS,
//        /// \brief The response was generated directly by the caching module.
//        CACHE_MODULE_RESPONSE,
//        /// \brief The response was generated from the cache after validating the entry with the origin server.
//        VALIDATED,
//
//    };
//
//    /// \brief Destroy the BaseCache.
//    virtual ~BaseCache()
//    {
//    }
//
//    /// \brief Determine if the given value is cached.
//    ///
//    /// Depending on the cache implementation, it may be more efficient to
//    /// simply call get() and check for a nullptr.
//    ///
//    /// \param key The key to check.
//    /// \returns true if the requested value is cached.
//    bool has(const KeyType& key) const
//    {
//        return doHas(key);
//    }
//
//    /// \brief Get a value by its key.
//    /// \param key The key to get.
//    /// \returns true a shared pointer to the value or nullptr if not cached.
//    std::shared_ptr<ValueType> get(const KeyType& key) const
//    {
//        auto result = doGet(key);
//
//        if (result != nullptr)
//        {
//            this->onHit.notify(this, key);
//        }
//        else
//        {
//            this->onMiss.notify(this, key);
//        }
//        
//        return result;
//    }
//
//    /// \brief Cache a value.
//    ///
//    /// Putting a value in the cache will overwrite any existing value for the
//    /// given key.
//    ///
//    /// \param key The key to cache.
//    /// \param entry The value to cache.
//    void put(const KeyType& key, std::shared_ptr<ValueType> entry)
//    {
//        doPut(key, entry);
//        onPut.notify(this, std::make_pair(key, entry));
//    }
//
//    /// \brief Cache a value.
//    ///
//    /// Putting a value in the cache will overwrite any existing value for the
//    /// given key.
//    ///
//    /// The value will be copied to a shared pointer, thus a copy-constructor
//    /// must be avaialble.
//    ///
//    /// \param key The key to cache.
//    /// \param entry The value to cache.
//    void put(const KeyType& key, const ValueType& entry)
//    {
//        doPut(key, std::make_shared<ValueType>(entry));
//    }
//
//    /// \brief Remove a value from the cache.
//    /// \param key The key to remove.
//    /// \returns true if the key was removed.
//    void remove(const KeyType& key)
//    {
//        doRemove(key);
//        onRemoved.notify(this, key);
//    }
//
//    /// \returns the number of elements in the cache.
//    std::size_t size() const
//    {
//        return doSize();
//    }
//
//    /// \brief Clear the entire cache.
//    void clear()
//    {
//        doClear();
//        onCleared.notify(this);
//    }
//
//    /// \brief An event called when a value is cached.
//    mutable ofEvent<const std::pair<KeyType, std::shared_ptr<ValueType>>> onPut;
//
//    /// \brief An event called when a value is removed.
//    mutable ofEvent<const KeyType> onRemoved;
//
//    /// \brief An event called when a value is gotten and a cached value is returned.
//    mutable ofEvent<const KeyType> onHit;
//
//    /// \brief An event called when a value is gotten and a cached value is not returned.
//    mutable ofEvent<const KeyType> onMiss;
//
//    /// \brief An event called when the cache is cleared.
//    mutable ofEvent<void> onCleared;
//
//protected:
//    virtual bool doHas(const KeyType& key) const = 0;
//    virtual std::shared_ptr<ValueType> doGet(const KeyType& key) const = 0;
//    virtual void doPut(const KeyType& key, std::shared_ptr<ValueType> entry) = 0;
//    virtual void doRemove(const KeyType& key) = 0;
//    virtual std::size_t doSize() const = 0;
//    virtual void doClear() = 0;
//
//};
//

} } // namespace ofx::Cache