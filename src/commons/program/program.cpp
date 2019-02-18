#include "program.h"
#include "alglib/dataanalysis.h"
#include "alglib/stdafx.h"
#include "commons/utility/utility.h"
#include <algorithm>
#include <boost/range/irange.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
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
      // std::istringstream iss(line);
      // std::string s_dump;
      // iss >> s_dump >> s_dump; // 'p' 'cnf'
      // iss >> vars_num;         // p value
      // std::cout << "INFO : |vars| = " << vars_num << std::endl;
    } else if (line[0] != 'c' && line[0] != 'p') {
      clause_t cline(line);
      clauses.push_back(cline);
    }
  }

  // set up the vars2clauses_map
  for (clause_t &clause : clauses)
    for (var_t v : clause.vs) {
      vars2clauses_map[std::abs(v)].insert(&clause);
      if (v > 0)
        true_match[v].insert(&clause);
      else
        false_match[-v].insert(&clause);
      vars.insert(std::abs(v));
    }

  // save to all_clause_ps
  for (auto &clause : clauses)
    all_clause_ps.insert(&clause);

  // mark var_bit_id
  {
    size_t i = 0;
    for (var_t v : vars)
      var_bit_id[v] = i++;
  }

  vars_num = vars.size();

  // setting up the clause mask and reversed
  for (auto &clause : clauses) {
    clause.mask.resize(vars_num, false);
    clause.reversed.resize(vars_num);
    for (var_t v : clause.vs) {
      clause.mask.set(var_bit_id[std::abs(v)]);
      clause.reversed.set(var_bit_id[std::abs(v)], !(v > 0));
    }
  }
  // end..

  std::cout << "INFO : |vars| = " << vars_num << std::endl;
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

var_bitset program::read_model(z3::model &m, decls_t &decls,
                               vset_t &printing_vars) {
  var_bitset res;
  res.resize(printing_vars.size());
  size_t cursor = 0;
  for (var_t v : printing_vars)
    res.set(cursor++, model_of_v(m, v, decls) == Z3_L_TRUE);
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
  for (auto &clause : clauses) {

    z3::expr_vector V(c);

    for (var_t v : clause.vs)
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
    res.push_back(read_model(m, decls));
    dont_gen_m_again(opt, m, exprs, decls);
  }
  std::cout << std::endl;
  return res;
}

bool program::verify_var_bitset(const var_bitset &vbt) {
  for (size_t i = 0; i < clauses.size(); i++) {
    // if (!((vbt & clauses[i].mask) ^ clauses[i].reversed).any())
    //   return false;
    bool pass = false;
    for (auto &v : clauses[i].vs) {
      if (vbt[var_bit_id[std::abs(v)]] == bool(v > 0)) {
        pass = true;
        break;
      }
    }
    if (!pass)
      return false;
  } // end for ith clause
  return true;
}

bin_tree_node *program::create_sub_guide_tree(std::set<var_bitset> deltas) {
  bin_tree_node *subroot = new bin_tree_node;
  subroot->deltas = std::move(deltas);
  // TODO set leaf condition here!
  if (subroot->deltas.size() < 10)
    return subroot;

  // run alglib k-means++ to separte the deltas into two groups
  alglib::clusterizerstate s; // alglib::ahcreport rep;
  alglib::kmeansreport rep;
  alglib::ae_int_t disttype = 2;
  alglib::real_2d_array xy;
  xy.setlength(subroot->deltas.size(), subroot->deltas.begin()->size());

  size_t di = 0;
  for (auto &d : subroot->deltas) {
    for (size_t j = 0; j < d.size(); j++)
      xy(di, j) = d.test(j) ? 1 : 0;
    di++;
  }

  alglib::clusterizercreate(s);
  alglib::clusterizersetpoints(s, xy, disttype);
  alglib::clusterizerrunkmeans(s, 2, rep);
  // END of k-menas++. result stored in rep.cidx

  std::set<var_bitset> ldeltas, rdeltas;
  di = 0;
  for (auto &d : subroot->deltas) {
    if (rep.cidx[di++] == 1)
      ldeltas.insert(d);
    else
      rdeltas.insert(d);
  }

  subroot->left = create_sub_guide_tree(ldeltas);
  subroot->right = create_sub_guide_tree(rdeltas);
  return subroot;
}

btree program::create_mutate_guide_tree(vbitset_vec_t &samples) {
  std::set<var_bitset> deltas;
  for (size_t i = 0; i < samples.size(); i++)
    for (size_t j = i + 1; j < samples.size(); j++)
      deltas.insert(samples[i] ^ samples[j]);
  btree res;
  res.root = create_sub_guide_tree(deltas);
  return res;
}

void program::mutate_the_seed_with_tree(btree &tree, var_bitset &seed,
                                        vbitset_vec_t &samples) {
  if (!tree.node_union_inter_delta_set) {
    tree.node_union_inter_delta_set = true;
    // marking the union and intersection var_bitste for each tree node
    tree.traverse(TRA_T_POST_ORDER, [&](bin_tree_node *node) {
      node->union_delta.resize(vars_num, false);
      node->intersection_delta.resize(vars_num, true);
      for (auto &d : node->deltas) {
        node->union_delta |= d;
        node->intersection_delta &= d;
      }
    });
  } // end of setting union and intersection for each node

  tree.traverse(TRA_T_PRE_ORDER, [&](bin_tree_node *node) {
    std::cout << node->union_delta.count() << " "
              << node->intersection_delta.count() << std::endl;
  });
}
