#include <thread>
#include <iostream>
#include <queue>
#include <functional>
#include <fstream>
using namespace std;

template <typename T>
class MutexSafe
{
private:
    mutex _mutex;
    T* _resource;
    T* operator ->(){}
    T& operator &(){}
public:
    MutexSafe(T* resource):_resource(resource){}
    ~MutexSafe(){delete _resource;}
    void lock()
    {
        _mutex.lock();
    }
    void unlock()
    {
        _mutex.unlock();
    }
    bool try_lock()
    {
        return _mutex.try_lock();
    }
    mutex& Mutex()
    {
        return _mutex;
    }
    T& Acquire (unique_lock<MutexSafe<T>>& lock)
    {
        MutexSafe<T> *_safe = lock.mutex();
        if(&_safe->Mutex()!=&_mutex)
        {
            throw "wrong lock object passed to Acquire function.\n";
        }
        return *_resource;
    }
    T& Acquire (unique_lock<mutex>& lock)
    {
        if(lock.mutex()!=&_mutex)
        {
            throw "wrong lock object passed to Acquire function.\n";
        }
        return *_resource;
    }
};
template <typename MsgType>
class MsgQueue
{
private:
    queue<MsgType> _queue; 
    mutex _mutex;
    condition_variable _enqCv;
    condition_variable _deqCv;
    int _limit;
public:
    MsgQueue(int limit=3):_limit(limit){}
    void Enqueue(MsgType & msg)
    {
        unique_lock<mutex> lock(_mutex);
        if(_queue.size()>=_limit)
        {
            //cout<<"queue is full, wait()..."<<endl;
            _enqCv.wait(lock,[this]{return _queue.size()<_limit;});
        }
        _queue.push(msg);
        _deqCv.notify_one();
    }
    MsgType& Dequeue()
    {
        unique_lock<mutex> lock(_mutex);
        if(_queue.size()<=0)
        {
            cout<<"queue is empty, wait()..."<<endl;
            _deqCv.wait(lock, [this]{return _queue.size()>0;});
        }
        MsgType& msg = _queue.front();
        _queue.pop();
        _enqCv.notify_one();
        return msg;
    }
    int Size()
    {
        unique_lock<mutex> lock(_mutex);
        return _queue.size();
    }
};

struct StockCommand
{
    string command;
    float price;
    StockCommand(){}
    StockCommand(const StockCommand& cp):command(cp.command),price(cp.price){}
    void ExecuteCommand()
    {
        if(price>0)
            cout<<"Command "<<command<<" is executed at price $"<<price<<endl;
        else
            cout<<"market closed because the price is $"<<price<<endl;
    }
};

typedef MsgQueue<StockCommand> TaskQueueType;
typedef MutexSafe<TaskQueueType> TaskQueueSafe;
class ThreadPool
{
private:
    int _limit;
    vector<thread*> _workerThreads;
    TaskQueueType& _taskQueue;
    bool _threadPoolStop=false;
public:
  
    void ExecuteCommand()
    {
        while(1)
        {
            StockCommand task=_taskQueue.Dequeue();
            task.ExecuteCommand();
            if(task.price<0)
            {
                _threadPoolStop=true;
                _taskQueue.Enqueue(task);//tell other threads to stop
            }
            if(_threadPoolStop)
            {
                cout<< "thread finshed!"<<endl;
                return;
            }
            //the sleep function simulates that the task takes a while to finish 
            std::this_thread::sleep_for(std::chrono::milliseconds(rand()%100));
        }
    }

    ThreadPool(TaskQueueType& taskQueue,int limit=3):_limit(limit),_taskQueue(taskQueue){
        for(int i=0;i<_limit;++i)
        {
            _workerThreads.push_back(new thread(&ThreadPool::ExecuteCommand,this));
        }
    }
    ~ThreadPool(){
        for(auto threadObj: _workerThreads)
        {
            if(threadObj->joinable())
                {
                    threadObj->join();
                    delete threadObj;
                }
        }
    }
};

void TestThreadPool()
{
    TaskQueueType taskQueue(5);
    ThreadPool pool(taskQueue,3);
    //the sleep function simulates the situation that all  
    //worker threads are waiting for the empty message queue at the beginning
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    for(int i=0;i<10;i++)
    {
        StockCommand task;
        task.price= i+1;
        if(task.price >5)
            task.command = "sell at price $";
        else
            task.command = "buy at price $";    
        taskQueue.Enqueue(task);
    }
    StockCommand marketCloseCommand;
    marketCloseCommand.command="Market Closed!";
    marketCloseCommand.price=-1;
    taskQueue.Enqueue(marketCloseCommand);
}
int main()
{
    TestThreadPool();
    return 0;
}
