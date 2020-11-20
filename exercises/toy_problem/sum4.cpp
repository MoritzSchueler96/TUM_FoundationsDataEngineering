#include <iostream>
#include <string>
#include <charconv>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

static const char *findPattern(const char *iter, const char *end,
                               uint64_t pattern) {
  auto end8 = end - 8;
  for (; iter < end8; iter += 8) {
    uint64_t block = *reinterpret_cast<const uint64_t *>(iter);
    constexpr uint64_t high = 0x8080808080808080ull;
    constexpr uint64_t low = ~high;
    uint64_t lowChars = (~block) & high;
    uint64_t foundPattern = ~((((block & low) ^ pattern) + low) & high);
    uint64_t matches = foundPattern & lowChars;
    if (matches)
      break;
  }

  while ((iter < end) && ((*iter) != '\n'))
    ++iter;
  if (iter < end)
    ++iter;
  return iter;
}

int main(int argc, char *argv[]) {
  if (argc != 2)
    return 1;

  int handle = open(argv[1], O_RDONLY);
  lseek(handle, 0, SEEK_END);
  auto length = lseek(handle, 0, SEEK_CUR);
  void *data = mmap(nullptr, length, PROT_READ, MAP_SHARED, handle, 0);
  auto begin = static_cast<const char *>(data), end = begin + length;

  unsigned sum = 0;
  for (auto iter = begin; iter < end;) {
    const char *last = nullptr;
    unsigned col = 0;
    for (; iter < end; ++iter)
      if ((*iter) == '|') {
        ++col;
        if (col == 4) {
          last = iter + 1;
        } else if (col == 5) {
          unsigned v;
          from_chars(last, iter, v);
          sum += v;

          iter = findPattern(iter, end, 0xA0A0A0A0A0A0A0A0ull);
          break;
        }
      }
  }
  cout << sum << endl;

  munmap(data, length);
  close(handle);
}
