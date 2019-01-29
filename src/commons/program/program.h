#pragma once

#include "commons/clause/clause.h"
#include "commons/dudg/DUDG.h"
#include "commons/udg/UDG.h"
#include <map>
#include <set>
#include <string.h>
#include <vector>
#include <z3++.h>

typedef int var_t;                  // variable
typedef std::set<clause *> cpset_t; // clause point set
typedef std::set<var_t> vset_t;     // variable set
typedef std::map<var_t, z3::expr> exprs_t;
typedef std::map<var_t, z3::func_decl> decls_t;
typedef std::vector<z3::model> z3_model_vec_t;

cpset_t operator-(const cpset_t &A, const cpset_t &B);
cpset_t operator+(const cpset_t &A, const cpset_t &B); // the union
vset_t operator-(const vset_t &A, const vset_t &B);

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

  std::vector<vset_t> find_kernal_vars(const vset_t &considering);
  std::vector<vset_t> find_kernal_vars() { return find_kernal_vars(vars); }

  z3_model_vec_t solve(z3::optimize &opt, vset_t &unsolved_vars, exprs_t &exprs,
                       decls_t &decls, int gen_size);

public:
  int vars_num;
  program(std::string input_file);
  z3_model_vec_t solve();
  z3_model_vec_t gen_N_models(int N);
};
