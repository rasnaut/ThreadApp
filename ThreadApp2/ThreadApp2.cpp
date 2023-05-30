#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<thread>
#include<vector>
#include<cstdlib>
#include<chrono>
#include<mutex>
#include<condition_variable>
#include <shared_mutex>

#pragma warning(disable: 4996)

using namespace std;

mutex MY_MUTEX;
condition_variable MY_COND_VAR;
condition_variable reader_cv;

shared_mutex shMutex;

vector<int> ARR;
size_t SIZE_ARR = 100;
size_t SIZE_FILL = 20;
size_t SIZE_SHOW = 20;

size_t whatingWriter;
size_t workingWriter;
size_t workingReader;

class Locker {
  unique_lock<mutex> lock;
  /*size_t whatingWriter;
  size_t workingWriter;
  size_t workingReader;*/

public:
  Locker() : lock(MY_MUTEX) /*, whatingWriter(0), workingWriter(0), workingReader(0)*/ {}

  void RaderLock() {
    cout << "Reader START wating" << endl;
    reader_cv.wait(lock, [this]() { return workingWriter == 0 && whatingWriter == 0; });
    cout << "Reader STOP wating" << endl;
    workingReader++;
  }

  void ReaderUnlock() {
    workingReader--;
    reader_cv.notify_all();
  }

  void WriterLock() {
    whatingWriter++;
    MY_COND_VAR.wait(lock, [this]() { return workingWriter == 0 && workingReader == 0; });
    workingWriter++;
    whatingWriter--;
  }

  void WriterUnlock() {
    workingWriter--;
    if (whatingWriter)
      MY_COND_VAR.notify_all();
    else
      reader_cv.notify_all();

    MY_COND_VAR.notify_all();
  }
};

Locker MY_LOCKER;

void print_time() {
  const auto now = chrono::high_resolution_clock::now();
  auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

  cout << ms.count() << "ms: ";
}

void FillVec() {
  for (int i = 0; i < SIZE_FILL; i++) {
    int index = rand() % SIZE_ARR;
    //std::unique_lock<std::shared_mutex> lock(shMutex);
    shMutex.lock();
    //MY_LOCKER.WriterLock();
    print_time();
    cout << "Changing " << index << " element with " << ARR[index];
    ARR[index] = rand() % 10;
    cout << " to " << ARR[index] << endl;
    shMutex.unlock();
    //MY_LOCKER.WriterUnlock();
    std::this_thread::sleep_for(1ms);

  }
}

void ShowVec() {
  vector<int> show_arr(SIZE_SHOW, -1);
  for (int i = 0; i < SIZE_SHOW; i++) {
    int index = rand() % SIZE_ARR;
    //std::shared_lock<std::shared_mutex> lock(shMutex);
    shMutex.lock_shared();
    //MY_LOCKER.RaderLock();
    print_time();
    cout << "Reading " << index << " element: " << ARR[index] << endl;
    show_arr[i] = ARR[index];
    shMutex.unlock_shared();
    //MY_LOCKER.ReaderUnlock();
  }
  for (int i = 0; i < SIZE_SHOW; i++) cout << show_arr[i] << " ";
  cout << endl;
}


int main()
{
  system("chcp 1251");
  system("cls");

  ARR.reserve(SIZE_ARR);

  for (int i = 0; i < SIZE_ARR; i++) {
    ARR.push_back(rand() % 10);
    cout << "[" << i << "]" << ARR[i] << " ";
  }
  cout << endl;

  srand(time(nullptr));

  thread Writer1(FillVec);
  //thread Writer2(FillVec);
  thread Reader1(ShowVec);
  thread Reader2(ShowVec);

  Writer1.join();
  //Writer2.join();
  Reader1.join();
  Reader2.join();

  return 0;
}