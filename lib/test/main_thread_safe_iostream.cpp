#include <iostream>
#include <thread>
#include "thread_safe_iostream.hpp"

void printNumbers(const std::string& p_prefix) {
    threadSafeCout.setPrefix(p_prefix);
    for (int i = 1; i <= 5; ++i) {
        threadSafeCout << "Number: " << i << std::endl;       
    }
}

void getNumber(const std::string& p_prefix) {
    int number;
    threadSafeCout.setPrefix(p_prefix);
    threadSafeCout << "Enter a number: ";
    threadSafeCout >> number;
    threadSafeCout << "You entered: " << number << std::endl;
}

void promptUser(const std::string& p_prefix) {
    std::string name;
    threadSafeCout.setPrefix(p_prefix);
    threadSafeCout.prompt("Enter your name: ", name);
    threadSafeCout << "Hello, " << name << "!" << std::endl;
}

int main() {
    std::string prefix1 = "[Thread 1] ";
    std::string prefix2 = "[Thread 2] ";

    std::thread thread1(printNumbers, prefix1);
    std::thread thread2(printNumbers, prefix2);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread thread3(getNumber, "[Thread 3] ");
    std::thread thread4(getNumber, "[Thread 4] ");
    std::thread thread5(promptUser, "[Thread 5] ");
    std::thread thread6(promptUser, "[Thread 6] ");

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    thread5.join();
    thread6.join();
}
