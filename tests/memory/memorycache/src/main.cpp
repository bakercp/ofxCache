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
        ofxTestEq(aCache.size(), 0, testName);

        aCache.add(1, 2);
        aCache.add(3, 4);
        aCache.add(5, 6);
        ofxTestEq(aCache.size(), 3, testName);
        //ofxTestEq(aCache.getAllKeys().size() == 3);
        ofxTest(aCache.has(1), testName);
        ofxTest(aCache.has(3), testName);
        ofxTest(aCache.has(5), testName);
        ofxTestEq(*aCache.get(1), 2, testName);
        ofxTestEq(*aCache.get(3), 4, testName);
        ofxTestEq(*aCache.get(5), 6, testName);
        aCache.clear();
        ofxTest(!aCache.has(1), testName);
        ofxTest(!aCache.has(3), testName);
        ofxTest(!aCache.has(5), testName);
    }


    void testCacheSize0()
    {
        // cache size 0 is illegal
        try
        {
            ofxCache::LRUMemoryCache<int, int> aCache(0);
            ofxTestEq(0, 1, "Testing init (this will be a failure).");
        }
        catch (Poco::InvalidArgumentException&)
        {
        }
    }


    void testCacheSize1()
    {
        std::string testName = "testCacheSize1";
        ofxCache::LRUMemoryCache<int, int> aCache(1);
        aCache.add(1, 2);
        ofxTest(aCache.has(1), testName);
        ofxTest(*aCache.get(1), testName);

        aCache.add(3, 4); // replaces 1
        ofxTest(!aCache.has(1), testName);
        ofxTest(aCache.has(3), testName);
        ofxTestEq(*aCache.get(3), 4, testName);

        aCache.add(5, 6);
        ofxTest(!aCache.has(1), testName);
        ofxTest(!aCache.has(3), testName);
        ofxTest(aCache.has(5), testName);
        ofxTestEq(*aCache.get(5), 6, testName);

        aCache.remove(5);
        ofxTest(!aCache.has(5), testName);

        // removing illegal entries should work too
        aCache.remove(666);
    }

    void testCacheSize2()
    {
        // 3-1 represents the cache sorted by pos, elements get replaced at the end of the list
        // 3-1|5 -> 5 gets removed
        ofx::LRUCache<int, int> aCache(2);
        aCache.add(1, 2); // 1
        ofxTest(aCache.has(1), "");
        ofxTestEq(*aCache.get(1), 2, "");

        aCache.add(3, 4); // 3-1
        ofxTest(aCache.has(1), "");
        ofxTest(aCache.has(3), "");
        ofxTestEq(*aCache.get(1), 2, ""); // 1-3
        ofxTestEq(*aCache.get(3), 4, ""); // 3-1

        aCache.add(5, 6); // 5-3|1
        ofxTest(!aCache.has(1), "");
        ofxTest(aCache.has(3), "");
        ofxTest(aCache.has(5), "");
        ofxTestEq(*aCache.get(5), 6, "");  // 5-3
        ofxTestEq(*aCache.get(3), 4, "");  // 3-5

        // test remove from the end and the beginning of the list
        aCache.remove(5); // 3
        ofxTest(!aCache.has(5), "");
        ofxTestEq(*aCache.get(3), 4, "");  // 3
        aCache.add(5, 6); // 5-3
        ofxTestEq(*aCache.get(3), 4, "");  // 3-5
        aCache.remove(3); // 5
        ofxTest(!aCache.has(3), "");
        ofxTestEq(*aCache.get(5), 6, "");  // 5

        // removing illegal entries should work too
        aCache.remove(666);

        aCache.clear();
        ofxTest(!aCache.has(5), "");
    }

    void testCacheSizeN()
    {
        // 3-1 represents the cache sorted by pos, elements get replaced at the end of the list
        // 3-1|5 -> 5 gets removed
        ofxCache::LRUMemoryCache<int, int> aCache(3);
        aCache.add(1, 2); // 1
        ofxTest(aCache.has(1),"");
        ofxTestEq(*aCache.get(1), 2, "");

        aCache.add(3, 4); // 3-1
        ofxTest(aCache.has(1), "");
        ofxTest(aCache.has(3), "");
        ofxTestEq(*aCache.get(1), 2, ""); // 1-3
        ofxTestEq(*aCache.get(3), 4, ""); // 3-1

        aCache.add(5, 6); // 5-3-1
        ofxTest(aCache.has(1), "");
        ofxTest(aCache.has(3), "");
        ofxTest(aCache.has(5), "");
        ofxTestEq(*aCache.get(5), 6, "");  // 5-3-1
        ofxTestEq(*aCache.get(3), 4, "");  // 3-5-1

        aCache.add(7, 8); // 7-5-3|1
        ofxTest(!aCache.has(1), "");
        ofxTest(aCache.has(7), "");
        ofxTest(aCache.has(3), "");
        ofxTest(aCache.has(5), "");
        ofxTestEq(*aCache.get(5), 6, "");  // 5-7-3
        ofxTestEq(*aCache.get(3), 4, "");  // 3-5-7
        ofxTestEq(*aCache.get(7), 8, "");  // 7-3-5

        // test remove from the end and the beginning of the list
        aCache.remove(5); // 7-3
        ofxTest(!aCache.has(5), "");
        ofxTestEq(*aCache.get(3), 4, "");  // 3-7
        aCache.add(5, 6); // 5-3-7
        ofxTestEq(*aCache.get(7), 8, "");  // 7-5-3
        aCache.remove(7); // 5-3
        ofxTest(!aCache.has(7), "");
        ofxTest(aCache.has(3), "");
        ofxTestEq(*aCache.get(5), 6, "");  // 5-3

        // removing illegal entries should work too
        aCache.remove(666);

        aCache.clear();
        ofxTest(!aCache.has(5), "");
        ofxTest(!aCache.has(3), "");
    }


    void testDuplicateAdd()
    {
        ofxCache::LRUMemoryCache<int, int> aCache(3);
        aCache.add(1, 2); // 1
        ofxTest(aCache.has(1), "");
        ofxTestEq(*aCache.get(1), 2, "");
        aCache.add(1, 3);
        ofxTest(aCache.has(1), "");
        ofxTestEq(*aCache.get(1),  3, "");
    }
    
    
    void testUpdate()
    {
        std::string testName = "testUpdate";
        addCnt = 0;
        removeCnt = 0;
        ofxCache::LRUMemoryCache<int, int> aCache(3);

        auto lAdd = aCache.onAdd.newListener(this, &ofApp::onAdd);
        auto lUpdate = aCache.onAdd.newListener(this, &ofApp::onUpdate);
        auto lRemove = aCache.onRemove.newListener(this, &ofApp::onRemove);

        aCache.add(1, 2); // 1 ,one add event
        ofxTestEq(addCnt, 1, testName);
        ofxTestEq(updateCnt, 1, testName);
        ofxTestEq(removeCnt, 0, testName);

        ofxTest(aCache.has(1), testName);
        ofxTestEq(*aCache.get(1), 2, testName);

        ofxTestEq(addCnt, 1, testName);
        ofxTestEq(updateCnt, 1, testName);
        ofxTestEq(removeCnt, 0, testName);

        ofxTestEq(aCache.get(1000), std::shared_ptr<int>(), testName);

        aCache.add(1, 3);
        ofxTestEq(addCnt, 2, testName);
        ofxTestEq(updateCnt, 2, testName);
        ofxTestEq(removeCnt, 1, testName);

        aCache.remove(1);
        ofxTestEq(addCnt, 2, testName);
        ofxTestEq(updateCnt, 2, testName);
        ofxTestEq(removeCnt, 2, testName);

        ofxTestEq(aCache.size(), 0, testName);

    }

    void onAdd(const std::pair<int, std::shared_ptr<int>>& args)
    {
        ++addCnt;
    }

    void onUpdate(const std::pair<int, std::shared_ptr<int>>& args)
    {
        ++updateCnt;
    }


    void onRemove(const int& args)
    {
        ++removeCnt;
    }

    int addCnt = 0;
    int updateCnt = 0;
    int removeCnt = 0;
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
