#pragma once
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <optional>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include "Template.h"
using namespace Template;

struct Async
{

   




enum class TaskSync
{
    Async,
    MainThread,
    Inherit,
};

struct ITask
{
    virtual std::function<void()> operator()(TaskSync& sync) = 0;
    virtual ~ITask() = default;
    TaskSync sync = TaskSync::MainThread;
    ITask* next = nullptr;
    int32_t offset = 0;

};

struct Thread {
    std::thread thread;
    std::mutex mutex;
    std::atomic<bool> kill = false;

    Thread() : kill(false) {}

    template<typename >
    void Task()
    {

    }

};



class ThreadPool {
public:

    std::vector<std::unique_ptr<Thread>> threads;
    std::vector<std::unique_ptr<Thread>> disposedThreads;

    std::queue<std::function<void()>> asyncTasks;
    std::queue<std::function<void()>> syncTasks;
    std::mutex queueMutex;
    std::condition_variable event;
    std::atomic_bool clear_all = false;

    size_t threadCount = 1;

    ThreadPool() = default;

    ~ThreadPool() {
        numThreads(0);
        clearDisposedThreads();
    }

    void WorkFunction(Thread* thread) {
        while (true) {
            std::function<void()> task = nullptr;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                event.wait(lock, [&thread, this] {
                    return thread->kill.load() || !asyncTasks.empty();
                    });

                if (thread->kill.load() || (clear_all.load() && asyncTasks.empty())) break;

                if (!asyncTasks.empty()) {
                    task = asyncTasks.front();
                    asyncTasks.pop();
                }
            }
            if (task) {
                task();
            }
        }
    }

    void emplaceTask(std::function<void()> task, TaskSync sync) {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (sync == TaskSync::Async) {
            asyncTasks.emplace(std::move(task));
            lock.unlock();
            event.notify_one();
        }
        else {
            syncTasks.emplace(std::move(task));
        }
    }

    void workOnMainThread() {
        while (!syncTasks.empty()) {
            auto task = syncTasks.front();
            syncTasks.pop();
            task();
        }
    }
    void clearDisposedThreads()
    {
        for (auto& thread : disposedThreads)
        {
            if (thread->thread.joinable())
            {
                thread->thread.join();
            }
        }
        disposedThreads.clear();
    }
    void clearExpiredThreads()
    {
        for (size_t i = 0; i < disposedThreads.size(); i++)
        {
            if (!disposedThreads[i]->thread.joinable())
            {
                std::swap(disposedThreads[i], disposedThreads.back());
                disposedThreads.pop_back();
                i--;
            }
        }
    }
    void numThreads(size_t _threadCount)
    {
        this->threadCount = _threadCount;
        while (threads.size() != _threadCount)
        {
            if (threads.size() > _threadCount)
            {
                if (const auto& handle = threads.back())
                {
                    handle->kill.store(true);
                    disposedThreads.emplace_back(std::move(threads.back()));
                    threads.pop_back();
                }
            }
            if (threads.size() < _threadCount)
            {
                auto thread = std::make_unique<Thread>();
                thread->thread = std::thread(&ThreadPool::WorkFunction, this, thread.get());
                threads.emplace_back(std::move(thread));

            }
        }
        event.notify_all();
    }

    
    void cleanUp()
    {
        clear_all.store(true);
        for (const auto& thread : threads)
        {
            if (thread->thread.joinable())
            {
                thread->thread.join();
            }
        }

        clearDisposedThreads();
    }
    
};

struct Task
{
    size_t padding;
};

template<typename Function, typename Object, typename... Args>
    requires MemberFunctionPointer<Function, Object, Args...>
struct AsyncTask
{
    std::function<void()> taskFunc;

    AsyncTask(Function f, Object* obj, Args&&... args) {
        taskFunc = [f, obj, ...args = std::forward<Args>(args)]() mutable {
            (obj->*f)(args...);
            };
    }
    operator Task()
    {
        send();
        return { 0 };
    }
    void send()
    {
        if (taskFunc)
        {
            Async::Schedule(std::move(taskFunc), TaskSync::Async);
        }
    }
    ~AsyncTask()
    {
        send();
    }
};


template<typename Function, typename Object, typename... Args>
    requires MemberFunctionPointer<Function, Object, Args...>
struct SyncTask
{
    std::function<void()> taskFunc;

