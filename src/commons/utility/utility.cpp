#include "utility.h"
#include <cmath>
#include <limits>
#include <numeric>

void print(int s) { std::cout << s << std::endl; }
void print(double s) { std::cout << s << std::endl; }
void print(std::string s) { std::cout << s << std::endl; }

void print(std::vector<int> s) {
  std::cout << "<<";
  for (auto &i : s)
    std::cout << i << " ";
  std::cout << ">>" << std::endl;
}

void print(std::set<int> s) {
  std::cout << "<<";
  for (auto &i : s)
    std::cout << i << " ";
  std::cout << ">>" << std::endl;
}

template <class T> void debug(T s) {
  std::cout << "DEBUG:: " << s << std::endl;
}

template void debug<int>(int);
template void debug<std::string>(std::string);

double std_dev(std::vector<int> v, double avg) {
  double E = 0;
  double inverse = 1.0 / static_cast<double>(v.size());
  for (size_t i = 0; i < v.size(); i++) {
    E += pow(static_cast<double>(v[i]) - avg, 2);
  }
  return sqrt(inverse * E);
}

double std_dev(std::vector<int> v) {
  if (v.size() == 1)
    return std::numeric_limits<double>::max();
  double avg = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
  return std_dev(v, avg);
}
