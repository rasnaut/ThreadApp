#include<iostream>
#include<vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include<chrono>
#include <string>
#include<atomic>
#include <shared_mutex>
using namespace std;

vector<string> vec;

mutex _lock;
unique_lock<mutex> locker(_lock);
condition_variable cv;
atomic<bool> writing = false;
atomic<bool> writer_waiting = false;
atomic<int> readers = 0;

std::shared_mutex sh_mutex;

void print_time()
{
  const auto now = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
  std::cout << ms.count() << "ms: ";
}

void vectorAdd()
{
  std::unique_lock<std::shared_mutex> lock(sh_mutex);
  vec.push_back(to_string(4));
  print_time();
  cout << "wrote" << endl;
}

void vectorRead(int n)
{
  std::shared_lock<std::shared_mutex> lock(sh_mutex);
  for (auto i : vec)
  { 
    print_time();
    cout << "Read thread " << n << " " << i << endl;
  } 
}

int main()
{
  vec.push_back(to_string(1));
  vec.push_back(to_string(2));
  vec.push_back(to_string(3));
  thread t1(vectorAdd);
  thread t2(vectorRead, 1);
  thread t3(vectorRead, 2);
  thread t4(vectorRead, 3);
  thread t5(vectorRead, 4);
  thread t6(vectorAdd);
  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  t6.join();
  return 0;
}
