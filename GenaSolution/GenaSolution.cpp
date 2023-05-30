#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<thread>
#include<vector>
#include<cstdlib>
#include<chrono>
#include<mutex>
#include<condition_variable>

#pragma warning(disable: 4996)

using namespace std;



vector<int> ARR;
size_t SIZE_ARR = 100;
size_t SIZE_FILL = 20;
size_t SIZE_SHOW = 20;

//atomic<size_t> whatingWriter = 0;
//atomic<size_t> workingWriter = 0;
//atomic<size_t> workingReader = 0;

class Locker {
  mutex MY_MUTEX;
  condition_variable MY_COND_VAR;
  unique_lock<mutex> lock;
  atomic<size_t> whatingWriter;
  atomic<size_t> workingWriter;
  atomic<size_t> workingReader;
  atomic<size_t> waitingReader;

public:
  Locker() : lock(MY_MUTEX), whatingWriter(0), workingWriter(0), workingReader(0), waitingReader(0){}

  void RaderLock() {
    waitingReader++;
    //cout << "Reader START wating " << workingWriter << whatingWriter << endl;
    MY_COND_VAR.wait(lock, [this]() { 
      //cout << "CHECK workingWriter: " << workingWriter << ", whatingWriter: " << whatingWriter << endl;
      return workingWriter == 0 && whatingWriter == 0; 
      });
    waitingReader--;
    cout << "Reader STOP wating" << endl;
    workingReader++;
  }

  void ReaderUnlock() {
    workingReader--;
    MY_COND_VAR.notify_all();
    cout << "Reader UNLOCK!!!" << endl;
  }

  void WriterLock() {
    whatingWriter++;
    cout << "WRITER START wating " << workingWriter << whatingWriter << endl;
    MY_COND_VAR.wait(lock, [this]() { return workingWriter == 0 && workingReader == 0; });
    cout << "WRITER STOP wating " << workingWriter << whatingWriter << endl;
    workingWriter++;
    whatingWriter--;
  }

  void WriterUnlock() {
    workingWriter--;
    MY_COND_VAR.notify_all();
    cout << "WRITER UNLOCK ";
    cout << "workingWriter " << workingWriter << " whatingWriter " << whatingWriter << " waitingReader: " << waitingReader << endl;
    this_thread::sleep_for(1ms);
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

    MY_LOCKER.WriterLock();
    print_time();
    cout << i << "!!!CHANGING " << index << " element to " << ARR[index];
    ARR[index] = rand() % 10;
    cout << " на " << ARR[index] << endl;
    MY_LOCKER.WriterUnlock();
  }
  //MY_COND_VAR.notify_all();

}

void ShowVec() {
  vector<int> show_arr(SIZE_SHOW, -1);
  for (int i = 0; i < SIZE_SHOW; i++) {
    int index = rand() % SIZE_ARR;

    MY_LOCKER.RaderLock();
    print_time();
    cout << "READING " << index << " element: " << ARR[index] << endl;
    show_arr[i] = ARR[index];
    MY_LOCKER.ReaderUnlock();
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

  thread Reader1(ShowVec);
  thread Reader2(ShowVec);
  thread Writer1(FillVec);
  //thread Writer2(FillVec);

  Reader1.join();
  Reader2.join();
  Writer1.join();
  //Writer2.join();

  return 0;
}