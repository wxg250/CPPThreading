#include <thread>
#include <iostream>
#include <queue>
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

struct StockBlackboard
{
    float price;
    const string name;
    StockBlackboard(const string stockName, float stockPrice=0):name(stockName),price(stockPrice){}
};

typedef MutexSafe<StockBlackboard> StockSafe;

void PeterUpdateStock_Notify (StockSafe & safe, condition_variable& condition)
{
    for (int i=0;i<4;i++)
    {
        {
            unique_lock<StockSafe> lock(safe);
            StockBlackboard& stock= safe.Acquire(lock);
            if(i==2)
                stock.price=99;
            else
                stock.price = abs(rand()% 100);
            cout<<"Peter udpated the price to $"<<stock.price<<endl;
            if(stock.price>90)
            {
                lock.unlock();
                cout<<"Peter notified Danny at price $"<<stock.price<<", ";
                condition.notify_one();
                this_thread::sleep_for(std::chrono::milliseconds(10));
                lock.lock();
            }
        }
    std::this_thread::sleep_for(std::chrono::milliseconds(rand()%10));
    }
}
void DannyWait_ReadStock(StockSafe & safe, condition_variable& priceCondition)
{
    unique_lock<mutex> lock(safe.Mutex());
    cout<<"Danny is waiting for the right price to sell..."<<endl;
    priceCondition.wait(lock);
    StockBlackboard& stock= safe.Acquire(lock);
    if(stock.price>90)
        cout<<"Danny sell at: $"<<stock.price<<endl;
    else
        {
            cout<<"False alarm at $"<<stock.price<<" wait again..."<<endl;
            priceCondition.wait(lock);
            StockBlackboard& stock= safe.Acquire(lock);
            if(stock.price>90)
            cout<<"Danny sell at: $"<<stock.price<<endl;
        }
}

void TestConditionVariable()
{
    StockSafe safe (new StockBlackboard("APPL",30));
    condition_variable priceCondition;
    thread DannyThread(DannyWait_ReadStock, std::ref(safe), std::ref(priceCondition));
    thread PeterThread( PeterUpdateStock_Notify, std::ref(safe), std::ref(priceCondition));
    PeterThread.join();
    DannyThread.join();
}

int main()
{
    TestConditionVariable();
    return 0;
}