    SyncTask(Function f, Object* obj, Args&&... args) {
        taskFunc = [f, obj, ...args = std::forward<Args>(args)]() mutable {
            (obj->*f)(args...);
            };
    }
    operator Task()
    {
        send();
        return { 0 };
    }
    void send()
    {
        if (taskFunc)
        {
            Async::Schedule(std::move(taskFunc), TaskSync::MainThread);
        }
    }
    ~SyncTask()
    {
        send();
    }
};

struct IJobBase {};

template<typename T>
struct IJob : IJobBase
{
    IJob() = default;
    bool complete = false;
    Task exit()
    {
        complete = true;
        delete reinterpret_cast<T*>(this);
        return { 0 };
    }
    ~IJob()
    {
        if (!complete)
        {
            throw std::runtime_error("Jobs must be allocated on the heap");
            exit();
        }
    }
    IJob(IJob&& other)noexcept = delete;
    IJob& operator=(IJob&& other)noexcept = delete;
    IJob(IJob& other)noexcept = delete;
    IJob& operator=(IJob& other)noexcept = delete;

};

struct VirtualTask
{
    virtual int operator()(void* enviroment, int execute) = 0;
    virtual ~VirtualTask() = default;
    TaskSync sync;
};

template<typename Function>
struct TaskFunction : public VirtualTask {
    using DecayedFunction = typename std::decay<Function>::type;
    DecayedFunction function;

    TaskFunction(Function func, TaskSync sync) : function(std::forward<Function>(func)) {
        this->sync = sync;
    }

    int operator()(void* environment, int id) override {
        auto obj = static_cast<typename Lambda<DecayedFunction>::firstParameter>(environment);
        function(obj);
        return id + 1;
    }

    ~TaskFunction() override = default;
};
template<typename Function>
struct ConditionalTaskFunction : public VirtualTask {
    using DecayedFunction = typename std::decay<Function>::type;
    DecayedFunction function;

    ConditionalTaskFunction(Function func, TaskSync sync) : function(std::forward<Function>(func)) {
        this->sync = sync;
    }

    int operator()(void* environment, int id) override {
        auto obj = static_cast<typename Lambda<DecayedFunction>::firstParameter>(environment);
        ;
        return function(obj, id);
    }

    ~ConditionalTaskFunction() override = default;
};
template<typename Function>
struct ParallelTask : public VirtualTask {
    using DecayedFunction = typename std::decay<Function>::type; // Ensure Function is not a reference
    DecayedFunction function; // Use decayed tokenType for storage
    int32_t numThreads;

    ParallelTask(Function&& func, TaskSync sync, int32_t numThreads) : function(std::forward<Function>(func)) {
        this->sync = sync;
        this->numThreads = numThreads;
    }

    int operator()(void* environment, int id) override {
        const int numthreads = this->numThreads;
        auto obj = static_cast<typename Lambda<DecayedFunction>::firstParameter>(environment);
        std::atomic<int> count;
        std::condition_variable condition;
        std::mutex mutex;
        for (int32_t i = 0; i < numthreads; i++) {
            Async::Schedule([&, i]() {
                function(obj, i);
                std::lock_guard<std::mutex> waitForOthers(mutex);
                ++count;
                condition.notify_one();
                }, TaskSync::Async);
        }
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this, numthreads, &count]() { return numthreads == count; });
        return id + 1;
    }

    ~ParallelTask() override = default;
};

template <typename UniformSpace>
struct BuildJob : UniformSpace {

    std::vector<std::unique_ptr<VirtualTask>> functions;
    size_t current = 0;

    template<typename...Args>
    BuildJob(Args&&...args) :UniformSpace(std::forward<Args>(args)...)
    {

    }

    void yield() {
        if (current < functions.size())
        {
            auto func = functions[current].get();
            TaskSync sync = func->sync;

            const std::function<void()> enqueueFunc = [this, func]() {
                current = func->operator()(this, current);
                yield();
                };

            switch (sync)
            {
            case TaskSync::Async:
                Async::Schedule(enqueueFunc, sync); break;
            case TaskSync::Inherit:
                current = functions[current]->operator()(this, current);
                yield(); break;
            case TaskSync::MainThread:
                Async::Schedule(enqueueFunc, sync); break;
            }
        }
        else
        {
            delete this;
        }
    }
};

template <typename UniformSpace>
    requires std::is_constructible_v<UniformSpace>
struct JobBuilder {
    BuildJob<UniformSpace>* job = nullptr;
    TaskSync taskSync = TaskSync::Inherit;


