#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
/*template<typename RetVal, typename... Types>
class Command {
  std::function<RetVal(Types)> fPtr;
  std::thread* pThr;
  Types tt;

public:
  Command(std::function<RetVal(Types)> _fPtr, Types ... _tt) : fPtr(_fPtr), tt(_tt) {}
  void execute() {
    pThr = new std::thread(fPtr, tt[0], tt[1]);
  }
  void tirm() {
    if (pThr->joinable()) {
      pThr->join();
    }
    else delete pThr;
  }

};*/


#include <mutex>
#include <condition_variable>
#include <chrono>
std::atomic<int> turn(0);
int count_loop[3] = { 0,0,0 };
std::mutex myMutex;
std::condition_variable myCondVar;

void print_time() {
  auto now = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
  std::cout << ms.count() << "ms: ";
}

void print_message(int index_thr, const int numThreads) {
 
  for (int i = 0; i < 5; ++i) {
    std::unique_lock<std::mutex> lock(myMutex);
    myCondVar.wait(lock, [index_thr]() {return index_thr == turn; });
    print_time();
    std::cout << "Thread " << index_thr << std::endl;
    turn = (turn + 2) % numThreads; // Передаём очередь следующему потоку
    myCondVar.notify_all();
  }
}
std::vector<std::thread*> vecThread;

void threadRunner(int thread_count) {
  for (int i = 0; i < thread_count; i++) {
    std::thread* tA = new std::thread(print_message, i, thread_count);
    vecThread.push_back(tA);
  }
}
void threadFinish() {
  for (auto thread : vecThread) {
    thread->join();
    delete thread;
  }
  vecThread.clear();
}

int main() {

  int array[10] = { 0,0,0,1,2,2,1,1 };
  threadRunner(3);
  threadFinish();
  
  std::cout << count_loop[0] << std::endl;
  std::cout << count_loop[1] << std::endl;
  std::cout << count_loop[2] << std::endl;
  return 0;
}
