#pragma once

#include <boost/dynamic_bitset.hpp>

#include <map>
#include <set>
#include <string.h>
#include <unordered_set>
#include <vector>
#include <z3++.h>

struct clause_t {
  std::vector<int> vs;
  std::set<int> avs;

  clause_t(std::string s, std::set<int> &indv);

  std::size_t unique() const;
  bool empty() { return vs.size() <= 1; }
  int get_determined();

  boost::dynamic_bitset<> mask, reversed;

  friend std::ostream &operator<<(std::ostream &, const clause_t &);
};
