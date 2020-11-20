#include <iostream>
#include <string>
#include <charconv>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

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
    for (; iter < end; ++iter)
      if ((*iter) == '|')
      {
        ++col;
        if (col == 4)
        {
          last = iter + 1;
        }
        else if (col == 5)
        {
          unsigned v;
          from_chars(last, iter, v);
          sum += v;
          while ((iter < end) && ((*iter) != '\n'))
            ++iter;
          if (iter < end)
            ++iter;
          break;
        }
      }
  }
  cout << sum << endl;

  munmap(data, length);
  close(handle);
}
