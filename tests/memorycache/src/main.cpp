#include "ofxCache.h"
#include "ofxUnitTests.h"

class ofApp: public ofxUnitTestsApp
{
    void run()
    {
        testClear();
        testCacheSize0();
        testCacheSize1();
        testCacheSize2();
        testCacheSizeN();
        testDuplicateAdd();
        testUpdate();


        std::cout << sizeof(Poco::Int64) << " " << std::numeric_limits<Poco::Int64>::max() << std::endl;
        std::cout << sizeof(Poco::UInt64) << " " << std::numeric_limits<Poco::UInt64>::max() << std::endl;

        std::cout << sizeof(uint64_t) << " " << std::numeric_limits<uint64_t>::max() << std::endl;
        std::cout << sizeof(int64_t) << " " << std::numeric_limits<int64_t>::max() << std::endl;
        std::cout << sizeof(int32_t) << " " << std::numeric_limits<int32_t>::max() << std::endl;
        std::cout << sizeof(uint32_t) << " " << std::numeric_limits<uint32_t>::max() << std::endl;


    }


    void testClear()
    {
        std::string testName = "testClear";
        ofxCache::LRUMemoryCache<int, int> aCache(3);
        test_eq(aCache.size(), 0, testName);

        aCache.put(1, 2);
        aCache.put(3, 4);
        aCache.put(5, 6);
        test_eq(aCache.size(), 3, testName);
        //test_eq(aCache.getAllKeys().size() == 3);
        test(aCache.has(1), testName);
        test(aCache.has(3), testName);
        test(aCache.has(5), testName);
        test_eq(*aCache.get(1), 2, testName);
        test_eq(*aCache.get(3), 4, testName);
        test_eq(*aCache.get(5), 6, testName);
        aCache.clear();
        test(!aCache.has(1), testName);
        test(!aCache.has(3), testName);
        test(!aCache.has(5), testName);
    }


    void testCacheSize0()
    {
        // cache size 0 is illegal
        try
        {
            ofxCache::LRUMemoryCache<int, int> aCache(0);
            test_eq(0, 1, "Testing init (this will be a failure).");
        }
        catch (Poco::InvalidArgumentException&)
        {
        }
    }


    void testCacheSize1()
    {
        std::string testName = "testCacheSize1";
        ofxCache::LRUMemoryCache<int, int> aCache(1);
        aCache.put(1, 2);
        test(aCache.has(1), testName);
        test(*aCache.get(1), testName);

        aCache.put(3, 4); // replaces 1
        test(!aCache.has(1), testName);
        test(aCache.has(3), testName);
        test_eq(*aCache.get(3), 4, testName);

        aCache.put(5, 6);
        test(!aCache.has(1), testName);
        test(!aCache.has(3), testName);
        test(aCache.has(5), testName);
        test_eq(*aCache.get(5), 6, testName);

        aCache.remove(5);
        test(!aCache.has(5), testName);

        // removing illegal entries should work too
        aCache.remove(666);
    }

    void testCacheSize2()
    {
        // 3-1 represents the cache sorted by pos, elements get replaced at the end of the list
        // 3-1|5 -> 5 gets removed
        ofx::LRUCache<int, int> aCache(2);
        aCache.add(1, 2); // 1
        test(aCache.has(1), "");
        test_eq(*aCache.get(1), 2, "");

        aCache.add(3, 4); // 3-1
        test(aCache.has(1), "");
        test(aCache.has(3), "");
        test_eq(*aCache.get(1), 2, ""); // 1-3
        test_eq(*aCache.get(3), 4, ""); // 3-1

        aCache.add(5, 6); // 5-3|1
        test(!aCache.has(1), "");
        test(aCache.has(3), "");
        test(aCache.has(5), "");
        test_eq(*aCache.get(5), 6, "");  // 5-3
        test_eq(*aCache.get(3), 4, "");  // 3-5

        // test remove from the end and the beginning of the list
        aCache.remove(5); // 3
        test(!aCache.has(5), "");
        test_eq(*aCache.get(3), 4, "");  // 3
        aCache.add(5, 6); // 5-3
        test_eq(*aCache.get(3), 4, "");  // 3-5
        aCache.remove(3); // 5
        test(!aCache.has(3), "");
        test_eq(*aCache.get(5), 6, "");  // 5

        // removing illegal entries should work too
        aCache.remove(666);

        aCache.clear();
        test(!aCache.has(5), "");
    }

