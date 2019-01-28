#include "clause.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>

// z3::context c;
// z3::expr literal(int v) {
//   return c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort());
// }

clause::clause(std::string s) {
  std::istringstream iss(s);
  int v;
  while (!iss.eof()) {
    iss >> v;
    if (v) {
      vs.push_back(v);
      avs.insert(abs(v));
    }
  }
  std::sort(vs.begin(), vs.end());
}

// z3::expr clause::toExpr(z3::context &c) {
//   // z3::context c;
//   z3::expr_vector res(c);
//   for (int v : vs) {
//     z3::expr l = c.constant(
//         c.str_symbol(std::to_string(v > 0 ? v : -v).c_str()), c.bool_sort());
//     if (v > 0)
//       res.push_back(l);
//     else
//       res.push_back(!l);
//     // if (v > 0)
//     //   res.push_back(literal(v));
//     // else
//     //   res.push_back(!literal(-v));
//   }
//
//   return mk_or(res);
// }

std::size_t clause::unique() const {
  // https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
  std::size_t seed = vs.size();
  for (auto &i : vs) {
    seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

/**
 * return 0 if there is no determined values
 * return +/- value for the unique determined values
 * @return [description]
 */
int clause::get_determined() {
  if (vs.size() != 1)
    return 0;
  else
    return vs[0];
}

std::ostream &operator<<(std::ostream &os, const clause &me) {
  for (int v : me.vs)
    os << v << " ";
  return os;
}
