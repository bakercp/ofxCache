//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


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
class BaseReadableHTTPStore: public BaseReadableURIStore<KeyType, ValueType, HTTP::ClientExchange>
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
    return response->getStatus() == HTTP::Response::HTTP_OK;
}


template<typename KeyType, typename ValueType>
std::shared_ptr<ValueType> BaseReadableHTTPStore<KeyType, ValueType>::doGet(const KeyType& key)
{
    HTTP::Client client;
    HTTP::Context context(_settings);
    HTTP::GetRequest request(this->keyToURI(key));
    auto response = client.execute(context, request);
    HTTP::ClientExchange transaction(context, request, *response.get());
    return this->rawToValue(transaction);
}


} } // namespace ofx::Cache
