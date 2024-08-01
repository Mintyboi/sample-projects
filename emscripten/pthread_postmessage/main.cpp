#include <iostream>       // std::cout
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <future>
#include <chrono>         // std::chrono::seconds
#include <sstream>

int count = 1;

int pause_thread(int n)
{
    std::cout << "Thread started...\n";
    int threadid = count++;
    std::this_thread::sleep_for (std::chrono::seconds(n));
    std::stringstream msg;
    msg << "[" << threadid << "]" << " pause of " << n << " seconds ended\n";
    std::cout << msg.str();
    return 1;
}

int main() 
{
    // std::cout << "Spawning 4 futures...\n";
    
    // auto t1 = std::async(std::launch::async, pause_thread, 5);
    // std::cout << "After 1...\n";
    
    // auto t2 = std::async(std::launch::async, pause_thread, 6);
    // std::cout << "After 2...\n";
    
    // auto t3 = std::async(std::launch::async, pause_thread, 2);
    // std::cout << "After 3...\n";
    
    // auto t4 = std::async(std::launch::async, pause_thread, 3);
    // std::cout << "After 4...\n";

    // std::stringstream msg;
    // msg << "t1 Result = " << t1.get() << std::endl; std::cout << msg.str(); msg.str("");
    // msg << "t2 Result = " << t2.get() << std::endl; std::cout << msg.str(); msg.str("");
    // msg << "t3 Result = " << t3.get() << std::endl; std::cout << msg.str(); msg.str("");
    // msg << "t4 Result = " << t4.get() << std::endl; std::cout << msg.str(); msg.str("");
    // std::cout << "All threads joined!\n";


    std::cout << "Spawning 4 threads...\n";
    std::thread t1 = std::thread(pause_thread,6);
    std::thread t2 = std::thread(pause_thread,6);
    t1.detach();
    t2.detach();

    std::this_thread::sleep_for (std::chrono::seconds(10));
    std::thread t3 = std::thread(pause_thread,2);
    std::thread t4 = std::thread(pause_thread,3);
    std::cout << "Done spawning threads. Now waiting for them to join:\n";
    
    t3.join();
    t4.join();
    
    std::cout << "All threads joined!\n";

  return 0;
}   