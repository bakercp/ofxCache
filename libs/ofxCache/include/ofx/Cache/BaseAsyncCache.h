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


#include "ofx/Cache/BaseCache.h"


namespace ofx {
namespace Cache {



/// \brief A simple base cache type.
///
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
template<typename KeyType, typename ValueType>
class BaseAsyncCache: public BaseCache<KeyType, ValueType>
{
public:
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
        auto result = this->doGet(key);

        if (result != nullptr)
        {
            onRequestComplete.notify(this, std::make_tuple(key, result, BaseCache<KeyType, ValueType>::CacheStatus::CACHE_HIT));
        }
        else
        {
            doRequest(key);
        }
    }

    /// \brief An event that is called when the a value is requested.
    mutable ofEvent<const std::tuple<KeyType, std::shared_ptr<ValueType>, typename BaseCache<KeyType, ValueType>::CacheStatus>> onRequestComplete;

protected:
    virtual void doRequest(const KeyType& key) = 0;

};


} } // namespace ofx::Cache
