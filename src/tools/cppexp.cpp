// Example program
#include <iostream>
#include <string>
#include <vector>

struct Cmpx {
  int a;
  bool d;
  double e;

  Cmpx(int a1, bool d1, double e1) : a(a1), d(d1), e(e1) {}
};

void print_vec(const std::vector<int> &vec) {
  for (auto x : vec) {
    std::cout << ' ' << x;
  }
  std::cout << '\n';
}

int main() {
  std::vector<Cmpx> res;
  int r = 5;
  while (r-- > 0) {
    // Cmpx tmp(r, true, 2.4);
    // std::cout << &tmp << std::endl;
    res.push_back(Cmpx(r, true, 2.5));
  }
  std::cout << &res[0] << std::endl;
  std::cout << res[1].a << std::endl;

  std::cout << "+++++" << std::endl;
  std::vector<int> R{1, 2, 3, 4, 5};
  std::vector<int> tail{10, 20, 30};
  R.insert(R.end(), tail.begin(), tail.end());
  print_vec(R);
  print_vec(tail);
  std::cout << "+++++" << std::endl;

  std::string path1, path2;
  path1 = path2 = "r110";
  path1 += '0';
  path2 += '1';
  std::cout << (path1.at(29)) << std::endl;
  std::cout << path1 << std::endl;
  std::cout << path2 << std::endl;
  return 0;
}
