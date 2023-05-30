#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include<cstdlib>
#include<chrono>

void print_time() {
  const auto now = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  std::cout << ms.count() << "ms: ";
}

class ReadWriteLock {
  std::mutex m;
  std::condition_variable readerQ, writerQ;
  std::atomic<int> activeReaders, waitingWriters, activeWriters;
public:
  ReadWriteLock() : activeReaders(0), waitingWriters(0), activeWriters(0) {}

  void read_lock() {
    std::unique_lock<std::mutex> lock(m);
    while (waitingWriters > 0)
      readerQ.wait(lock);
    ++activeReaders;
  }

  void read_unlock() {
    std::unique_lock<std::mutex> lock(m);
    --activeReaders;
    if (activeReaders == 0)
      writerQ.notify_all();
  }

  void write_lock() {
    std::unique_lock<std::mutex> lock(m);
    ++waitingWriters;
    while (activeReaders > 0 || activeWriters > 0)
      writerQ.wait(lock);
    --waitingWriters;
    ++activeWriters;
  }

  void write_unlock() {
    std::unique_lock<std::mutex> lock(m);
    --activeWriters;
    if (waitingWriters > 0)
      writerQ.notify_all();
    else
      readerQ.notify_all();
  }
};

std::vector<int> numbers;
ReadWriteLock rwLock;

void Writer(const std::string& name) {
  for (int j = 0; j < 5; ++j) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    rwLock.write_lock();
    print_time();
    std::cout << name << " added number: ";
    for (int i = 0; i < 20; ++i) { 
      int num = rand() % 100+1;
      int index = rand() % 100;
      numbers[index] = num;
      
      std::cout<< num << "[" << index << "] ";
    }
    print_time();
    std::cout << "\n" << std::endl;
    rwLock.write_unlock();
  }
  
}

void Reader(const std::string& name) {
  for (int i = 0; i < 5; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    rwLock.read_lock();
    print_time();
    std::cout << name << " reading numbers: ";
    for (int num : numbers) {
      std::cout << num << ' ';
    }
    std::cout << "\n" << std::endl;

    rwLock.read_unlock();
  }
}

int main() {
  srand(time(nullptr));
  for (int i = 0; i < 100; i++) {
    numbers.push_back(i);
    std::cout << "[" << i << "]" << numbers[i] << " ";
  }
  std::cout << "\n" << std::endl;
  std::thread w1(Writer, "Writer1");
  //std::thread w2(Writer, "Writer2");
  std::thread r1(Reader, "Reader1");
  std::thread r2(Reader, "Reader2");

  w1.join();
  //w2.join();
  r1.join();
  r2.join();

  return 0;
}
