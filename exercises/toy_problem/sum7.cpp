#include <charconv>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

static constexpr uint64_t buildPattern(char c)
{
  // Convert 00000000000000CC -> CCCCCCCCCCCCCCCC
  uint64_t v = c;
  return (v << 56 | v << 48 | v << 40 | v << 32 | v << 24 | v << 16 | v << 8) |
         v;
}

template <char separator>
static const char *findPattern(const char *iter, const char *end)
// Returns the position after the pattern within [iter, end[, or end if not
// found
{
  // Loop over the content in blocks of 8 characters
  auto end8 = end - 8;
  constexpr uint64_t pattern = buildPattern(separator);
  for (; iter < end8; iter += 8)
  {
    // Check the next 8 characters for the pattern
    uint64_t block = *reinterpret_cast<const uint64_t *>(iter);
    constexpr uint64_t high = 0x8080808080808080ull;
    constexpr uint64_t low = ~high;
    uint64_t lowChars = (~block) & high;
    uint64_t foundPattern = ~((((block & low) ^ pattern) + low) & high);
    uint64_t matches = foundPattern & lowChars;
    if (matches)
      return iter + (__builtin_ctzll(matches) >> 3) + 1;
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
  // Loop over the content in blocks of 8 characters
  auto end8 = end - 8;
  constexpr uint64_t pattern = buildPattern(separator);
  for (; iter < end8; iter += 8)
  {
    // Check the next 8 characters for the pattern
    uint64_t block = *reinterpret_cast<const uint64_t *>(iter);
    constexpr uint64_t high = 0x8080808080808080ull;
    constexpr uint64_t low = ~high;
    uint64_t lowChars = (~block) & high;
    uint64_t foundPattern = ~((((block & low) ^ pattern) + low) & high);
    uint64_t matches = foundPattern & lowChars;
    if (matches)
    {
      unsigned hits = __builtin_popcountll(matches); // gives number of bits that are set
      if (hits >= n)
      {
        for (; n > 1; n--)
          matches &= matches - 1; // clears the last bit that is one
        return iter + (__builtin_ctzll(matches) >> 3) + 1;
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
  void *data = mmap(nullptr, length, PROT_READ, MAP_SHARED | MAP_POPULATE, handle, 0);
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
