#include "commons/utility/utility.h"
#include "program.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <z3++.h>

cpset_t operator-(const cpset_t &A, const cpset_t &B) {
  cpset_t remain;
  for (auto &i : A)
    if (B.find(i) == B.end())
      remain.insert(i);
  return remain;
}

cpset_t operator+(const cpset_t &A, const cpset_t &B) {
  cpset_t sum;
  for (auto &i : A)
    sum.insert(i);
  for (auto &j : B)
    sum.insert(j);

  return sum;
}

vset_t operator-(const vset_t &A, const vset_t &B) {
  vset_t remain;
  for (auto i : A)
    if (B.find(i) == B.end())
      remain.insert(i);
  return remain;
}

std::istream &operator>>(std::istream &is, vset_t &obj) {
  var_t i;
  obj.clear();
  while (is >> i)
    obj.insert(i);
  return is;
}

std::ostream &operator<<(std::ostream &os, const vset_t &obj) {
  for (var_t i : obj)
    os << i << " ";
  return os;
}

/**
 * Parsing the cnf file as the program.
 * Does not return anything set up in the private variables
 * @param input_file name of cnf file
 */
program::program(std::string input_file) {
  z3::context c;
  z3::expr_vector exp(c);
  std::ifstream f(input_file);
  if (!f.is_open()) {
    std::cout << "Error: No such file " << input_file << std::endl;
    abort();
  }
  std::string line;
  while (getline(f, line)) {
    if (line.find("c ind ") == 0) {
      // TODO is independent var valid?
    } else if (line.find("p cnf") == 0) {
      std::istringstream iss(line);
      std::string s_dump;
      iss >> s_dump >> s_dump; // 'p' 'cnf'
      iss >> vars_num;         // p value
      std::cout << "INFO : |vars| = " << vars_num << std::endl;
    } else if (line[0] != 'c' && line[0] != 'p') {
      clause cline(line);
      clauses.push_back(cline);
    }
  }

  // set up the vars2clauses_map
  for (clause &each_clause : clauses)
    for (var_t v : each_clause.vs) {
      vars2clauses_map[std::abs(v)].insert(&each_clause);
      if (v > 0)
        true_match[v].insert(&each_clause);
      else
        false_match[-v].insert(&each_clause);
      vars.insert(std::abs(v));
    }

  // save to all_clause_ps
  for (auto &each_clause : clauses)
    all_clause_ps.insert(&each_clause);

  std::cout << "INIT : Loading " << input_file << " done." << std::endl;
}

vset_t program::get_clauses_defined_vars(cpset_t &css) {
  vset_t res;
  for (auto &each_clause_p : css)
    for (var_t i : each_clause_p->avs)
      res.insert(i);
  return res;
}

cpset_t program::get_vars_defined_clauses(vset_t &vs) {
  cpset_t res;
  for (var_t v : vs)
    res = res + vars2clauses_map[v];
  return res;
}

Z3_lbool program::model_of_v(z3::model &model, var_t v, decls_t &decls) {
  z3::expr b = model.get_const_interp(decls.at(v));
  return b.bool_value(); // Z3_L_TRUE(1) Z3_L_UNDEF(0) Z3_L_FALSE(-1)
}

void program::dont_gen_m_again(z3::optimize &opt, z3::model &m, exprs_t &exprs,
                               decls_t &decls, const vset_t &considered_vars) {
  z3::expr_vector V(c);

  for (var_t v : considered_vars)
    V.push_back(model_of_v(m, v, decls) == Z3_L_TRUE ? !exprs.at(v)
                                                     : exprs.at(v));
  opt.add(mk_or(V));
}

void program::frozen_parial_of_m(z3::optimize &opt, z3::model &m,
                                 decls_t &decls, exprs_t &exprs,
                                 vset_t &to_fronzen_vars) {
  for (var_t v : to_fronzen_vars)
    opt.add(model_of_v(m, v, decls) == Z3_L_TRUE ? exprs.at(v) : !exprs.at(v));
}

std::string program::read_model(z3::model &m, decls_t &decls,
                                vset_t &printing_vars) {
  std::string res("");
  for (var_t v : printing_vars)
    res += model_of_v(m, v, decls) == Z3_L_TRUE ? '1' : '0';
  return res;
}

vbitset_vec_t program::gen_N_models(int N) {
  // create exprs and decls
  exprs_t exprs;
  decls_t decls;
  for (var_t v : vars) {
    z3::expr l =
        c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort());
    exprs.insert(std::make_pair(v, l));
    decls.insert(std::make_pair(v, l.decl()));
  }
  // END create exprs and decls

  // load the whole model
  z3::optimize opt(c);
  for (auto &each_clause : clauses) {
    z3::expr_vector V(c);

    for (var_t v : each_clause.vs)
      V.push_back(v > 0 ? exprs.at(v) : !exprs.at(-v));
    opt.add(mk_or(V));
  }
  // END load the whole model

  vbitset_vec_t res;
  double total = static_cast<double>(N);
  std::cout << "Generating " << N << " models" << std::endl;
  while (N-- > 0) {
    print_progress(1 - N / total);
    z3::check_result has_correct = opt.check();
    if (has_correct != z3::sat)
      break;
    z3::model m = opt.get_model();
    res.push_back(var_bitset(read_model(m, decls)));
    dont_gen_m_again(opt, m, exprs, decls);
  }
  std::cout << std::endl;
  return res;
}

// bin_tree_node *program::create_sub_guide_tree(vbitset_vec_t &samples,
//                                               vset_t &consider) {
//   bin_tree_node *subroot = new bin_tree_node(consider);
//   // spliting the left and right basing on the samples
//   // s1. randomly pick up one variable
//
//   // end of spliting
//
//   subroot->left = create_sub_guide_tree(samples, TODO1);
//   subroot->right = create_sub_guide_tree(samples, TODO2);
//   return subroot;
// }
//
// btree program::create_flip_guid_tree() {
//   vbitset_vec_t samples = get_N_models(100); // TODO set here
//   // find any diffs
//
// }
