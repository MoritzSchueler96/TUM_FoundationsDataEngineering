#include <iostream>
#include <string>
#include <charconv>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

// constexpr -> compute at compile time
static constexpr uint64_t buildPattern(char c)
{
  // Convert 00000000000000CC -> CCCCCCCCCCCCCCCC
  uint64_t v = c;
  return (v << 56 | v << 48 | v << 40 | v << 32 | v << 24 | v << 16 | v << 8) |
         v;
}

template <char separator> // compute at compile time
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
      return iter + (__builtin_ctzll(matches) >> 3) + 1; // count trailing zeros
  }

  // Check the last few characters explicitly
  while ((iter < end) && ((*iter) != separator))
    ++iter;
  if (iter < end)
    ++iter;
  return iter;
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
    const char *last = nullptr;
    unsigned col = 0;
    for (; iter < end;)
    {
      iter = findPattern<'|'>(iter, end);
      if (iter != end)
      {
        ++col;
        if (col == 4)
        {
          last = iter;
        }
        else if (col == 5)
        {
          unsigned v;
          from_chars(last, iter - 1, v);
          sum += v;
          iter = findPattern<'\n'>(iter, end);
          break;
        }
      }
    }
  }
  cout << sum << endl;

  munmap(data, length);
  close(handle);
}
