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
