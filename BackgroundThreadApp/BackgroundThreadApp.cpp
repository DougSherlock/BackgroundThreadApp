// BackgroundThreadApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <future>
#include <mutex>
#include <string>
#include <sstream>
#include <chrono>
#include "BackgroundThreadApp.h"

using namespace std;

mutex logMutex;
void Log(const char* msg)
{
    unique_lock<mutex> ul(logMutex);
    cout << msg << endl;
}
void Log(stringstream& ss)
{
    Log(ss.str().c_str());
    ss.clear();
    ss << "";
}
std::thread::id mainThreadId;

long getTick()
{
    auto duration = chrono::system_clock::now().time_since_epoch();
    auto millis = chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return (long) millis;
}

string func1(int loop, int sleepMs, int cancelMs)
{
    long startMs = getTick();
    for (int i = 0; i < loop; i++)
    {
        std::unique_lock<mutex> ul(logMutex);        
        cout << ((this_thread::get_id() == mainThreadId) ? "main " : "work ") 
            << this_thread::get_id()
            << " " << __FUNCTION__ << " " << i << " \n";

        if ((getTick() - startMs) > cancelMs)
        {
            cout << ((this_thread::get_id() == mainThreadId) ? "main " : "work ")
                << this_thread::get_id()
                << " " << __FUNCTION__ << " quttting " << " \n";
            break;
        }

        ul.unlock();
        this_thread::sleep_for(chrono::milliseconds(sleepMs));
    }

    stringstream ss;
    ss << "thread " << this_thread::get_id() << " done\n";
    return ss.str();
}


void TestAsync() // sample code that uses std::async object
{
    auto res1 = std::async(launch::async, func1, 5, 1000, rand() % 5000);
    auto res2 = std::async(launch::async, func1, 5, 1000, rand() % 5000);

    cout << func1(5, 1000, rand() % 5000); // execute func1 in the main thread

    cout << res1.get();
    cout << res2.get();
}


class scoped_thread {
    std::thread t;
public:
    explicit scoped_thread(std::thread t_) : t(std::move(t_)) {
        Log(__FUNCTION__);
        if (!t.joinable()) throw std::logic_error("No thread");
    }
    ~scoped_thread() {
        Log(__FUNCTION__);
        t.join();
    }
    scoped_thread(scoped_thread&) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};



void TestScopedThread() // sample code that uses std::thread object with a scoped_thread class which waits (calls join) when it goes out of scope
{
    stringstream ss;
    ss << __FUNCTION__ << " started";
    Log(ss);

    scoped_thread t1(std::thread(func1, 5, 1000, rand() % 5000));
    scoped_thread t2(std::thread(func1, 5, 1000, rand() % 5000));

    ss << __FUNCTION__ << " done";
    Log(ss);
}

int main()
{
    mainThreadId = this_thread::get_id();
    cout << "thread:" << this_thread::get_id() << " " << __FUNCTION__ << " started\n";

    TestAsync();

    TestScopedThread();

    cout << __FUNCTION__ << " press Enter\n";
    getchar();
    cout << __FUNCTION__ << " done\n";
}


