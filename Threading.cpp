#include <iostream>
#include <vector>
#include <iterator> // std::back_inserter
#include <functional> // std::plus
#include <algorithm> // std::transform
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>


using namespace std;

recursive_mutex cout_mutex; // global mutexes are horrible of course, is test code only

void threadFunc(int x) {
  int scaleFactor = 100;
  auto sleepTimeInMs = max(0, (5 - x)*scaleFactor);
  //std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimeInMs));
  lock_guard<recursive_mutex> lock(cout_mutex);
  cout << "thread " << std::this_thread::get_id() << " slept=" << sleepTimeInMs << " ms value=" << x << endl;
}

class ProducerConsumer {
  public:
    void produce(int count) {
      for (int i=0; i<count; ++i) {
        std::this_thread::sleep_for(chrono::milliseconds(500));
        std::lock_guard<std::mutex> lock(mutex);
        cout << "producer thread " << std::this_thread::get_id() << " produced item=" << i << endl;
        workItemQueue.push_back(i);
        conditionVariable.notify_one();
      }
    }
    void consume(int count) {
      for (int i=0; i<count; ++i)
      {
        std::unique_lock<std::mutex> lock(mutex);
        conditionVariable.wait(lock, [&]{return !workItemQueue.empty();});
        int workItem = workItemQueue.back();
        workItemQueue.pop_back();
        // processing the item while holding the lock would be silly, whatever, it's just a test
        cout << "consumer thread " << std::this_thread::get_id() << " consumed item=" << workItem << endl;
      }
      cout << "consumer thread exits\n";
    }
  private:
    vector<int> workItemQueue;

    std::mutex mutex;
    std::condition_variable conditionVariable;
};

void main() {
  {
    vector<thread> threads;
    for (int i=0; i<5; ++i)
      threads.push_back(thread(threadFunc, i));

    // sadly doesnt compile (copy c'tor)
    //for (auto t : threads)
    //  t.join();

    std::for_each(threads.begin(), threads.end(), [](thread& t) { t.join(); });
  }

  {
    ProducerConsumer producerConsumer;
    thread prod([&]() { producerConsumer.produce(5); });
    thread con1([&]() { producerConsumer.consume(2); });
    thread con2([&]() { producerConsumer.consume(2); });
    prod.join();
    con1.join();
    con2.join();
  }
}