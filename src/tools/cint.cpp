#include "commons/udg/UDG.h"
#include "commons/utility/utility.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <string.h>
#include <typeinfo>
#include <unordered_set>
#include <vector>
#include <z3++.h>

z3::context c;
z3::expr literal(int v) {
  return c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort());
}

class ClauseStr {
public:
  std::vector<int> vs;

  ClauseStr(std::string s, std::vector<int> &ind, UDG<int> &connections) {
    std::istringstream iss(s);
    int v;
    while (!iss.eof()) {
      iss >>
          v; // if (std::find(ind.begin(), ind.end(), abs(v)) != ind.end() && v)
      if (v) {
        vs.push_back(v);
        connections.add_node(abs(v));
      }
    }
    std::sort(vs.begin(), vs.end());
    for (size_t i = 1; i < vs.size(); ++i)
      connections.add_edge(abs(vs[0]), abs(vs[i]));
  }

  z3::expr toExpr() {
    z3::expr_vector clause(c);
    for (int v : vs) {
      if (v > 0)
        clause.push_back(literal(v));
      else
        clause.push_back(!literal(-v));
    }
    return mk_or(clause);
  }

  std::size_t unique() const {
    // stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
    std::size_t seed = vs.size();
    for (auto &i : vs) {
      seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }

  bool empty() { return vs.size() <= 1; }
  friend std::ostream &operator<<(std::ostream &os, const ClauseStr &me);
};

std::ostream &operator<<(std::ostream &os, const ClauseStr &me) {
  for (int v : me.vs)
    os << v << " ";
  return os;
}

class StrongConnected {
  std::string input_file;
  std::vector<int> ind;

  z3::optimize opt;

public:
  StrongConnected(std::string input) : input_file(input), opt(c) {}

  void parse_cnf() {
    z3::expr_vector exp(c);
    std::ifstream f(input_file);
    if (!f.is_open()) {
      std::cout << "Error opening input file\n";
      abort();
    }
    std::unordered_set<int> indset;
    bool has_ind = false;
    int max_var = 0;
    std::string line;
    std::unordered_set<size_t> css;
    UDG<int> connections;
    while (getline(f, line)) {
      std::istringstream iss(line);
      if (line.find("c ind ") == 0) {
        std::string s;
        iss >> s;
        iss >> s;
        int v;
        while (!iss.eof()) {
          iss >> v;
          if (v && indset.find(v) == indset.end()) {
            indset.insert(v);
            ind.push_back(v);
            has_ind = true;
          }
        }
      } else if (line[0] != 'c' && line[0] != 'p') {
        ClauseStr clause(line, ind, connections);
        size_t tmp;
        if (!clause.empty() &&
            std::find(css.begin(), css.end(), (tmp = clause.unique())) ==
                css.end()) {
          css.insert(tmp);
        }
      }
    } // getline

    f.close();
    if (!has_ind) {
      for (int lit = 0; lit <= max_var; ++lit) {
        if (indset.find(lit) != indset.end()) {
          ind.push_back(lit);
        }
      }
    }
    z3::expr formula = mk_and(exp);
    opt.add(formula);
    std::cout << "start checking" << std::endl;

    timer tmp_timer;
    std::cout << "connections# " << connections.count_component_nums()
              << std::endl;
    std::cout << tmp_timer.duration() << std::endl;
  } // parse_cnf
};

int main(int argc, char *argv[]) {
  // StrongConnected s(argv[1]);
  StrongConnected s("Benchmarks/ConcreteActivityService.sk_13_28.cnf");
  std::cout << "running in cint." << std::endl;
  s.parse_cnf();
}

// int main(int argc, char *artv[]) {
//   std::cout << "running in backup cint module." << std::endl;
//   UDG<int> graph;
//   graph.add_node(1);
//   graph.add_node(2);
//   graph.add_node(3);
//
//   graph.add_edge(1, 2);
//   graph.add_edge(2, 3);
//
//   std::cout << graph.count_component_nums() << std::endl;
// }
