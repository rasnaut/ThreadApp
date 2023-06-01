// PromiseFutureApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <future>
#include <atomic>
using namespace std::chrono_literals;


std::atomic<bool> turn(false);
std::atomic<int> value(0);
std::mutex mx;

void funcAA(std::condition_variable &cv) {
  std::this_thread::sleep_for(10ms);
  value = 10;
  turn = true;
  cv.notify_one();
}

void funcBB(std::condition_variable &cv) {
  std::unique_lock<std::mutex> lock(mx);
  cv.wait(lock, [] { return turn.load(); });
  std::cout << value << std::endl;
  turn = false;
}

void funcA(std::promise<int> &prom) {
    std::this_thread::sleep_for(10ms);
    //
    //do something
    //
    prom.set_value(1);
    std::this_thread::sleep_for(10ms);
    std::cout << "funcA finish\n";
}

void funcB(std::future<int> &fut) {
  fut.wait();
  std::cout << fut.get() << std::endl;
}
int main()
{
  std::promise<int> mProm;
  std::future<int> mFut = mProm.get_future();
  std::thread promiseThread(funcA, std::ref(mProm));
  std::thread futureThread(funcB, std::ref(mFut));
  
    
  promiseThread.join();
  futureThread.join();
    std::cout << "Hello World!\n";
    
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
