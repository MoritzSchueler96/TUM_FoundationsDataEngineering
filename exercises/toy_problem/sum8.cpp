#include <immintrin.h>
#include <charconv>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

template <char separator>
static const char *findPattern(const char *iter, const char *end)
// Returns the position after the pattern within [iter, end[, or end if not
// found
{
  // Loop over the content in blocks of 32 characters
  auto end32 = end - 32;
  const auto pattern = _mm256_set1_epi8(separator);
  for (; iter < end32; iter += 32)
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
  auto end32 = end - 32;
  const auto pattern = _mm256_set1_epi8(separator);
  for (; iter < end32; iter += 32)
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

int main(int argc, char *argv[])
{
  if (argc != 2)
    return 1;

  int handle = open(argv[1], O_RDONLY);
  lseek(handle, 0, SEEK_END);
  auto length = lseek(handle, 0, SEEK_CUR);
  void *data = mmap(nullptr, length, PROT_READ, MAP_SHARED, handle, 0);
  auto begin = static_cast<const char *>(data), end = begin + length;

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
  cout << sum << endl;

  munmap(data, length);
  close(handle);
}
