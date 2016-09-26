//// =============================================================================
////
//// Copyright (c) 2014-2016 Christopher Baker <http://christopherbaker.net>
////
//// Permission is hereby granted, free of charge, to any person obtaining a copy
//// of this software and associated documentation files (the "Software"), to deal
//// in the Software without restriction, including without limitation the rights
//// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//// copies of the Software, and to permit persons to whom the Software is
//// furnished to do so, subject to the following conditions:
////
//// The above copyright notice and this permission notice shall be included in
//// all copies or substantial portions of the Software.
////
//// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//// THE SOFTWARE.
////
//// =============================================================================
//
//
//#pragma once
//
//
//#include "ofx/TaskQueue.h"
//#include "ofx/Cache/LRUMemoryCache.h"
//#include "ofx/Cache/BaseAsyncCache.h"
//
//
//namespace ofx {
//namespace Cache {
//
//
//template<typename KeyType, typename ValueType>
//class CacheRequestTask;
//
//
//template<typename KeyType, typename ValueType>
//class BaseResourceCacheLoader
//{
//public:
//    virtual ~BaseResourceCacheLoader()
//    {
//    }
//    
//    /// \brief Load do a key request.
//    ///
//    /// This method is called from within tasks to load
//    ///
//    /// \param resource A shared pointer to the resource.
//    /// \param key key to load.
//    virtual std::shared_ptr<ValueType> load(CacheRequestTask<KeyType, ValueType>& request) = 0;
//
//    /// \brief Convert a given Key to a unique task id.
//    /// \param key key to convert.
//    /// \returns a unique string corresponding to the key.
//    virtual std::string toTaskId(const KeyType& key) const = 0;
//
//};
//
//
//template<typename KeyType, typename ValueType>
//class CacheRequestTask: public Poco::Task
//{
//public:
//    typedef std::pair<KeyType, std::shared_ptr<ValueType>> KeyValuePair;
//
//    CacheRequestTask(const KeyType& key, BaseResourceCacheLoader<KeyType, ValueType>& loader):
//        Poco::Task(loader.toTaskId(key)),
//        _key(key),
//        _loader(loader)
//    {
//    }
//
//    virtual ~CacheRequestTask()
//    {
//    }
//
//    void runTask() override
//    {
//        std::shared_ptr<ValueType> value = _loader.load(*this);
//
//        if (value != nullptr)
//        {
//            postNotification(new Poco::TaskCustomNotification<KeyValuePair>(this, std::make_pair(_key, value)));
//        }
//        else
//        {
//            throw Poco::IOException("Unable to load value for key.");
//        }
//    }
//
//    void setProgress(float progress)
//    {
//        Poco::Task::setProgress(progress);
//    }
//
//    KeyType key() const
//    {
//        return _key;
//    }
//
//private:
//    /// The key to load.
//    KeyType _key;
//    BaseResourceCacheLoader<KeyType, ValueType>& _loader;
//
//    friend class BaseResourceCacheLoader<KeyType, ValueType>;
//
//};
//
//
///// \brief A resource cache is a composite of a memory cache and a disk cache.
/////
///// When an value is requested with request() it will first search for the value
///// in the LRUMemeoryCache. If it is not available, a thread will be queued to
///// attempt to load the resource into the cache. Subclasses must implement the
///// load() function from the BaseKeyRequestTaskLoader() interface.
/////
///// \tparam KeyType The key type.
///// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
//template<typename KeyType, typename ValueType>
//class BaseResourceCache:
//    public BaseCache<KeyType, ValueType>,
//    public BaseResourceCacheLoader<KeyType, ValueType>
//{
//public:
////    typedef std::tuple<KeyType, std::shared_ptr<ValueType>, typename BaseCache<KeyType, ValueType>::CacheStatus> RequestCompleteArgs;
////    typedef std::pair<KeyType, std::string> RequestFailedArgs;
//
////    enum class RequestState
////    {
////        /// \brief The request is unknown.
////        UNKNOWN,
////        /// \brief The request is idle.
////        IDLE,
////        /// \brief The request is starting.
////        STARTING,
////        /// \brief The request is starting.
////        RUNNING,
////        /// \brief The request is being cancelled.
////        CANCELLING,
////        /// \brief The request is finished.
////        FINISHED
////    };
//
//    /// \brief Create a BaseResourceCache with the given parameters.
//    /// \param cacheSize The number of values to cache in memory.
//    /// \param taskQueue The task queue used to make carry out KeyRequestTasks.
//    BaseResourceCache(std::unique_ptr<BaseCache<KeyType, ValueType>> memoryCache = std::make_unique<LRUMemoryCache<KeyType, ValueType>>(),
//                      TaskQueue& taskQueue = TaskQueue::instance());
//
//    /// \brief Destroy the BaseResourceCache.
//    virtual ~BaseResourceCache();
//
//    /// \brief Request the given key.
//    ///
//    /// If the request is already queued, it will be ignored.  Results will be
//    /// returned via onRequestComplete, onRequestCancelled or onRequestFailed.
//    ///
//    /// \param key The key to cancel.
//    void request(const KeyType& key);
//
//    /// \brief Cancel any outstanding request for the given key.
//    ///
//    /// If there is no request for the given key, the request will be ignored.
//    /// This will attempt to cancel both queued and active requests.
//    ///
//    /// \param key The key to cancel.
//    void cancelRequest(const KeyType& key);
//
//    /// \brief Cancel any outstanding queued request for the given key.
//    ///
//    /// If there is no queued request for the given key, the request will be
//    /// ignored.
//    ///
//    /// \param key The key to cancel.
//    void cancelQueuedRequest(const KeyType& key);
//
//    /// \brief Get the progress 0 - 1 for the given key.
//    ///
//    /// For items that are not loaded or have no progress, return 0.  For items
//    /// that are in progress, return a value 0 - 1.  For items already in the
//    /// cache, return 1.
//    ///
//    /// \param key The key to query.
//    /// \returns the progress on a scale of 0 - 1.
//    float requestProgress(const KeyType& key) const;
//
//    /// \brief Get the state of a given request.
//    /// \param key The key to query.
//    /// \returns the state of the request.
//    RequestState requestState(const KeyType& key) const;
//
//    /// \brief An event that is called when the a value is requested.
//    ofEvent<const RequestCompleteArgs> onRequestComplete;
//
//    /// \brief This event is called when a resource request is cancelled.
//    ofEvent<const KeyType> onRequestCancelled;
//
//    /// \brief This event is called when a requested resource fails.
//    ///
//    /// The key will be paired with an error.
//    ofEvent<const RequestFailedArgs> onRequestFailed;
//
//protected:
//    bool doHas(const KeyType& key) const// override
//    {
//        return _memoryCache->has(key);
//    }
//
//    std::shared_ptr<ValueType> doGet(const KeyType& key)// override
//    {
//        return _memoryCache->get(key);
//    }
//
//    void doSet(const KeyType& key, std::shared_ptr<ValueType> entry)// override
//    {
//        _memoryCache->set(key, entry);
//    }
//
//    void doRemove(const KeyType& key)// override
//    {
//        _memoryCache->remove(key);
//    }
//
//    std::size_t doSize() //override
//    {
//        return _memoryCache->size();
//    }
//    void doClear() //override
//    {
//        _memoryCache->clear();
//    }
//
//    bool onTaskCancelled(const TaskQueueEventArgs& args);
//    bool onTaskFailed(const TaskFailedEventArgs& args);
//    bool onTaskCustomNotification(const TaskCustomNotificationEventArgs& args);
//
//    /// \brief A list of task ids.
//    std::map<std::string, KeyType> _requests;
//
//private:
//    /// \brief The shared task queue.
//    TaskQueue& _taskQueue;
//
//    /// \brief The task event listener.
//    ofEventListener _onTaskCancelledListener;
//    ofEventListener _onTaskFailedListener;
//    ofEventListener _onTaskCustomNotificationListener;
//
//    //    ofEventListener _onMem
//
//    std::unique_ptr<BaseCache<KeyType, ValueType>> _memoryCache;
//
//};
//
//
//template<typename KeyType, typename ValueType>
//BaseResourceCache<KeyType, ValueType>::BaseResourceCache(std::unique_ptr<BaseCache<KeyType, ValueType>> memoryCache,
//                                                         TaskQueue& taskQueue):
//    _memoryCache(std::move(memoryCache)),
//    _taskQueue(taskQueue),
//    _onTaskCancelledListener(_taskQueue.onTaskCancelled.newListener(this, &BaseResourceCache::onTaskCancelled)),
//    _onTaskFailedListener(_taskQueue.onTaskFailed.newListener(this, &BaseResourceCache::onTaskFailed)),
//    _onTaskCustomNotificationListener(_taskQueue.onTaskCustomNotification.newListener(this, &BaseResourceCache::onTaskCustomNotification))
//{
//}
//
//
//template<typename KeyType, typename ValueType>
//BaseResourceCache<KeyType, ValueType>::~BaseResourceCache()
//{
//}
//
//
//template<typename KeyType, typename ValueType>
//void BaseResourceCache<KeyType, ValueType>::request(const KeyType& key)
//{
//    try
//    {
//        auto taskId = this->toTaskId(key);
//        _taskQueue.start(taskId, new CacheRequestTask<KeyType, ValueType>(key, *this));
//        _requests[taskId] = key;;
//    }
//    catch (const Poco::ExistsException& exc)
//    {
//        // Do nothing.
//    }
//}
//
//
//template<typename KeyType, typename ValueType>
//void BaseResourceCache<KeyType, ValueType>::cancelRequest(const KeyType& key)
//{
//    try
//    {
//        _taskQueue.cancel(this->toTaskId(key));
//    }
//    catch (const Poco::ExistsException& exc)
//    {
//        // Do nothing.
//    }
//}
//
//
//template<typename KeyType, typename ValueType>
//void BaseResourceCache<KeyType, ValueType>::cancelQueuedRequest(const KeyType& key)
//{
//    try
//    {
//        _taskQueue.cancelQueued(this->toTaskId(key));
//    }
//    catch (const Poco::ExistsException& exc)
//    {
//        // Do nothing.
//    }
//}
//
//
//template<typename KeyType, typename ValueType>
//float BaseResourceCache<KeyType, ValueType>::requestProgress(const KeyType& key) const
//{
//    try
//    {
//        return _taskQueue.getTaskProgress(this->toTaskId(key));
//    }
//    catch (const Poco::ExistsException& exc)
//    {
//        return 0;
//    }
//}
//
//
//template<typename KeyType, typename ValueType>
//typename BaseResourceCache<KeyType, ValueType>::RequestState BaseResourceCache<KeyType, ValueType>::requestState(const KeyType& key) const
//{
//    try
//    {
//        Poco::Task::TaskState status = _taskQueue.getTaskState(toTaskId(key));
//
//        switch (status)
//        {
//            case Poco::Task::TASK_IDLE:
//                return RequestState::IDLE;
//            case Poco::Task::TASK_STARTING:
//                return RequestState::STARTING;
//            case Poco::Task::TASK_RUNNING:
//                return RequestState::RUNNING;
//            case Poco::Task::TASK_CANCELLING:
//                return RequestState::CANCELLING;
//            case Poco::Task::TASK_FINISHED:
//                return RequestState::FINISHED;
//            default:
//                return RequestState::NONE;
//        }
//    }
//    catch (const Poco::ExistsException& exc)
//    {
//        return RequestState::NONE;
//    }
//}
//
//
//template<typename KeyType, typename ValueType>
//bool BaseResourceCache<KeyType, ValueType>::onTaskCancelled(const TaskQueueEventArgs& args)
//{
//    auto iter = _requests.find(args.taskId());
//
//    if (iter != _requests.end())
//    {
//        ofNotifyEvent(onRequestCancelled, iter->second, this);
//        _requests.erase(iter);
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}
//
//
//template<typename KeyType, typename ValueType>
//bool BaseResourceCache<KeyType, ValueType>::onTaskFailed(const TaskFailedEventArgs& args)
//{
//    auto iter = _requests.find(args.taskId());
//    
//    if (iter != _requests.end())
//    {
//        auto evt = std::make_pair(iter->second, args.getException().displayText());
//        ofNotifyEvent(onRequestFailed, evt, this);
//        _requests.erase(iter);
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}
//
//
//template<typename KeyType, typename ValueType>
//bool BaseResourceCache<KeyType, ValueType>::onTaskCustomNotification(const TaskCustomNotificationEventArgs& args)
//{
//    auto iter = _requests.find(args.taskId());
//    
//    if (iter != _requests.end())
//    {
//        typename CacheRequestTask<KeyType, ValueType>::KeyValuePair result;
//        
//        if (args.extract(result))
//        {
//            // Cache it!
//            this->set(result.first, result.second);
//        }
//        else
//        {
//            ofLogError("BaseResourceCache<KeyType, ValueType>::onTaskCustomNotification") << "Unable to extract the value.";
//        }
//        
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}
//
//
//
/////// \brief A resource cache is a composite of a memory cache and a disk cache.
///////
/////// When an value is requested with request() it will first search for the value
/////// in the LRUMemeoryCache. If it is not available, a thread will be queued to
/////// attempt to load the resource into the cache. Subclasses must implement the
/////// load() function from the BaseKeyRequestTaskLoader() interface.
///////
/////// \tparam KeyType The key type.
/////// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
////template<typename KeyType, typename ValueType>
////class BaseResourceCache:
////    public BaseKeyRequestTaskLoader<KeyType, ValueType>,
////    public BaseAsyncCache<KeyType, ValueType>
////{
////public:
////    enum class RequestState
////    {
////        /// \brief The request is unknown.
////        UNKNOWN,
////        /// \brief The request is idle.
////        IDLE,
////        /// \brief The request is starting.
////        STARTING,
////        /// \brief The request is starting.
////        RUNNING,
////        /// \brief The request is being cancelled.
////        CANCELLING,
////        /// \brief The request is finished.
////        FINISHED
////    };
////
////    /// \brief Create a BaseResourceCache with the given parameters.
////    /// \param cacheSize The number of values to cache in memory.
////    /// \param taskQueue The task queue used to make carry out KeyRequestTasks.
////    BaseResourceCache(std::unique_ptr<BaseCache<KeyType, ValueType>> memoryCache = std::make_unique<LRUMemoryCache<KeyType, ValueType>>(),
////                      TaskQueue& taskQueue = TaskQueue::instance());
////
////    /// \brief Destroy the BaseResourceCache.
////    virtual ~BaseResourceCache();
////
////    /// \brief Cancel any outstanding request for the given key.
////    ///
////    /// If there is no request for the given key, the request will be ignored.
////    /// This will attempt to cancel both queued and active requests.
////    ///
////    /// \param key The key to cancel.
////    void cancelRequest(const KeyType& key);
////
////    /// \brief Cancel any outstanding queued request for the given key.
////    ///
////    /// If there is no queued request for the given key, the request will be
////    /// ignored.
////    ///
////    /// \param key The key to cancel.
////    void cancelQueuedRequest(const KeyType& key);
////
////    /// \brief Get the progress 0 - 1 for the given key.
////    ///
////    /// For items that are not loaded or have no progress, return 0.  For items
////    /// that are in progress, return a value 0 - 1.  For items already in the
////    /// cache, return 1.
////    ///
////    /// \param key The key to query.
////    /// \returns the progress on a scale of 0 - 1.
////    float requestProgress(const KeyType& key) const;
////
////    /// \brief Get the state of a given request.
////    /// \param key The key to query.
////    /// \returns the state of the request.
////    RequestState requestState(const KeyType& key) const;
////
////    /// \brief This event is called when a resource request is cancelled.
////    ofEvent<const KeyType> onRequestCancelled;
////
////    /// \brief This event is called when a requested resource fails.
////    ///
////    /// The key will be paired with an error.
////    ofEvent<const std::pair<KeyType, std::string>> onRequestFailed;
////
////protected:
////    void doRequest(const KeyType& key) override;
////
////    bool doHas(const KeyType& key) const override
////    {
////        return _memoryCache->has(key);
////    }
////
////    std::shared_ptr<ValueType> doGet(const KeyType& key) override
////    {
////        return _memoryCache->get(key);
////    }
////
////    void doSet(const KeyType& key, std::shared_ptr<ValueType> entry) override
////    {
////        _memoryCache->set(key, entry);
////    }
////
////    void doRemove(const KeyType& key) override
////    {
////        _memoryCache->remove(key);
////    }
////
////    std::size_t doSize() override
////    {
////        return _memoryCache->size();
////    }
////    void doClear() override
////    {
////        _memoryCache->clear();
////    }
////
////
////    bool onTaskCancelled(const TaskQueueEventArgs& args);
////    bool onTaskFailed(const TaskFailedEventArgs& args);
////    bool onTaskCustomNotification(const TaskCustomNotificationEventArgs& args);
////
////    /// \brief A list of task ids.
////    std::map<std::string, KeyType> _requests;
////
////private:
////    /// \brief The shared task queue.
////    TaskQueue& _taskQueue;
////
////    /// \brief The task event listener.
////    ofEventListener _onTaskCancelledListener;
////    ofEventListener _onTaskFailedListener;
////    ofEventListener _onTaskCustomNotificationListener;
////
//////    ofEventListener _onMem
////
////
////    std::unique_ptr<BaseCache<KeyType, ValueType>> _memoryCache;
////
////};
////
////
////template<typename KeyType, typename ValueType>
////BaseResourceCache<KeyType, ValueType>::BaseResourceCache(std::unique_ptr<BaseCache<KeyType, ValueType>> memoryCache,
////                                                         TaskQueue& taskQueue):
////    _memoryCache(std::move(memoryCache)),
////    _taskQueue(taskQueue),
////    _onTaskCancelledListener(_taskQueue.onTaskCancelled.newListener(this, &BaseResourceCache::onTaskCancelled)),
////    _onTaskFailedListener(_taskQueue.onTaskFailed.newListener(this, &BaseResourceCache::onTaskFailed)),
////    _onTaskCustomNotificationListener(_taskQueue.onTaskCustomNotification.newListener(this, &BaseResourceCache::onTaskCustomNotification))
////{
////}
////
////
////template<typename KeyType, typename ValueType>
////BaseResourceCache<KeyType, ValueType>::~BaseResourceCache()
////{
////}
////
////
////template<typename KeyType, typename ValueType>
////void BaseResourceCache<KeyType, ValueType>::doRequest(const KeyType& key)
////{
////    try
////    {
////        auto taskId = this->toTaskId(key);
////        _taskQueue.start(taskId, new KeyRequestTask<KeyType, ValueType>(key, *this));
////        _requests[taskId] = key;;
////    }
////    catch (const Poco::ExistsException& exc)
////    {
////        // Do nothing.
////    }
////}
////
////
////template<typename KeyType, typename ValueType>
////void BaseResourceCache<KeyType, ValueType>::cancelRequest(const KeyType& key)
////{
////    try
////    {
////        _taskQueue.cancel(this->toTaskId(key));
////    }
////    catch (const Poco::ExistsException& exc)
////    {
////        // Do nothing.
////    }
////}
////
////
////template<typename KeyType, typename ValueType>
////void BaseResourceCache<KeyType, ValueType>::cancelQueuedRequest(const KeyType& key)
////{
////    try
////    {
////        _taskQueue.cancelQueued(this->toTaskId(key));
////    }
////    catch (const Poco::ExistsException& exc)
////    {
////        // Do nothing.
////    }
////}
////
////
////template<typename KeyType, typename ValueType>
////float BaseResourceCache<KeyType, ValueType>::requestProgress(const KeyType& key) const
////{
////    try
////    {
////        return _taskQueue.getTaskProgress(this->toTaskId(key));
////    }
////    catch (const Poco::ExistsException& exc)
////    {
////        return 0;
////    }
////}
////
////
////template<typename KeyType, typename ValueType>
////typename BaseResourceCache<KeyType, ValueType>::RequestState BaseResourceCache<KeyType, ValueType>::requestState(const KeyType& key) const
////{
////    try
////    {
////        Poco::Task::TaskState status = _taskQueue.getTaskState(toTaskId(key));
////
////        switch (status)
////        {
////            case Poco::Task::TASK_IDLE:
////                return RequestState::IDLE;
////            case Poco::Task::TASK_STARTING:
////                return RequestState::STARTING;
////            case Poco::Task::TASK_RUNNING:
////                return RequestState::RUNNING;
////            case Poco::Task::TASK_CANCELLING:
////                return RequestState::CANCELLING;
////            case Poco::Task::TASK_FINISHED:
////                return RequestState::FINISHED;
////            default:
////                return RequestState::NONE;
////        }
////    }
////    catch (const Poco::ExistsException& exc)
////    {
////        return RequestState::NONE;
////    }
////}
////
////
////template<typename KeyType, typename ValueType>
////bool BaseResourceCache<KeyType, ValueType>::onTaskCancelled(const TaskQueueEventArgs& args)
////{
////    auto iter = _requests.find(args.taskId());
////
////    if (iter != _requests.end())
////    {
////        ofNotifyEvent(onRequestCancelled, iter->second, this);
////        _requests.erase(iter);
////        return true;
////    }
////    else
////    {
////        return false;
////    }
////}
////
////
////template<typename KeyType, typename ValueType>
////bool BaseResourceCache<KeyType, ValueType>::onTaskFailed(const TaskFailedEventArgs& args)
////{
////    auto iter = _requests.find(args.taskId());
////
////    if (iter != _requests.end())
////    {
////        auto evt = std::make_pair(iter->second, args.getException().displayText());
////        ofNotifyEvent(onRequestFailed, evt, this);
////        _requests.erase(iter);
////        return true;
////    }
////    else
////    {
////        return false;
////    }
////}
////
////
////template<typename KeyType, typename ValueType>
////bool BaseResourceCache<KeyType, ValueType>::onTaskCustomNotification(const TaskCustomNotificationEventArgs& args)
////{
////    auto iter = _requests.find(args.taskId());
////
////    if (iter != _requests.end())
////    {
////        typename KeyRequestTask<KeyType, ValueType>::KeyValuePair result;
////
////        if (args.extract(result))
////        {
////            // Cache it!
////            this->set(result.first, result.second);
////        }
////        else
////        {
////            ofLogError("BaseResourceCache<KeyType, ValueType>::onTaskCustomNotification") << "Unable to extract the value.";
////        }
////
////        return true;
////    }
////    else
////    {
////        return false;
////    }
////}
//
//
////////////////////////
////
////class SimpleResourceCache: public BaseResourceCache<std::string, std::string>
////{
////public:
////    using BaseResourceCache<std::string, std::string>::BaseResourceCache;
////
////    std::shared_ptr<std::string> load(KeyRequestTask<std::string, std::string>& task) override
////    {
////        // task.isCancelled()
////        task.setProgress(1);
////
////        auto result = std::make_shared<std::string>("hello");
////
////        if (result != nullptr)
////        {
////            return result;
////        }
////        else
////        {
////            throw Poco::IOException("Unable to load string.");
////        }
////    }
////
////    std::string toTaskId(const std::string& key) const override {
////        std::string _key = key;
////        std::reverse(_key.begin(), _key.end());
////        return _key;
////    }
////
////};
//
//
//} } // namespace ofx::Cache
//
