//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#include "ofApp.h"


static std::vector<std::string> keys = {
    "https://httpbin.org/redirect-to?url=https://httpbin.org/image",
    "https://httpbin.org/image",
    "https://httpbin.org/image/png",
    "https://httpbin.org/image/jpeg",
    "https://httpbin.org/image/webp"
};


void ofApp::setup()
{
    // This is the memory cache.
    cache = std::make_unique<ofxCache::LRUMemoryCache<std::string, ofImage>>(keys.size());

    // This is the HTTP store.
    cache->setChild<SimpleReadableHTTPPixelStore>();
}

void ofApp::draw()
{
    int x = 0;
    int y = 0;

    for (auto key: keys)
    {
        auto image = cache->get(key);
        if (image)
        {
            image->draw(x, y);
            y += image->getHeight();
        }
    }

}
