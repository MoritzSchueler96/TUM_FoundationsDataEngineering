#include <atomic>
#include <charconv>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

static unsigned computeSum(const unsigned *begin, const unsigned *end) {
  unsigned sum = 0;
  for (auto iter = begin; iter < end;++iter)
    sum+=*iter;
  return sum;
}

static const unsigned* findBoundary(const unsigned *begin, const unsigned*end,
                                unsigned chunkCount, unsigned index) {
  if (index == 0)
    return begin;
  if (index == chunkCount)
    return end;

  return begin + ((end - begin) * index / chunkCount);
}

int main(int argc, char *argv[]) {
  if (argc != 2)
    return 1;

  int handle = open(argv[1], O_RDONLY);
  lseek(handle, 0, SEEK_END);
  auto length = lseek(handle, 0, SEEK_CUR);
  void *data = mmap(nullptr, length, PROT_READ, MAP_SHARED, handle, 0);
  madvise(data, length, MADV_SEQUENTIAL);
  madvise(data, length, MADV_WILLNEED);
  auto begin = static_cast<const unsigned *>(data), end = begin + (length/sizeof(unsigned));

  atomic<unsigned> sum = 0;
  vector<thread> threads;

  for (unsigned index = 0, threadCount = thread::hardware_concurrency();
       index != threadCount; ++index) {
    threads.push_back(thread([index, threadCount, begin, end, &sum]() {
      // Executed on a background thread
      auto from = findBoundary(begin, end, threadCount, index);
      auto to = findBoundary(begin, end, threadCount, index + 1);
      unsigned localSum = computeSum(from, to);
      sum += localSum;
    }));
  }

  for (auto &t : threads)
    t.join();

  cout << sum.load() << endl;

  munmap(data, length);
  close(handle);
}