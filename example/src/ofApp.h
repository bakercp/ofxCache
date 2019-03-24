//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofMain.h"
#include "ofxCache.h"


class ofApp: public ofBaseApp
{
public:
    void setup() override;

    ofxCache::LRUMemoryCache<std::string, std::string> memCache;

};
