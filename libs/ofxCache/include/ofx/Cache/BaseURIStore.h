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


#include "ofx/Cache/BaseStore.h"


namespace ofx {
namespace Cache {


/// \brief A read-only URI store.
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
/// \tparam BufferType The raw buffered data type. Usually extends ofBuffer or similar.
template<typename KeyType, typename ValueType, typename RawType>
class BaseReadableURIStore: public BaseReadableStore<KeyType, ValueType>
{
public:
    /// \brief Destroy the BaseStore.
    virtual ~BaseReadableURIStore()
    {
    }

    /// \brief Convert the given key to a URI.
    /// \param key The key type to convert.
    /// \returns a URI to access the value.
    virtual std::string keyToURI(const KeyType& key) const = 0;
    
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
    public BaseReadableURIStore<KeyType, ValueType, RawType>,
    public virtual BaseWritableStore<KeyType, ValueType>
{
public:
    /// \brief Destroy the BaseWritableURIStore.
    virtual ~BaseWritableURIStore()
    {
    }

};


} } // namespace ofx::Cache
