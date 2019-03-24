//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofMain.h"
#include "ofxHTTP.h"
#include "ofxCache.h"
#include "ofx/Cache/BaseHTTPStore.h"



class SimpleReadableHTTPPixelStore: public ofxCache::BaseReadableHTTPStore<std::string, ofImage>
{
public:
    //using ofxCache::BaseReadableHTTPStore<std::string, ofImage>::BaseReadableHTTPStore;

    virtual ~SimpleReadableHTTPPixelStore()
    {
    }

    std::string keyToURI(const std::string& key) const
    {
        return key;
    }

    std::shared_ptr<ofImage> rawToValue(ofxHTTP::ClientExchange& rawType)
    {
        return nullptr;
//        Poco::Net::MediaType mediaType(rawType.response().getContentType());
//
//        if (rawType.response().getStatus() == ofxHTTP::Response::HTTP_OK && mediaType.matches("image"))
//        {
//            ofPixels pixels;
//
//            if (ofLoadImage(pixels, ofBuffer(rawType.response().stream())))
//            {
//                return std::make_shared<ofImage>(pixels);
//            }
//            else
//            {
//                return nullptr;
//            }
//        }
//        else return nullptr;
    }
    
};



class ofApp: public ofBaseApp
{
public:
    void setup();
    void draw();

    std::unique_ptr<ofxCache::LRUMemoryCache<std::string, ofImage>> cache;


};
