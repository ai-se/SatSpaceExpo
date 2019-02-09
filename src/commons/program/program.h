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
cpset_t operator+(const cpset_t &A, const cpset_t &B); // the union

vset_t operator-(const vset_t &A, const vset_t &B);
std::istream &operator>>(std::istream &is, vset_t &obj);
std::ostream &operator<<(std::ostream &os, const vset_t &obj);

class program {
  z3::context c;
  std::vector<clause> clauses;
  std::map<var_t, cpset_t> vars2clauses_map;
  std::map<var_t, cpset_t> true_match,
      false_match;       // if var of var_t(key) set to true/ false,
                         // then set of caluses can be matched
                         // DUDG<var_t> c_udg;
  cpset_t all_clause_ps; // all clause pointers
  vset_t vars;           // all variables

  std::pair<int, double> get_vs_ex_interior(vset_t &vs);
  vset_t get_clauses_defined_vars(cpset_t &css);
  cpset_t get_vars_defined_clauses(vset_t &vs);

  Z3_lbool model_of_v(z3::model &model, var_t v, decls_t &decls);
  void dont_gen_m_again(z3::optimize &opt, z3::model &m, exprs_t &exprs,
                        decls_t &decls, const vset_t &considered_vars);
  void dont_gen_m_again(z3::optimize &opt, z3::model &m, exprs_t &exprs,
                        decls_t &decls) {
    dont_gen_m_again(opt, m, exprs, decls, vars);
  }
  void frozen_parial_of_m(z3::optimize &opt, z3::model &m, decls_t &decls,
                          exprs_t &exprs, vset_t &to_fronzen_vars);

  std::string read_model(z3::model &m, decls_t &decls, vset_t &printing_vars);
  std::string read_model(z3::model &m, decls_t &decls) {
    return read_model(m, decls, vars);
  }

  bin_tree_node *create_sub_guide_tree(vbitset_vec_t &samples,
                                       var_bitset &consider);

public:
  int vars_num;
  program(std::string input_file);
  vbitset_vec_t gen_N_models(int N);
  btree create_mutate_guide_tree();
};
