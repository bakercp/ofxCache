//
// Copyright (c) 2014 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#pragma once


#include "ofx/TaskQueue.h"
#include "ofx/Cache/LRUMemoryCache.h"
#include "ofx/Cache/BaseAsyncCache.h"


namespace ofx {
namespace Cache {


template<typename KeyType, typename ValueType>
class CacheRequestTask;


template<typename KeyType, typename ValueType>
class BaseResourceCacheLoader
{
public:
    virtual ~BaseResourceCacheLoader()
    {
    }
    
    /// \brief Load do a key request.
    ///
    /// This method is called from within tasks to load
    ///
    /// \param resource A shared pointer to the resource.
    /// \param key key to load.
    virtual std::shared_ptr<ValueType> load(CacheRequestTask<KeyType, ValueType>& request) = 0;

    /// \brief Convert a given Key to a unique task id.
    /// \param key key to convert.
    /// \returns a unique string corresponding to the key.
    virtual std::string toTaskId(const KeyType& key) const = 0;

};


template<typename KeyType, typename ValueType>
class CacheRequestTask: public Poco::Task
{
public:
    typedef std::pair<KeyType, std::shared_ptr<ValueType>> KeyValuePair;

    CacheRequestTask(const KeyType& key, BaseResourceCacheLoader<KeyType, ValueType>& loader):
        Poco::Task(loader.toTaskId(key)),
        _key(key),
        _loader(loader)
    {
    }

    virtual ~CacheRequestTask()
    {
    }

    void runTask() override
    {
        std::shared_ptr<ValueType> value = _loader.load(*this);

        if (value != nullptr)
        {
            postNotification(new Poco::TaskCustomNotification<KeyValuePair>(this, std::make_pair(_key, value)));
        }
        else
        {
            throw Poco::IOException("Unable to load value for key.");
        }
    }

    void setProgress(float progress)
    {
        Poco::Task::setProgress(progress);
    }

    KeyType key() const
    {
        return _key;
    }

private:
    /// The key to load.
    KeyType _key;
    BaseResourceCacheLoader<KeyType, ValueType>& _loader;

