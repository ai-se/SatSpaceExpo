#include "utility.h"
#include <cmath>
#include <limits>
#include <numeric>
#include <random>

void debug(int s) { std::cout << s << std::endl; }
void debug(double s) { std::cout << s << std::endl; }
void debug(std::string s) { std::cout << s << std::endl; }

void debug(std::vector<int> s) {
  std::cout << "<<";
  for (auto &i : s)
    std::cout << i << " ";
  std::cout << ">>" << std::endl;
}

void debug(std::set<int> s) {
  std::cout << "<<";
  for (auto &i : s)
    std::cout << i << " ";
  std::cout << ">>" << std::endl;
}

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

std::set<int> random_pickup(std::set<int> src, int cnt) {
  if (static_cast<int>(src.size()) <= cnt)
    return src;

  double rate = cnt / (double)src.size();
  std::set<int> res;
  for (int i : src)
    if (rand() < rate)
      res.insert(i);

  // if sample too much
  while ((int)res.size() > cnt) {
    auto it(res.begin());
    std::advance(it, rand() % (res.size()));
    res.erase(*it);
  }

  // if need more
  while ((int)res.size() < cnt) {
    auto it(src.begin());
    std::advance(it, rand() % (src.size()));
    res.insert(*it);
  }

  return res;
}

std::vector<size_t> rnd_pick_idx(size_t length, int cnt) {
  std::set<size_t> res;
  while (res.size() < static_cast<size_t>(cnt))
    res.insert(rand() % length);
  std::vector<size_t> resv(res.begin(), res.end());
  return resv;
}

std::set<int> first_N_elements(std::set<int> src, int cnt) {
  std::set<int> res;
  for (int x : src) {
    if (!cnt-- > 0)
      break;
    res.insert(x);
  }
  return res;
}

void print_progress(double percentage) {
  int val = (int)(percentage * 100);
  int lpad = (int)(percentage * PBWIDTH);
  int rpad = PBWIDTH - lpad;
  printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
  fflush(stdout);
}

var_bitset locate_diffs(vbitset_vec_t &inputs) {
  // set locaation bit =1 when there exist diffs in
  var_bitset locations;
  locations.resize(inputs[0].size(), false);
  for (size_t i = 1; i < inputs.size(); i++) {
    auto mask = inputs[i - 1] ^ inputs[i];
    locations |= mask;
  }

  return locations;
}

std::vector<size_t> sort_indexes(const std::vector<double> &v) {

  // initialize original index locations
  std::vector<size_t> idx(v.size());
  iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) { return v[i1] < v[i2]; });

  // sort again to get ridx
  std::vector<size_t> ridx(v.size());
  iota(ridx.begin(), ridx.end(), 0);

  sort(ridx.begin(), ridx.end(),
       [&idx](size_t i1, size_t i2) { return idx[i1] < idx[i2]; });

  return ridx;
}

var_bitset truncate_bitset(var_bitset &truncating, var_bitset &mask) {
  var_bitset res;
  size_t L = mask.count();
  if (L == 0)
    return res;
  res.resize(L);
  size_t cursor = 0;

  size_t i = mask.find_first();
  do {
    res.set(cursor, truncating[i]);
    i = mask.find_next(i);
    cursor++;
  } while (cursor < L);

  return res;
}

size_t hamming_dist(var_bitset &v1, var_bitset &v2) {
  return (v1 ^ v2).count();
}

size_t hash_sizet_vec(std::vector<size_t> input) {
  // https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
  std::size_t seed = input.size();
  for (auto &i : input) {
    seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}