//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier: MIT
//


#pragma once


namespace ofx {
namespace Cache {


template<typename KeyType, typename ValueType>
class AyncResourceLoader:
{
public:
    virtual ~AyncResourceLoader();

};


template<typename KeyType, typename ValueType>
AyncResourceLoader<KeyType, ValueType>::~AyncResourceLoader()
{
}


} } // namespace ofx::Cache