    void testCacheSizeN()
    {
        // 3-1 represents the cache sorted by pos, elements get replaced at the end of the list
        // 3-1|5 -> 5 gets removed
        ofxCache::LRUMemoryCache<int, int> aCache(3);
        aCache.put(1, 2); // 1
        test(aCache.has(1),"");
        test_eq(*aCache.get(1), 2, "");

        aCache.put(3, 4); // 3-1
        test(aCache.has(1), "");
        test(aCache.has(3), "");
        test_eq(*aCache.get(1), 2, ""); // 1-3
        test_eq(*aCache.get(3), 4, ""); // 3-1

        aCache.put(5, 6); // 5-3-1
        test(aCache.has(1), "");
        test(aCache.has(3), "");
        test(aCache.has(5), "");
        test_eq(*aCache.get(5), 6, "");  // 5-3-1
        test_eq(*aCache.get(3), 4, "");  // 3-5-1

        aCache.put(7, 8); // 7-5-3|1
        test(!aCache.has(1), "");
        test(aCache.has(7), "");
        test(aCache.has(3), "");
        test(aCache.has(5), "");
        test_eq(*aCache.get(5), 6, "");  // 5-7-3
        test_eq(*aCache.get(3), 4, "");  // 3-5-7
        test_eq(*aCache.get(7), 8, "");  // 7-3-5

        // test remove from the end and the beginning of the list
        aCache.remove(5); // 7-3
        test(!aCache.has(5), "");
        test_eq(*aCache.get(3), 4, "");  // 3-7
        aCache.put(5, 6); // 5-3-7
        test_eq(*aCache.get(7), 8, "");  // 7-5-3
        aCache.remove(7); // 5-3
        test(!aCache.has(7), "");
        test(aCache.has(3), "");
        test_eq(*aCache.get(5), 6, "");  // 5-3

        // removing illegal entries should work too
        aCache.remove(666);

        aCache.clear();
        test(!aCache.has(5), "");
        test(!aCache.has(3), "");
    }


    void testDuplicateAdd()
    {
        ofxCache::LRUMemoryCache<int, int> aCache(3);
        aCache.put(1, 2); // 1
        test(aCache.has(1), "");
        test_eq(*aCache.get(1), 2, "");
        aCache.put(1, 3);
        test(aCache.has(1), "");
        test_eq(*aCache.get(1),  3, "");
    }
    
    
    void testUpdate()
    {
        std::string testName = "testUpdate";
        putCnt = 0;
        removeCnt = 0;
        hitCnt = 0;
        missCnt = 0;
        ofxCache::LRUMemoryCache<int, int> aCache(3);

        auto lAdd = aCache.onPut.newListener(this, &ofApp::onPut);
        auto lRemove = aCache.onRemoved.newListener(this, &ofApp::onRemove);
        auto lHit = aCache.onHit.newListener(this, &ofApp::onHit);
        auto lMiss = aCache.onMiss.newListener(this, &ofApp::onMiss);

        aCache.put(1, 2); // 1 ,one add event
        test_eq(putCnt, 1, testName);
        test_eq(removeCnt, 0, testName);
        
        test(aCache.has(1), testName);
        test_eq(*aCache.get(1), 2, testName);

        test_eq(aCache.get(1000), std::shared_ptr<int>(), testName);
        test_eq(missCnt, 1, testName);
        test_eq(hitCnt, 1, testName);
        aCache.put(1, 3);

        test_eq(putCnt, 2, testName);

        test_eq(removeCnt, 0, testName);
        test(aCache.has(1), testName);
        test_eq(*aCache.get(1), 3, testName);
    }

    void onHit(const int& args)
    {
        ++hitCnt;
    }

    void onMiss(const int& args)
    {
        ++missCnt;
    }

    void onPut(const std::pair<int, std::shared_ptr<int>>& args)
    {
        ++putCnt;
    }
    
    
    void onRemove(const int& args)
    {
        ++removeCnt;
    }

    int putCnt = 0;
    int removeCnt = 0;

    int hitCnt = 0;
    int missCnt = 0;
};


#include "ofAppNoWindow.h"
#include "ofAppRunner.h"


int main()
{
	ofInit();
	auto window = std::make_shared<ofAppNoWindow>();
	auto app = std::make_shared<ofApp>();
	ofRunApp(window, app);
	return ofRunMainLoop();
}