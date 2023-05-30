#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <chrono>

void print_time() {
  const auto now = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

  std::cout << ms.count() << "ms: ";
}

class BankAccount {
  int balance = 0;
  std::shared_mutex sm;
public:
  void deposit(int amount) {
    std::unique_lock<std::shared_mutex> lock(sm);
    balance += amount;
    print_time();
    std::cout << "Deposit: " << amount << ", New balance: " << balance << std::endl;
  }

  void withdraw(int amount) {
    std::unique_lock<std::shared_mutex> lock(sm);
    if (balance >= amount) {
      balance -= amount;
      print_time();
      std::cout << "Withdraw: " << amount << ", New balance: " << balance << std::endl;
    }
  }

  void getBalance() {
    std::shared_lock<std::shared_mutex> lock(sm);
    print_time();
    std::cout << "Current Balance: " << balance << std::endl;
  }
};

int main() {
  BankAccount account;
  std::vector<std::thread> threads;

  // Create deposit threads
  for (int i = 0; i < 3; i++) {
    threads.push_back(std::thread([&]() {
      for (int j = 0; j < 5; j++) {
        account.deposit(100);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
      }
      }));
  }

  // Create withdraw threads
  for (int i = 0; i < 3; i++) {
    threads.push_back(std::thread([&]() {
      for (int j = 0; j < 5; j++) {
        account.withdraw(50);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
      }
      }));
  }

  // Create observer threads
  for (int i = 0; i < 3; i++) {
    threads.push_back(std::thread([&]() {
      for (int j = 0; j < 5; j++) {
        account.getBalance();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
      }
      }));
  }

  // Join all threads
  for (auto& thread : threads) {
    thread.join();
  }

  return 0;
}
