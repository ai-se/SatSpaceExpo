#pragma once

#include "commons/bintree/binaryTree.h"
#include "commons/clause/clause.h"
#include "commons/dudg/DUDG.h"
#include "commons/udg/UDG.h"
#include "commons/utility/utility.h"
#include <iostream>
#include <map>
#include <set>
#include <string.h>
#include <vector>
#include <z3++.h>

cpset_t operator-(const cpset_t &A, const cpset_t &B);
cpset_t &operator-=(cpset_t &lhs, const cpset_t &rhs);
cpset_t &operator+=(cpset_t &lhs, const cpset_t &rhs);
cpset_t operator+(const cpset_t &A, const cpset_t &B); // the union

vset_t operator-(const vset_t &A, const vset_t &B);
std::istream &operator>>(std::istream &is, vset_t &obj);
std::ostream &operator<<(std::ostream &os, const vset_t &obj);

struct program {
  z3::context c;
  std::map<var_t, cpset_t> vars2clauses_map;
  std::map<var_t, cpset_t> true_match,
      false_match;       // if var of var_t(key) set to true/ false,
                         // then set of caluses can be matched
                         // DUDG<var_t> c_udg;
  cpset_t all_clause_ps; // all clause_t pointers
  std::map<var_t, size_t> var_bit_id;
  int vars_num;
  std::vector<clause_t> clauses;
  vset_t vars; // all variables
  vset_t indv;
  timer solver_clock;
  int global_sampled;
  int cc = 0;

  program(std::string input_file);
  std::set<var_bitset> solve(vbitset_vec_t &samples, std::ofstream &ofs,
                             double max_time);
  void mutate_the_seed_with_tree(btree &tree, var_bitset &seed,
                                 std::set<var_bitset> &results_container,
                                 vbitset_vec_t &next_samples,
                                 std::ofstream &ofs, z3::optimize &opt,
                                 decls_t &decls, exprs_t &exprs);
  cpset_t related_cluases(var_bitset &indicator);

  // Tools
  Z3_lbool model_of_v(z3::model &model, var_t v, decls_t &decls);
  void dont_gen_m_again(z3::optimize &opt, z3::model &m, exprs_t &exprs,
                        decls_t &decls, const vset_t &considered_vars);
  void dont_gen_m_again(z3::optimize &opt, z3::model &m, exprs_t &exprs,
                        decls_t &decls) {
    dont_gen_m_again(opt, m, exprs, decls, vars);
  }
  void frozen_parial_of_m(z3::optimize &opt, z3::model &m, decls_t &decls,
                          exprs_t &exprs, vset_t &to_fronzen_vars);
  void frozen_parial_of_vbit(z3::optimize &opt, var_bitset &v, decls_t &decls,
                             exprs_t &exprs, var_bitset &to_fronzen_vars);

  var_bitset read_model(z3::model &m, decls_t &decls, vset_t &printing_vars);
  var_bitset read_model(z3::model &m, decls_t &decls) {
    return read_model(m, decls, vars);
  }
  vbitset_vec_t gen_N_models(int N);
  bool verify_var_bitset(const var_bitset &vbt, cpset_t &toverify);
  // End of tools
};
