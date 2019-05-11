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

cpset_t &operator+=(cpset_t &lhs, const cpset_t &rhs) {
  for (auto i : rhs)
    lhs.insert(i);
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

cpset_t program::related_cluases(var_bitset &indicator) {
  cpset_t res;
  size_t i = 0;
  for (var_t v : vars) {
    if (!indicator.test(i)) {
      auto toadd = vars2clauses_map.at(v);
      res.insert(toadd.begin(), toadd.end());
    }
    i++;
  }
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

void program::frozen_parial_of_vbit(z3::optimize &opt, var_bitset &v,
                                    decls_t &decls, exprs_t &exprs,
                                    var_bitset &to_fronzen_vars) {
  size_t i = 0;
  for (var_t var : vars) {
    if (to_fronzen_vars.test(i))
      opt.add(v.test(i) ? exprs.at(var) : !exprs.at(var));
    i++;
  }
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
  // std::cout << "Generating " << N << " models" << std::endl;
  while (N-- > 0) {
    print_progress(1 - N / total);
    z3::check_result has_correct = opt.check();
    if (has_correct != z3::sat)
      break;
    z3::model m = opt.get_model();
    res.push_back(read_model(m, decls));
    dont_gen_m_again(opt, m, exprs, decls);
  }
  // std::cout << std::endl;
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
                                        vbitset_vec_t &next_samples,
                                        std::ofstream &ofs, z3::optimize &opt,
                                        decls_t &decls, exprs_t &exprs) {
  // the mutation
  std::set<size_t> idx_hash_memo;
  idx_hash_memo.clear();
  int life = 5;
  while (life > 0) {
    size_t prev_found = results_container.size();
    global_sampled++;
    auto idx = tree.rnd_pick_idx_based_on_probability(2); // TODO 2 or 3
    size_t tmp_hash = hash_sizet_vec(idx);
    if (idx_hash_memo.count(tmp_hash))
      continue;
    idx_hash_memo.insert(tmp_hash);
    auto mask = tree.deltas[idx[0]];
    for (auto idxi : idx)
      mask |= tree.deltas[idxi];
    auto gen = seed ^ mask;
    auto parent = tree.find_share_parent(idx);
    if (verify_var_bitset(gen, parent->should_verify)) {
      results_container.insert(gen);
      ofs << gen << std::endl;
      cc += 1;
    } else {
      // fixing via relaxing
      opt.push();
      frozen_parial_of_vbit(opt, gen, decls, exprs, mask);
      size_t trial = 0;
      while (opt.check() == z3::sat && trial++ < 5) {
        z3::model m = opt.get_model();
        auto fix_gen = read_model(m, decls);
        // if (!results_container.count(fix_gen))
        //   std::cout << "contribute new " << std::endl;
        results_container.insert(fix_gen);
        ofs << fix_gen << std::endl;
        if (trial == 1)
          next_samples.push_back(fix_gen);
        cc += 1;
        dont_gen_m_again(opt, m, exprs, decls);
      }
      opt.pop();

      // if (trial != 0)
      //   std::cout << "pass check" << std::endl;
      // else
      //   std::cout << "FAIL check" << std::endl;
    }

    if (results_container.size() == prev_found)
      life--;
  } // end for i
}

std::set<var_bitset> program::solve(vbitset_vec_t &samples, std::ofstream &ofs,
                                    double max_time) {
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

  solver_clock.startnow();
  global_sampled = 0;
  std::set<var_bitset> results;

  vbitset_vec_t S = samples;
  // while (true) {
  while (solver_clock.duration() < max_time) {
    std::cout << results.size() << std::endl;
    btree tree = btree(S);
    // create the fast verification memo
    tree.traverse(TRA_T_POST_ORDER, [&](bin_tree_node *node) {
      if (!node->left && !node->right) { // the leaves
        for (var_bitset *dp : node->delta_ps) {
          auto delta = *dp;
          for (var_t v : vars)
            if (delta.test(var_bit_id[v]))
              node->should_verify += vars2clauses_map[v];
        }
      } else if (node->left)
        node->should_verify += node->left->should_verify;
      else if (node->right)
        node->should_verify += node->right->should_verify;
    });
    // END of creating memo
    vbitset_vec_t next_samples;
    for (auto &sam : S) {
      mutate_the_seed_with_tree(tree, sam, results, next_samples, ofs, opt,
                                decls, exprs);
      ofs << "# " << solver_clock.duration() << " " << results.size() << " / "
          << global_sampled << std::endl;
    }

    if (next_samples.empty())
      break;

    next_samples.insert(next_samples.end(), S.begin(), S.end());
    S.clear();
    for (auto is : rnd_pick_idx(next_samples.size(), 100))
      S.push_back(next_samples[is]);
  }
  std::cout << "found valid unqiue # " << results.size() << std::endl;
  std::cout << "cc = " << cc << std::endl;
  return results;
}