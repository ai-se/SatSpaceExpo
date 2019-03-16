#include "program.h"
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

cpset_t &operator-=(cpset_t &lhs, const cpset_t &rhs) {
  for (auto i : rhs)
    lhs.erase(i);
  return lhs;
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
      std::istringstream iss(line);
      std::string s;
      iss >> s;
      iss >> s;
      int v;
      while (!iss.eof()) {
        iss >> v;
        if (v)
          indv.insert(v);
      }
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

  // std::cout << "INFO : |vars| = " << vars_num << std::endl;
  // std::cout << "INIT : Loading " << input_file << " done." << std::endl;
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

bool program::verify_var_bitset(const var_bitset &vbt, cpset_t &toverify) {
  for (auto cp : toverify) {
    bool pass = false;
    for (auto &v : cp->vs) {
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

void program::mutate_the_seed_with_tree(btree &tree, var_bitset &seed,
                                        std::set<var_bitset> &results_container,
                                        std::ofstream &ofs) {
  // create the fast verification memo, set the root first.
  tree.root->should_verify = all_clause_ps;
  for (var_t v : vars) {
    size_t i = var_bit_id[v];
    if (!tree.root->union_delta.test(i))
      tree.root->should_verify -= seed.test(i) ? true_match[v] : false_match[v];
  }

  tree.traverse(TRA_T_PRE_ORDER, [&](bin_tree_node *node) {
    if (node == tree.root) // already done
      return;

    node->should_verify =
        node->parent->should_verify; // substraction starting from the parent
    auto further_sub = node->union_delta ^ node->parent->union_delta;

    for (var_t v : vars) {
      size_t i = var_bit_id[v];
      if (further_sub.test(i))
        node->should_verify -= seed.test(i) ? true_match[v] : false_match[v];
    }
  });
  // END of creating memo

  // the mutation
  int cc = 0;
  for (size_t i = 0; i < 100; i++) {
    global_sampled++;
    auto idx = tree.rnd_pick_idx_based_on_probability(rand() % 2 + 2); // 2 or 3
    auto mask = tree.deltas[idx[0]];
    for (auto idxi : idx)
      mask |= tree.deltas[idxi];
    auto gen = seed ^ mask;
    if (verify_var_bitset(gen, tree.find_share_parent(idx)->should_verify)) {
      results_container.insert(gen);
      ofs << gen << std::endl;
      cc += 1;
    }
  }
  ofs << "# " << solver_clock.duration() << " " << results_container.size()
      << " / " << global_sampled << std::endl;
  // std::cout << cc << std::endl;
}

std::set<var_bitset> program::solve(vbitset_vec_t &samples,
                                    std::ofstream &ofs) {
  // std::map<var_bitset, int> deltas;
  // for (size_t i = 0; i < samples.size(); i++)
  //   for (size_t j = i + 1; j < samples.size(); j++) {
  //     var_bitset d = samples[i] ^ samples[j];
  //     if (deltas.find(d) != deltas.end())
  //       deltas[d] += 1;
  //     else
  //       deltas[d] = 1;
  //   }

  // std::vector<var_bitset> deltas_vec;

  // for (auto &D : deltas) {
  //   if (D.second > 13)
  //     deltas_vec.push_back(D.first);
  // }

  // int cc = 0;
  // std::set<var_bitset> ALL;
  // for (int i = 0; i < 100; i++) {
  //   // rand pick up two elements
  //   auto ee = rnd_pick_idx(deltas_vec.size(), 2);
  //   auto gen = samples[rand() % samples.size()] ^
  //              (deltas_vec[ee[0]] & deltas_vec[ee[1]]);
  //   if (!ALL.count(gen))
  //     ALL.insert(gen);
  //   else
  //     continue;
  //   if (verify_var_bitset(gen, all_clause_ps))
  //     cc += 1;
  // }
  // std::cout << cc / 100.0 << std::endl;
  // return;

  solver_clock.startnow();
  global_sampled = 0;
  btree T = btree(samples);
  std::set<var_bitset> results;
  for (int i = 0; i < 100; i++)
    mutate_the_seed_with_tree(T, samples[rand() % samples.size()], results,
                              ofs);
  std::cout << "found valid unqiue # " << results.size() << std::endl;
  return results;
}