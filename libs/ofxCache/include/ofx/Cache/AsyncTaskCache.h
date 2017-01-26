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
class AsyncCacheRequestTask;


template<typename KeyType, typename ValueType>
class BaseAsyncCacheLoader
{
public:
    virtual ~BaseAsyncCacheLoader()
    {
    }
    
    /// \brief Load do a key request.
    ///
    /// This method is called from within tasks to load
    ///
    /// \param resource A shared pointer to the resource.
    /// \param key key to load.
    virtual std::shared_ptr<ValueType> load(AsyncCacheRequestTask<KeyType, ValueType>& request) = 0;

    /// \brief Convert a given Key to a unique task id.
    /// \param key key to convert.
    /// \returns a unique string corresponding to the key.
    virtual std::string toTaskId(const KeyType& key) const = 0;

};


template<typename KeyType, typename ValueType>
class AsyncCacheRequestTask: public Poco::Task
{
public:
    typedef std::pair<KeyType, std::shared_ptr<ValueType>> KeyValuePair;

    AsyncCacheRequestTask(const KeyType& key, BaseAsyncCacheLoader<KeyType, ValueType>& loader):
        Poco::Task(loader.toTaskId(key)),
        _key(key),
        _loader(loader)
    {
    }

    virtual ~AsyncCacheRequestTask()
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
    BaseAsyncCacheLoader<KeyType, ValueType>& _loader;

    friend class BaseAsyncCacheLoader<KeyType, ValueType>;

};


/// \tparam KeyType The key type.
/// \tparam ValueType The value type (e.g. a std::shared_ptr<ValueType>).
template<typename KeyType, typename ValueType>
class AsyncTaskCache:
    public BaseAsyncCache<KeyType, ValueType>,
    public BaseResourceCacheLoader<KeyType, ValueType>
{
public:
    typedef RequestCompleteArgs<KeyType, ValueType> RequestCompleteArgs;
    typedef RequestFailedArgs<KeyType> RequestFailedArgs;

    /// \brief Create a AsyncTaskCache with the given parameters.
    /// \param taskQueue The task queue used to make carry out KeyRequestTasks.
    AsyncTaskCache(TaskQueue& taskQueue = TaskQueue::instance());

    /// \brief Destroy the AsyncTaskCache.
    virtual ~AsyncTaskCache();

protected:
    virtual bool doHas(const KeyType& key) const override
    {
        ofLogError("AsyncTaskCache::doHas()") << "Not implemented.";
    }

    virtual std::shared_ptr<ValueType> doGet(const KeyType& key) override
    {
        ofLogError("AsyncTaskCache::doGet()") << "Not implemented.";
    }

    virtual void doAdd(const KeyType& key, std::shared_ptr<ValueType> entry) override
    {
        ofLogError("AsyncTaskCache::doAdd()") << "Not implemented.";
    }

    virtual void doUpdate(const KeyType& key, std::shared_ptr<ValueType> entry) override
    {
        ofLogError("AsyncTaskCache::doUpdate()") << "Not implemented.";
    }

    virtual void doRemove(const KeyType& key) override
    {
        ofLogError("AsyncTaskCache::doRemove()") << "Not implemented.";
    }

    virtual std::size_t doSize() override
    {
        ofLogError("AsyncTaskCache::doSize()") << "Not implemented.";
        return 0;
    }
    virtual void doClear() override
    {
        ofLogError("AsyncTaskCache::doClear()") << "Not implemented.";
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

};


template<typename KeyType, typename ValueType>
AsyncTaskCache<KeyType, ValueType>::AsyncTaskCache(TaskQueue& taskQueue):
    _taskQueue(taskQueue),
    _onTaskCancelledListener(_taskQueue.onTaskCancelled.newListener(this, &AsyncTaskCache::onTaskCancelled)),
    _onTaskFailedListener(_taskQueue.onTaskFailed.newListener(this, &AsyncTaskCache::onTaskFailed)),
    _onTaskCustomNotificationListener(_taskQueue.onTaskCustomNotification.newListener(this, &AsyncTaskCache::onTaskCustomNotification))
{
}


template<typename KeyType, typename ValueType>
AsyncTaskCache<KeyType, ValueType>::~AsyncTaskCache()
{
}


template<typename KeyType, typename ValueType>
void AsyncTaskCache<KeyType, ValueType>::doRequest(const KeyType& key)
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
void AsyncTaskCache<KeyType, ValueType>::doCancelRequest(const KeyType& key)
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
void AsyncTaskCache<KeyType, ValueType>::doCancelQueuedRequest(const KeyType& key)
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
float AsyncTaskCache<KeyType, ValueType>::doRequestProgress(const KeyType& key) const
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
RequestState AsyncTaskCache<KeyType, ValueType>::doRequestState(const KeyType& key) const
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
bool AsyncTaskCache<KeyType, ValueType>::onTaskCancelled(const TaskQueueEventArgs& args)
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
bool AsyncTaskCache<KeyType, ValueType>::onTaskFailed(const TaskFailedEventArgs& args)
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
bool AsyncTaskCache<KeyType, ValueType>::onTaskCustomNotification(const TaskCustomNotificationEventArgs& args)
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

