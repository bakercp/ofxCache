//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier: MIT
//


#pragma once


#include "ofx/Cache/BaseURIStore.h"
#include "ofx/HTTP/ClientSessionSettings.h"
#include "ofx/HTTP/GetRequest.h"
#include "ofx/HTTP/Client.h"


namespace ofx {
namespace Cache {


/// \brief A simple File cache.
template <typename KeyType, typename ValueType>
class BaseReadableFileStore: public virtual BaseReadableURIStore<KeyType, ValueType, ofBuffer>
{
public:
    /// \brief Destroy the BaseReadableFileStore.
    virtual ~BaseReadableFileStore() { }

protected:

//    virtual bool doHas(const KeyType& key) const = 0;
//    virtual std::shared_ptr<ValueType> doGet(const KeyType& key) = 0;

    bool doHas(const KeyType& key) const override
    {
        return ofFile(this->keyToURI(key)).exists();
    }

    std::shared_ptr<ValueType> doGet(const KeyType& key) override
    {
        return this->rawToValue(ofLoadBufferFromFile(this->keyToURI(key)));
    }

};


/// \brief A simple File cache.
template <typename KeyType, typename ValueType>
class BaseWritableFileStore: public virtual BaseWritableURIStore<KeyType, ValueType, ofBuffer>
{
public:
    /// \brief Destroy the BaseReadableFileStore.
    virtual ~BaseWritableFileStore() { }

protected:
    void doAdd(const KeyType& key, std::shared_ptr<ValueType> entry) override
    {
        std::string uri = this->keyToURI(key);
        std::shared_ptr<ofBuffer> buffer = this->valueToRaw(*entry.get());

        if (!ofBufferToFile(uri, *buffer))
        {
            ofLogError("BaseWritableFileStore::doAdd") << "Failed to add " << uri;
        }
    }

    void doRemove(const KeyType& key) override
    {
        ofFile(this->keyToURI(key)).remove();
    }

};



} } // namespace ofx::Cache
