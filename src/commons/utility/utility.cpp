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

/**
 * ALL LESS THE BETTER!!
 * Brute force...
 * Binary domination
 * @param candidates The objective pair candidates
 * TODO efficiency improvement
 *
 * @return the index that contributes to the paretor frontier
 */
std::set<int> get_two_objs_PF(std::vector<std::pair<int, double>> candidates) {
  std::set<int> pf_indices; // pareto frontier
  for (size_t i = 0; i < candidates.size();
       i++) { // for each candidate, check whether it
              // is dominated by any other candidate
    bool dominated = false;
    for (size_t j = 0; j < candidates.size(); j++) {
      if (i == j)
        continue;
      if (candidates[i].first > candidates[j].first &&
          candidates[i].second > candidates[j].second) {
        dominated = true;
        break;
      } // end if dominated
    }   // end checking

    if (!dominated)
      pf_indices.insert(i);
  }
  return pf_indices;
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
