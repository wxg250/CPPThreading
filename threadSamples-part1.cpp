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
    template <class SafeT>
    T& Acquire (unique_lock<SafeT>& lock)
    {
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
