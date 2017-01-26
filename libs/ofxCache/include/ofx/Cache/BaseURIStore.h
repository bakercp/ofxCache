//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofx/Cache/BaseStore.h"


namespace ofx {
namespace Cache {


template<typename KeyType>
class BaseURIStore
{
public:
    virtual ~BaseURIStore()
    {
    }

    /// \brief Convert the given key to a URI.
    /// \param key The key type to convert.
    /// \returns a URI to access the value.
    virtual std::string keyToURI(const KeyType& key) const = 0;

};



/// \brief A read-only URI store.
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
/// \tparam BufferType The raw buffered data type. Usually extends ofBuffer or similar.
template<typename KeyType, typename ValueType, typename RawType>
class BaseReadableURIStore:
    public virtual BaseURIStore<KeyType>,
    public virtual BaseReadableStore<KeyType, ValueType>
{
public:
    /// \brief Destroy the BaseStore.
    virtual ~BaseReadableURIStore()
    {
    }

protected:

    /// \brief Convert a stored buffer to a stored value.
    /// \param A reference to the raw type.
    virtual std::shared_ptr<ValueType> rawToValue(RawType& rawType) = 0;
    
};



/// \brief A read/write URI store.
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
/// \tparam BufferType The raw buffered data type. Usually extends ofBuffer or similar.
template<typename KeyType, typename ValueType, typename RawType>
class BaseWritableURIStore:
    public virtual BaseURIStore<KeyType>,
    public virtual BaseWritableStore<KeyType, ValueType>
{
public:
    /// \brief Destroy the BaseWritableURIStore.
    virtual ~BaseWritableURIStore()
    {
    }

protected:
    /// \brief Convert a stored buffer to a stored value.
    /// \param A reference to the raw type.
    virtual std::shared_ptr<RawType> valueToRaw(ValueType& valueType) = 0;

    virtual void doAdd(const KeyType& key, std::shared_ptr<ValueType> entry) = 0;
    virtual void doRemove(const KeyType& key) = 0;

};


} } // namespace ofx::Cache
