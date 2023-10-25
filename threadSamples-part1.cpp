#include <thread>
#include <iostream>
#include <functional>
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
    
    //this is version 3 of Acquire
    // it requires the passed in lock parameter is the safe object itself
    //thus it avoids the issue that an arbitrary lock can access the resource.
    //Also, it now requires the lock to be the same type of Safe that Acquire is.
     T& Acquire (unique_lock<MutexSafe<T>>& lock)
    {
        MutexSafe<T> *_safe = lock.mutex();
        if(&_safe->Mutex()!=&_mutex)
        {
            throw "wrong lock object passed to Acquire function.\n";
        }
        return *_resource;
    }
    //This overloaded Acquire allows unique_lock<mutex> to be passed in as a parameter
    T& Acquire (unique_lock<mutex>& lock)
    {
        if(lock.mutex()!=&_mutex)
        {
            throw "wrong lock object passed to Acquire function.\n";
        }
        return *_resource;
    }
};
void DannyWrite(string &blackboard)
{
    blackboard=+"My";
    this_thread::sleep_for(std::chrono::milliseconds(rand()%3));
    blackboard+= " name";
    this_thread::sleep_for(std::chrono::milliseconds(rand()%3));
    blackboard+=" is";
    this_thread::sleep_for(std::chrono::milliseconds(rand()%3));
    blackboard+= " Danny\n";
}
void PeterWrite(string& blackboard)
{
    blackboard+="My";
    this_thread::sleep_for(std::chrono::milliseconds(rand()%3));
    blackboard+= " name";
    this_thread::sleep_for(std::chrono::milliseconds(rand()%3));
    blackboard+=" is";
    this_thread::sleep_for(std::chrono::milliseconds(rand()%3));
    blackboard+= " Peter\n";
}
void DemoResouceConflict()
{
    string blackboard;
    thread DannyThread(DannyWrite, std::ref(blackboard));
    thread PeterThread(PeterWrite,std::ref(blackboard));
    DannyThread.join();
    PeterThread.join();
    cout<<blackboard<<endl;
}

typedef MutexSafe<string>  Safe;
void SafeDannyWrite(Safe& safe)
{
    unique_lock<Safe> lock(safe);
    string& blackboard = safe.Acquire(lock);
    DannyWrite(blackboard);
}
void SafePeterWrite(Safe& safe)
{
    unique_lock<Safe> lock(safe);
    string& blackboard = safe.Acquire(lock);
    PeterWrite(blackboard);
}
void TestSafeSmartlock()
{
    Safe safe(new string());
    thread DannyThread(SafeDannyWrite,ref(safe));
    thread PeterThread(SafePeterWrite, ref(safe));
    DannyThread.join();
    PeterThread.join();
    unique_lock<Safe> lock(safe);
    string& blackboard = safe.Acquire(lock);
    cout<<blackboard<<endl;
}
void PeterWriteWithMutex(mutex& amutex, string& blackboard )
{
    unique_lock<std::mutex> lk(amutex);
    PeterWrite(blackboard);
}
void DannywriteWithMutex(mutex& amutex, string& blackboard)
{
    unique_lock<std::mutex> lk(amutex);
    DannyWrite(blackboard);
}
void TestNomalSafeLock()
{
    string blackboard;
    std::mutex amutex;
    thread DannyThread(DannywriteWithMutex, std::ref(amutex), std::ref(blackboard));
    thread PeterThread(PeterWriteWithMutex,std::ref(amutex), std::ref(blackboard));
    DannyThread.join();
    PeterThread.join();
    cout<<blackboard<<endl;
}
int main()
{
    //DemoResouceConflict();
    TestSafeSmartlock();
    //TestNomalSafeLock();
    
    return 0;
}
