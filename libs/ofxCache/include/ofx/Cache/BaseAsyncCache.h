//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofx/Cache/BaseCache.h"


namespace ofx {
namespace Cache {


enum class RequestState
{
    /// \brief The request is unknown.
    UNKNOWN,
    /// \brief The request is idle.
    IDLE,
    /// \brief The request is starting.
    STARTING,
    /// \brief The request is starting.
    RUNNING,
    /// \brief The request is being cancelled.
    CANCELLING,
    /// \brief The request is finished.
    FINISHED
};


template<typename KeyType>
class RequestFailedArgs
{
public:
    RequestFailedArgs(const KeyType& key, const std::string& error):
    _key(key),
    _error(error)
    {
    }

    virtual ~RequestFailedArgs()
    {
    }

    KeyType key() const
    {
        return _key;
    }

    std::string error() const
    {
        return _error;
    }
private:
    KeyType _key;
    std::string _error;

};


template<typename KeyType, typename ValueType>
class RequestCompleteArgs
{
public:
    RequestCompleteArgs(const KeyType& key,
                        std::shared_ptr<ValueType> value,
                        CacheStatus status):
    _key(key),
    _value(value),
    _status(status)
    {
    }

    virtual ~RequestCompleteArgs()
    {
    }

    KeyType key() const
    {
        return _key;
    }

    std::shared_ptr<ValueType> value() const
    {
        return _value;
    }

    CacheStatus status() const
    {
        return _status;
    }

private:
    KeyType _key;
    std::shared_ptr<ValueType> _value;
    CacheStatus _status;
};



/// \brief A simple base cache type.
///
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
template<typename KeyType, typename ValueType>
class BaseAsyncCache: public BaseCache<KeyType, ValueType>
{
public:
    typedef RequestCompleteArgs<KeyType, ValueType> _RequestCompleteArgs;
    typedef RequestFailedArgs<KeyType> _RequestFailedArgs;

    /// \brief Destroy the BaseCache.
    virtual ~BaseAsyncCache()
    {
    }

    /// \brief Request a value by its key.
    ///
    /// The value will be returned via the onRequested event.
    ///
    /// \param key The key to request.
    void request(const KeyType& key)
    {
        auto result = this->get(key);

        if (result != nullptr)
        {
            RequestCompleteArgs<KeyType, ValueType> args(key, result, CacheStatus::CACHE_HIT);
            onRequestComplete.notify(this, args);
        }
        else
        {
            doRequest(key);
        }
    }

    /// \brief Cancel any outstanding request for the given key.
    ///
    /// If there is no request for the given key, the request will be ignored.
    /// This will attempt to cancel both queued and active requests.
    ///
    /// \param key The key to cancel.
    void cancelRequest(const KeyType& key)
    {
        doCancelRequest(key);
    }

    /// \brief Cancel any outstanding queued request for the given key.
    ///
    /// If there is no queued request for the given key, the request will be
    /// ignored.
    ///
    /// \param key The key to cancel.
    void cancelQueuedRequest(const KeyType& key)
    {
        doCancelQueuedRequest(key);
    }

    /// \brief Get the progress 0 - 1 for the given key.
    ///
    /// For items that are not loaded or have no progress, return 0.  For items
    /// that are in progress, return a value 0 - 1.  For items already in the
    /// cache, return 1.
    ///
    /// \param key The key to query.
    /// \returns the progress on a scale of 0 - 1.
    float requestProgress(const KeyType& key) const
    {
        return doRequestProgress(key);
    }

    /// \brief Get the state of a given request.
    /// \param key The key to query.
    /// \returns the state of the request.
    RequestState requestState(const KeyType& key) const
    {
        return doRequestState(key);
    }

    /// \brief An event that is called when the a value is requested.
    mutable ofEvent<const RequestCompleteArgs<KeyType, ValueType>> onRequestComplete;

    /// \brief This event is called when a resource request is cancelled.
    mutable ofEvent<const KeyType> onRequestCancelled;

    /// \brief This event is called when a requested resource fails.
    ///
    /// The key will be paired with an error.
    mutable ofEvent<const RequestFailedArgs<KeyType>> onRequestFailed;

protected:
    virtual void doRequest(const KeyType& key) = 0;
    virtual void doCancelRequest(const KeyType& key) = 0;
    virtual void doCancelQueuedRequest(const KeyType& key) = 0;
    virtual float doRequestProgress(const KeyType& key) const = 0;
    virtual RequestState doRequestState(const KeyType& key) const = 0;

};


} } // namespace ofx::Cache