    friend class BaseResourceCacheLoader<KeyType, ValueType>;

};


/// \brief A resource cache is a composite of a memory cache and a disk cache.
///
/// When an value is requested with request() it will first search for the value
/// in the LRUMemoryCache. If it is not available, a thread will be queued to
/// attempt to load the resource into the cache. Subclasses must implement the
/// load() function from the BaseKeyRequestTaskLoader() interface.
///
/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
template<typename KeyType, typename ValueType>
class BaseResourceCache:
    public BaseAsyncCache<KeyType, ValueType>,
    public BaseResourceCacheLoader<KeyType, ValueType>
{
public:
    typedef RequestCompleteArgs<KeyType, ValueType> RequestCompleteArgs;
    typedef RequestFailedArgs<KeyType> RequestFailedArgs;

    /// \brief Create a BaseResourceCache with the given parameters.
    /// \param cacheSize The number of values to cache in memory.
    /// \param taskQueue The task queue used to make carry out KeyRequestTasks.
    BaseResourceCache(std::unique_ptr<BaseCache<KeyType, ValueType>> memoryCache = std::make_unique<LRUMemoryCache<KeyType, ValueType>>(),
                      TaskQueue& taskQueue = TaskQueue::instance());

    /// \brief Destroy the BaseResourceCache.
    virtual ~BaseResourceCache();

protected:
    bool doHas(const KeyType& key) const override
    {
        return _memoryCache->has(key);
    }

    std::shared_ptr<ValueType> doGet(const KeyType& key) override
    {
        return _memoryCache->get(key);
    }

    void doAdd(const KeyType& key, std::shared_ptr<ValueType> entry) override
    {
        _memoryCache->add(key, entry);
    }

    void doUpdate(const KeyType& key, std::shared_ptr<ValueType> entry) override
    {
        _memoryCache->update(key, entry);
    }

    void doRemove(const KeyType& key) override
    {
        _memoryCache->remove(key);
    }

    std::size_t doSize() override
    {
        return _memoryCache->size();
    }
    void doClear() override
    {
        _memoryCache->clear();
    }

    void doRequest(const KeyType& key) override;
    void doCancelRequest(const KeyType& key) override;
    void doCancelQueuedRequest(const KeyType& key) override;
    float doRequestProgress(const KeyType& key) const override;
    RequestState doRequestState(const KeyType& key) const override;

    bool onTaskCancelled(const TaskQueueEventArgs& args);
    bool onTaskFailed(const TaskFailedEventArgs& args);
    bool onTaskCustomNotification(const TaskCustomNotificationEventArgs& args);

    /// \brief A list of task ids.
    std::map<std::string, KeyType> _requests;

private:
    /// \brief The shared task queue.
    TaskQueue& _taskQueue;

    /// \brief The task event listener.
    ofEventListener _onTaskCancelledListener;
    ofEventListener _onTaskFailedListener;
    ofEventListener _onTaskCustomNotificationListener;

    std::unique_ptr<BaseCache<KeyType, ValueType>> _memoryCache;

};


template<typename KeyType, typename ValueType>
BaseResourceCache<KeyType, ValueType>::BaseResourceCache(std::unique_ptr<BaseCache<KeyType, ValueType>> memoryCache,
                                                         TaskQueue& taskQueue):
    _memoryCache(std::move(memoryCache)),
    _taskQueue(taskQueue),
    _onTaskCancelledListener(_taskQueue.onTaskCancelled.newListener(this, &BaseResourceCache::onTaskCancelled)),
    _onTaskFailedListener(_taskQueue.onTaskFailed.newListener(this, &BaseResourceCache::onTaskFailed)),
    _onTaskCustomNotificationListener(_taskQueue.onTaskCustomNotification.newListener(this, &BaseResourceCache::onTaskCustomNotification))
{
}


template<typename KeyType, typename ValueType>
BaseResourceCache<KeyType, ValueType>::~BaseResourceCache()
{
}


template<typename KeyType, typename ValueType>
void BaseResourceCache<KeyType, ValueType>::doRequest(const KeyType& key)
{
    try
    {
        auto taskId = this->toTaskId(key);
        _taskQueue.start(taskId, new CacheRequestTask<KeyType, ValueType>(key, *this));
        _requests[taskId] = key;;
    }
    catch (const Poco::ExistsException& exc)
    {
        // Do nothing.
    }
}


template<typename KeyType, typename ValueType>
void BaseResourceCache<KeyType, ValueType>::doCancelRequest(const KeyType& key)
{
    try
    {
        _taskQueue.cancel(this->toTaskId(key));
    }
    catch (const Poco::ExistsException& exc)
    {
        // Do nothing.
    }
}


template<typename KeyType, typename ValueType>
void BaseResourceCache<KeyType, ValueType>::doCancelQueuedRequest(const KeyType& key)
{
    try
    {
        _taskQueue.cancelQueued(this->toTaskId(key));
    }
    catch (const Poco::ExistsException& exc)
    {
        // Do nothing.
    }
}


template<typename KeyType, typename ValueType>
float BaseResourceCache<KeyType, ValueType>::doRequestProgress(const KeyType& key) const
{
    try
    {
        return _taskQueue.getTaskProgress(this->toTaskId(key));
    }
    catch (const Poco::ExistsException& exc)
    {
        return 0;
    }
}


template<typename KeyType, typename ValueType>
RequestState BaseResourceCache<KeyType, ValueType>::doRequestState(const KeyType& key) const
{
    try
    {
        Poco::Task::TaskState status = _taskQueue.getTaskState(this->toTaskId(key));

        switch (status)
        {
            case Poco::Task::TASK_IDLE:
                return RequestState::IDLE;
            case Poco::Task::TASK_STARTING:
                return RequestState::STARTING;
            case Poco::Task::TASK_RUNNING:
                return RequestState::RUNNING;
            case Poco::Task::TASK_CANCELLING:
                return RequestState::CANCELLING;
            case Poco::Task::TASK_FINISHED:
                return RequestState::FINISHED;
            default:
                return RequestState::UNKNOWN;
        }
    }
    catch (const Poco::ExistsException& exc)
    {
        return RequestState::UNKNOWN;
    }
}


template<typename KeyType, typename ValueType>
bool BaseResourceCache<KeyType, ValueType>::onTaskCancelled(const TaskQueueEventArgs& args)
{
    auto iter = _requests.find(args.taskId());

    if (iter != _requests.end())
    {
        ofNotifyEvent(this->onRequestCancelled, iter->second, this);
        _requests.erase(iter);
        return true;
    }
    else
    {
        return false;
    }
}


template<typename KeyType, typename ValueType>
bool BaseResourceCache<KeyType, ValueType>::onTaskFailed(const TaskFailedEventArgs& args)
{
    auto iter = _requests.find(args.taskId());
    
    if (iter != _requests.end())
    {
        RequestFailedArgs evt(iter->second, args.getException().displayText());
        ofNotifyEvent(this->onRequestFailed, evt, this);
        _requests.erase(iter);
        return true;
    }
    else
    {
        return false;
    }
}


template<typename KeyType, typename ValueType>
bool BaseResourceCache<KeyType, ValueType>::onTaskCustomNotification(const TaskCustomNotificationEventArgs& args)
{
    auto iter = _requests.find(args.taskId());
    
    if (iter != _requests.end())
    {
        typename CacheRequestTask<KeyType, ValueType>::KeyValuePair result;
        
        if (args.extract(result))
        {
            // Cache it!
            this->add(result.first, result.second);
        }
        else
        {
            ofLogError("BaseResourceCache<KeyType, ValueType>::onTaskCustomNotification") << "Unable to extract the value.";
        }
        
        return true;
    }
    else
    {
        return false;
    }
}


} } // namespace ofx::Cache
