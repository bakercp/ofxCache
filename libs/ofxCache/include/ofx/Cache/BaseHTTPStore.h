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
#include "ofx/HTTP/HeadRequest.h"
#include "ofx/HTTP/Client.h"


namespace ofx {
namespace Cache {


/// \brief A simple HTTP cache.
template <typename KeyType, typename ValueType>
class BaseReadableHTTPStore: public BaseReadableURIStore<KeyType, ValueType, HTTP::ClientTransaction>
{
public:
    BaseReadableHTTPStore(const HTTP::ClientSessionSettings& settings = HTTP::ClientSessionSettings());

    /// \brief Destroy the BaseReadableHTTPStore.
    virtual ~BaseReadableHTTPStore();

protected:
    virtual bool doHas(const KeyType& key) const override;
    std::shared_ptr<ValueType> doGet(const KeyType& key) override;

private:
    HTTP::ClientSessionSettings _settings;

};


template<typename KeyType, typename ValueType>
BaseReadableHTTPStore<KeyType, ValueType>::BaseReadableHTTPStore(const HTTP::ClientSessionSettings& settings):
    _settings(settings)
{
}


template<typename KeyType, typename ValueType>
BaseReadableHTTPStore<KeyType, ValueType>::~BaseReadableHTTPStore()
{
}


template<typename KeyType, typename ValueType>
bool BaseReadableHTTPStore<KeyType, ValueType>::doHas(const KeyType& key) const
{
    HTTP::Client client;
    HTTP::Context context(_settings);
    HTTP::HeadRequest request(this->keyToURI(key));
    auto response = client.execute(context, request);
    HTTP::HTTPUtils::consume(response->stream());
    return response->getStatus() == HTTP::BaseResponse::HTTP_OK;
}


template<typename KeyType, typename ValueType>
std::shared_ptr<ValueType> BaseReadableHTTPStore<KeyType, ValueType>::doGet(const KeyType& key)
{
    HTTP::Client client;
    HTTP::Context context(_settings);
    HTTP::GetRequest request(this->keyToURI(key));
    auto response = client.execute(context, request);
    HTTP::ClientTransaction transaction(request, *response.get(), context);
    return this->rawToValue(transaction);
}


} } // namespace ofx::Cache
