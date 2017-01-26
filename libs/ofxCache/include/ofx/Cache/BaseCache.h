//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofEvent.h"
#include "ofLog.h"
#include "ofx/Cache/BaseStore.h"


namespace ofx {
namespace Cache {


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
template<typename KeyType, typename ValueType, typename ChildKeyType = KeyType, typename ChildValueType = ValueType>
class BaseCache: public BaseWritableStore<KeyType, ValueType>
{
public:
    typedef BaseCache<ChildKeyType, ChildValueType> ChildStore;

    /// \brief Create a default BaseCache.
    BaseCache(std::unique_ptr<ChildStore> childStore = nullptr)
    {
        setChild(std::move(childStore));
    }

    /// \brief Destroy the BaseCache.
    virtual ~BaseCache()
    {
    }

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
    std::shared_ptr<ValueType> get(const KeyType& key) override
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

    /// \returns the number of elements in this cache node cache.
    std::size_t size()
    {
        return doSize();
    }

    /// \brief Clear all values in this cache node.
    void clear()
    {
        onClear.notify(this);
        doClear();
    }

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
    StoreType* setChild(std::unique_ptr<StoreType> store)
    {
        static_assert(std::is_base_of<ChildStore, StoreType>(), "StoreType must be derived from BaseStore.");
        // Get a raw pointer to the node for later.
        StoreType* pNode = store.get();

        // Take ownership of the node.
        _childStore = std::move(store);

        if (_childStore != nullptr)
        {
            ofAddListener(_childStore->onAdd, this, &BaseCache::onChildAdd);
            ofAddListener(_childStore->onUpdate, this, &BaseCache::onChildUpdate);
            ofAddListener(_childStore->onRemove, this, &BaseCache::onChildRemove);
            ofAddListener(_childStore->onHas, this, &BaseCache::onChildHas);
            ofAddListener(_childStore->onGet, this, &BaseCache::onChildGet);
            ofAddListener(_childStore->onClear, this, &BaseCache::onChildClear);
        }

        return pNode;
    }

    
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
    StoreType* setChild(Args&&... args)
    {
        return setChild(std::make_unique<StoreType>(std::forward<Args>(args)...));
    }

    /// \brief Release ownership of a child Cache.
    /// \param element The cache to release.
    /// \returns a std::unique_ptr<BaseCache<KeyType, ValueType>> to the child or nullptr if none.
    std::unique_ptr<ChildStore> removeChild()
    {
        if (_childStore != nullptr)
        {
            ofRemoveListener(_childStore->onAdd, this, &BaseCache::onChildAdd);
            ofRemoveListener(_childStore->onUpdate, this, &BaseCache::onChildUpdate);
            ofRemoveListener(_childStore->onRemove, this, &BaseCache::onChildRemove);
            ofRemoveListener(_childStore->onHas, this, &BaseCache::onChildHas);
            ofRemoveListener(_childStore->onGet, this, &BaseCache::onChildGet);
            ofRemoveListener(_childStore->onClear, this, &BaseCache::onChildClear);
        }

        return std::move(_childStore);
    }

    /// \brief An event called when the cache is cleared.
    ofEvent<void> onClear;
    
protected:
    bool onChildAdd(const std::pair<KeyType, std::shared_ptr<ValueType>>& evt)
    {
        return doOnChildAdd(evt);
    }

    bool onChildUpdate(const std::pair<KeyType, std::shared_ptr<ValueType>>& evt)
    {
        return doOnChildUpdate(evt);
    }

    bool onChildRemove(const KeyType& evt)
    {
        return doOnChildRemove(evt);
    }

    bool onChildHas(const KeyType& evt)
    {
        return doOnChildHas(evt);
    }

    bool onChildGet(const KeyType& evt)
    {
        return doOnChildGet(evt);
    }

    bool onChildClear()
    {
        return doOnChildClear();
    }

    virtual std::size_t doSize() = 0;
    virtual void doClear() = 0;

    virtual bool doOnChildAdd(const std::pair<KeyType, std::shared_ptr<ValueType>>& evt)
    {
        ofLogVerbose("BaseCache::doOnChildAdd") << "Not implmented.";
        return false;
    }

    virtual bool doOnChildUpdate(const std::pair<KeyType, std::shared_ptr<ValueType>>& evt)
    {
        ofLogVerbose("BaseCache::doOnChildUpdate") << "Not implmented.";
        return false;
    }

    virtual bool doOnChildRemove(const KeyType& evt)
    {
        ofLogVerbose("BaseCache::doOnChildRemove") << "Not implmented.";
        return false;
    }

    virtual bool doOnChildHas(const KeyType& evt)
    {
        ofLogVerbose("BaseCache::doOnChidoOnChildHasldAdd") << "Not implmented.";
        return false;
    }

    virtual bool doOnChildGet(const KeyType& evt)
    {
        ofLogVerbose("BaseCache::doOnChildGet") << "Not implmented.";
        return false;
    }

    virtual bool doOnChildClear()
    {
        ofLogVerbose("BaseCache::doOnChildClear") << "Not implmented.";
        return false;
    }

private:
    std::unique_ptr<ChildStore> _childStore = nullptr;

};



} } // namespace ofx::Cache
