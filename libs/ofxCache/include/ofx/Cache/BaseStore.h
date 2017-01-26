//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofEvents.h"


namespace ofx {
namespace Cache {


/// \brief A read-only data store.
///
/// These consist of safe (aka "nullipotent") methods, meaning that calling the
/// methods produces no side-effects in the underlying data.
///
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
template<typename KeyType, typename ValueType>
class BaseReadableStore
{
public:
    /// \brief Destroy the BaseStore.
    virtual ~BaseReadableStore()
    {
    }

    /// \brief Determine if the given value is available from this cache.
    ///
    /// \param key The key to check.
    /// \returns true if this cache node has the requested value.
    virtual bool has(const KeyType& key) const;

    /// \brief Get a value by its key.
    ///
    /// This method is synchronous and will block until the get operation is
    /// complete.
    ///
    /// \param key The key to get.
    /// \returns std::shared_ptr<ValueType> or nullptr if the cache missed.
    virtual std::shared_ptr<ValueType> get(const KeyType& key);

    /// \brief Event called when has is called.
    mutable ofEvent<const KeyType> onHas;

    /// \brief Event called when get is called.
    ofEvent<const KeyType> onGet;

protected:
    virtual bool doHas(const KeyType& key) const = 0;
    virtual std::shared_ptr<ValueType> doGet(const KeyType& key) = 0;

};


template<typename KeyType, typename ValueType>
bool BaseReadableStore<KeyType, ValueType>::has(const KeyType& key) const
{
    onHas.notify(this, key);
    return doHas(key);
}


template<typename KeyType, typename ValueType>
std::shared_ptr<ValueType> BaseReadableStore<KeyType, ValueType>::get(const KeyType& key)
{
    onGet.notify(this, key);
    return doGet(key);
}


/// \brief A writable data store.
///
/// These consist of safe (aka "nullipotent") methods, meaning that calling the
/// methods produces no side-effects in the underlying data.
///
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
template<typename KeyType, typename ValueType>
class BaseWritableStore: public virtual BaseReadableStore<KeyType, ValueType>
{
public:
    /// \brief Destroy the BaseWritableStore.
    virtual ~BaseWritableStore()
    {
    }

    /// \brief Cache a value.
    ///
    /// Adding a value in the cache will overwrite any existing value for the
    /// given key.
    ///
    /// By default the add operation is synchronous.
    ///
    /// \param key The key to cache.
    /// \param entry The value to cache.
    void add(const KeyType& key, std::shared_ptr<ValueType> entry);

    /// \copydoc add()
    void add(const KeyType& key, const ValueType& entry);

    /// \brief Cache a value.
    ///
    /// Adding a value in the cache will overwrite any existing value for the
    /// given key.
    ///
    /// By default the update operation is synchronous.
    ///
    /// \param key The key to cache.
    /// \param entry The value to cache.
    void update(const KeyType& key, std::shared_ptr<ValueType> entry);

    /// \copydoc update()
    void update(const KeyType& key, const ValueType& entry);

    /// \brief Remove a value from the cache.
    ///
    /// This will only remove the value from this cache node. The remove
    /// operation will not affect child nodes.
    ///
    /// If the key does not exist, the remove will be ignored.
    ///
    /// By default the put operation is synchronous.
    ///
    /// \param key The key to remove.
    void remove(const KeyType& key);

    /// \brief Event called when an value is added.
    ofEvent<const std::pair<KeyType, std::shared_ptr<ValueType>>> onAdd;

    /// \brief Event called when an existing value is updated.
    ofEvent<const std::pair<KeyType, std::shared_ptr<ValueType>>> onUpdate;

    /// \brief Event called when an existing value is removed.
    ofEvent<const KeyType> onRemove;

protected:
    virtual void doAdd(const KeyType& key, std::shared_ptr<ValueType> entry) = 0;
    
    virtual void doUpdate(const KeyType& key, std::shared_ptr<ValueType> entry)
    {
        doAdd(key, entry);
    }

    virtual void doRemove(const KeyType& key) = 0;

};


template<typename KeyType, typename ValueType>
void BaseWritableStore<KeyType, ValueType>::add(const KeyType& key,
                                                std::shared_ptr<ValueType> entry)
{
    remove(key);
    onAdd.notify(this, std::make_pair(key, entry));
    doAdd(key, entry);
}


template<typename KeyType, typename ValueType>
void BaseWritableStore<KeyType, ValueType>::add(const KeyType& key,
                                                const ValueType& entry)
{
    add(key, std::make_shared<ValueType>(entry));
}


template<typename KeyType, typename ValueType>
void BaseWritableStore<KeyType, ValueType>::update(const KeyType& key,
                                                   std::shared_ptr<ValueType> entry)
{
    auto args = std::make_pair(key, entry);

    if (this->has(key))
    {
        onUpdate.notify(this, args);
        doUpdate(key, entry);
    }
    else
    {
        onAdd.notify(this, args);
        doAdd(key, entry);
    }
}


template<typename KeyType, typename ValueType>
void BaseWritableStore<KeyType, ValueType>::update(const KeyType& key,
                                                   const ValueType& entry)
{
    update(key, std::make_shared<ValueType>(entry));
}


template<typename KeyType, typename ValueType>
void BaseWritableStore<KeyType, ValueType>::remove(const KeyType& key)
{
    if (this->has(key))
    {
        onRemove.notify(this, key);
        doRemove(key);
    }
}


} } // namespace ofx::Cache
