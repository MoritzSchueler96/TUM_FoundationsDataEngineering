#include <charconv>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 3)
    return 1;

  ifstream in(argv[1]);
  ofstream out(argv[2]);
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
          out.write(reinterpret_cast<char *>(&v), sizeof(unsigned));
          break;
        }
      }
  }
}