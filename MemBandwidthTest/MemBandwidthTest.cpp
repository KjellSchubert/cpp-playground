#include <iostream>
#include <thread>
#include <vector>
#include <future>
#include <algorithm>
#include <chrono>

using namespace std;

// returns totalFlopCount
int memBandwithWaster(vector<float>& workArea, chrono::steady_clock::time_point tEnd) {
  const auto workAreaSize = workArea.size();
  int loopCount = 0;
  while (chrono::steady_clock::now() < tEnd) {
    for (unsigned int i=0; i<workAreaSize; ++i)
      ++workArea[i]; // this here is the 'work' being done
    ++loopCount;
  }
  auto totalFlopCount = loopCount * workAreaSize;
  cout << "  loopCount=" << loopCount << "\n";
  return totalFlopCount;
}

struct Worker {
  thread thread;
  vector<float> workArea;
  int totalFlopCount; // result of memBandwithWaster(workArea)

  // not needed for clang, but for msvs 2012:
  // (this code sucks btw)
#ifdef _MSC_VER // msvs
  Worker(const Worker& rhs) {
    if (rhs.thread.get_id() != thread::id())
      throw std::runtime_error("rhs.thread was supposed to be a default-constructed thread");
    workArea = rhs.workArea;
    totalFlopCount = rhs.totalFlopCount;
  }
  // and rule of 3/5 tedium:
  Worker operator=(const Worker& rhs) {
    if (rhs.thread.get_id() != thread::id())
      throw std::runtime_error("rhs.thread was supposed to be a default-constructed thread");
    workArea = rhs.workArea;
    totalFlopCount = rhs.totalFlopCount;
    return *this;
  }
  Worker() {}
#endif
  
};

// @param workAreaSize in floats
void computeMegaflops(int threadN, int workAreaSize) {

  auto t0 = chrono::steady_clock::now();
  auto tEnd = t0 + chrono::seconds(1);

  // setup all workareas before starting the worker threads
  vector<Worker> workers(threadN);
  for (auto& worker : workers)
    worker.workArea.resize(workAreaSize);

  // now spawn workers, one for each work area
  for (auto& worker : workers) {
    thread workerThread([&worker, tEnd]() { 
      worker.totalFlopCount = memBandwithWaster(worker.workArea, tEnd);
    });
    std::swap(worker.thread, workerThread);
  }

  // aggregate result: sum of each thread's megaflops
  float totalFlopCount = 0;
  for (auto& worker : workers) {
    worker.thread.join();
    totalFlopCount += worker.totalFlopCount;
  }

  auto totalTimeInMs = chrono::duration_cast<chrono::milliseconds>(
                         chrono::steady_clock::now() - t0).count();
  auto megaFlops =
    (totalFlopCount * 1000.0f / totalTimeInMs) 
    / (1000*1000.0f);

  cout << "computeMegaflops(" << threadN << ", " << workAreaSize 
    << ") => totalFlopCount=" << static_cast<int>(totalFlopCount)
    << " totalTimeInMs=" << totalTimeInMs
    << " => " << megaFlops << " megaFlops\n";
}

int main() {

  auto runTest = [](int workAreaSize, int maxThreadN)
  {
    cout << "\n\ntesting with workAreaSize=" << workAreaSize << '\n';
    for (int threadN = 1; threadN<=maxThreadN; threadN*=2)
      computeMegaflops(threadN, workAreaSize);
  };

  runTest(10, 128);
  runTest(1000, 32);
  runTest(1000 * 1000, 32);

  return 0;
}