#pragma once

#include <map>
#include <set>
#include <string.h>
#include <unordered_set>
#include <vector>
#include <z3++.h>

struct clause {
  std::vector<int> vs;
  std::set<int> avs;

  clause(std::string s);
  z3::expr toExpr(z3::context &c);

  std::size_t unique() const;
  bool empty() { return vs.size() <= 1; }
  int get_determined();

  friend std::ostream &operator<<(std::ostream &, const clause &);
};