    JobBuilder() = default;
    void singletonInit()
    {
        if (job == nullptr)
        {
            job = new BuildJob<UniformSpace>();
        }
    }
    BuildJob<UniformSpace>* operator->()
    {
        singletonInit();
        return job;
    }

    template<typename LambdaT>
        requires (std::is_same_v<typename Lambda<LambdaT>::argsTuple, std::tuple<UniformSpace*, int32_t>>&&
    std::is_same_v< typename Lambda<LambdaT>::retType, void>)
        auto& parallel(LambdaT func, int32_t numThreads)
    {
        singletonInit();
        job->functions.emplace_back(std::make_unique<ParallelTask<LambdaT>>(std::forward<LambdaT>(std::move(func)), taskSync, numThreads));
        taskSync = TaskSync::Inherit;
        return *this;
    }
    template<typename LambdaT>
        requires (std::is_same_v<typename Lambda<LambdaT>::argsTuple, std::tuple<UniformSpace*>>&&
    std::is_same_v< typename Lambda<LambdaT>::retType, void>)
        auto& append(LambdaT func)
    {
        singletonInit();
        job->functions.emplace_back(std::make_unique < TaskFunction<LambdaT>>(std::forward<LambdaT>(std::move(func)), taskSync));
        taskSync = TaskSync::Inherit;
        return *this;
    }




    template<typename LambdaT>
        requires (std::is_same_v<typename Lambda<LambdaT>::argsTuple, std::tuple<UniformSpace*, int32_t>>&&
    std::is_same_v< typename Lambda<LambdaT>::retType, int32_t>)
        auto& branch(LambdaT func)
    {
        singletonInit();
        job->functions.emplace_back(std::make_unique <ConditionalTaskFunction<LambdaT>>(std::forward<LambdaT>(std::move(func)), taskSync));
        taskSync = TaskSync::Inherit;
        return *this;
    }


    auto& switchToMainThread()
    {
        return setSync<TaskSync::MainThread>();
    }

    auto& switchToAsync()
    {
        return setSync<TaskSync::Async>();
    }


    void Schedule() {
        singletonInit();
        job->yield();
        job = nullptr;
    }

    JobBuilder& operator = (JobBuilder& other) noexcept = delete;
    JobBuilder(JobBuilder& other) noexcept = delete;
    JobBuilder& operator = (JobBuilder&& other) noexcept
    {
        if (&other != this)
        {
            release();
            job = other.job;
            taskSync = other.taskSync;
        }
        return *this;
    }
    JobBuilder(JobBuilder&& other)noexcept :job(other.job), taskSync(other.taskSync)
    {
        other.job = nullptr;
    }
    void release()
    {
        delete job;
        job = nullptr;
    }
    ~JobBuilder()
    {
        release();
    }
private:
    template<TaskSync Sync>
    auto& setSync()
    {
        taskSync = Sync;
        return *this;
    }

};



	static void SetWorkers(size_t numWorkers)
	{
	    instance().numThreads(numWorkers);
	}
	static void CleanUp()
	{
	    instance().cleanUp();
	}
	template<typename FunctionPointer, typename Object>
	    requires (is_member_function_pointer_of_type_v<FunctionPointer, Object>&& MemberFunctionPointer<FunctionPointer, Object>)
	static void ScheduleAsync(FunctionPointer func, Object* inst) {
	    auto lambda = [func, inst]() {
	        (inst->*func)();
	        };
	    std::function<void()> function = lambda;
	    instance().emplaceTask(std::move(function), TaskSync::Async);
	}

	template<typename FunctionPointer, typename Object>
	    requires (is_member_function_pointer_of_type_v<FunctionPointer, Object>&& MemberFunctionPointer<FunctionPointer, Object>)
	static void ScheduleSync(FunctionPointer func, Object* inst) {
	    auto lambda = [func, inst]() {
	        (inst->*func)();
	        };
	    std::function<void()> function = lambda;
	    instance().emplaceTask(std::move(function), TaskSync::MainThread);
	}
	static void Schedule(std::function<void()> func, TaskSync sync = TaskSync::Async)
	{
	    instance().emplaceTask(std::move(func), sync);
	}
	static void WorkOnMainThread()
	{
	    instance().workOnMainThread();
	    instance().clearExpiredThreads();
	}
	static ThreadPool& instance()
	{
	    static ThreadPool inst;
	    return inst;
	}


};
