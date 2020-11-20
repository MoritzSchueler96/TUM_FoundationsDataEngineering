#include <charconv>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 2)
    return 1;

  ifstream in(argv[1]);
  unsigned sum = 0;
  string line;
  while (getline(in, line)) {
    const char *last = nullptr;
    unsigned col = 0;
    for (char &c : line)
      if (c == '|') {
        ++col;
        if (col == 4) {
          last = (&c) + 1;
        } else if (col == 5) {
          unsigned v;
          from_chars(last, &c, v);
          sum += v;
          break;
        }
      }
  }
  cout << sum << endl;
}