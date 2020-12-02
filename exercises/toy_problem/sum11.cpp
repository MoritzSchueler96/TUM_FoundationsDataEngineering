#include <atomic>
#include <charconv>
#include <fcntl.h>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

template <char separator>
static const char *findPattern(const char *iter, const char *end)
// Returns the position after the pattern within [iter, end[, or end if not
// found
{
  // Loop over the content in blocks of 32 characters
  auto end8 = end - 32;
  const auto pattern = _mm256_set1_epi8(separator);
  for (; iter < end8; iter += 32)
  {
    // Check the next 32 characters for the pattern
    auto block = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(iter));
    auto matches = _mm256_movemask_epi8(_mm256_cmpeq_epi8(block, pattern));
    if (matches)
      return iter + __builtin_ctzll(matches) + 1;
  }

  // Check the last few characters explicitly
  while ((iter < end) && ((*iter) != separator))
    ++iter;
  if (iter < end)
    ++iter;
  return iter;
}

template <char separator>
static const char *findNthPattern(const char *iter, const char *end, unsigned n)
// Returns the position after the pattern within [iter, end[, or end if not
// found
{
  // Loop over the content in blocks of 32 characters
  auto end8 = end - 32;
  const auto pattern = _mm256_set1_epi8(separator);
  for (; iter < end8; iter += 32)
  {
    // Check the next 32 characters for the pattern
    auto block = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(iter));
    auto matches = _mm256_movemask_epi8(_mm256_cmpeq_epi8(block, pattern));
    if (matches)
    {
      unsigned hits = __builtin_popcountll(matches);
      if (hits >= n)
      {
        for (; n > 1; n--)
          matches &= matches - 1;
        return iter + __builtin_ctzll(matches) + 1;
      }
      n -= hits;
    }
  }

  // Check the last few characters explicitly
  for (; iter < end; ++iter)
    if ((*iter) == separator)
      if ((--n) == 0)
        return iter + 1;

  return end;
}

static unsigned computeSum(const char *begin, const char *end)
{
  unsigned sum = 0;
  for (auto iter = begin; iter < end;)
  {
    auto last = findNthPattern<'|'>(iter, end, 4);
    iter = last;
    unsigned v = 0;
    while (iter < end)
    {
      char c = *(iter++);
      if ((c >= '0') && (c <= '9'))
        v = 10 * v + c - '0';
      else
        break;
    }
    sum += v;
    iter = findPattern<'\n'>(iter, end);
  }
  return sum;
}

static const char *findBoundary(const char *begin, const char *end,
                                unsigned chunkCount, unsigned index)
{
  if (index == 0)
    return begin;
  if (index == chunkCount)
    return end;

  const char *b = begin + ((end - begin) * index / chunkCount);
  return findPattern<'\n'>(b, end);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
    return 1;

  int handle = open(argv[1], O_RDONLY);
  lseek(handle, 0, SEEK_END);
  auto length = lseek(handle, 0, SEEK_CUR);
  void *data = mmap(nullptr, length, PROT_READ, MAP_SHARED, handle, 0);
  madvise(data, length, MADV_SEQUENTIAL);
  madvise(data, length, MADV_WILLNEED);
  auto begin = static_cast<const char *>(data), end = begin + length;

  vector<thread> threads;
  atomic<unsigned> sum;
  sum = 0;

  for (unsigned index = 0, threadCount = thread::hardware_concurrency();
       index != threadCount; ++index)
  {
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
