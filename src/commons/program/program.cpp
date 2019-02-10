#include "commons/utility/utility.h"
#include "program.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <math.h>
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

  vars_num = vars.size();
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

/**
 * NOTICE All operations are based on Boost::dynamic_bitset
 * @param  samples  [description]
 * @param  consider [description]
 * @return          [description]
 */
bin_tree_node *program::create_sub_guide_tree(vbitset_vec_t &samples,
                                              var_bitset &consider) {
  bin_tree_node *subroot = new bin_tree_node(consider);
  // std::cout << consider.count() << std::endl;
  if (consider.count() < sqrt(vars_num)) // TODO set par here
    return subroot;

  // spliting the left and right basing on the samples
  /* s1. randomly pick up one variable */
  size_t Ri;
  do {
    Ri = rand() % vars_num;
  } while (!consider[Ri]);
  
  /* s2. for each var. cal the diverse measure */
  std::map<size_t, int> t_diverse; // diverse measure when Ri set to true
  std::map<size_t, int> f_diverse;

  // O(n^2) complexity here?

  int case_count = 0; // marking how many sample with Ri set
  for (auto &sample : samples) {
    if (sample[Ri])
      case_count += 1;
  }

  int case_f_count =
      samples.size() - case_count; // how many sampel with Ri false

  for (size_t i = 0; i < consider.size(); i++) {
    if (!consider[i] || i == Ri)
      continue;

    t_diverse[i] = 0; // init
    f_diverse[i] = 0;

    for (auto &sample : samples) {
      if (sample[Ri] && sample[i])
        t_diverse[i] += 1;
      else if (!sample[Ri] && sample[i])
        f_diverse[i] += 1;
    }
  }

  /*
  up to now, when Ri set,
  var i has {t_diverse[i]} (T) over {case_count-t_diverse[i]} (F)

  when Ri false
  var i has {f_diverse[i]} (T) over {case_f_count-f_diverse[i]} (F)

  diverse measure of i should be (abs(T-F)/case_count,
      abs(T'-F')/(case_f_count))

   */
  std::vector<double> measure_t;
  std::vector<double> measure_f;

  for (size_t i = 0; i < consider.size(); i++) {
    if (!consider[i]) {
      measure_t.push_back(-1);
      measure_f.push_back(-1);
    }
    if (i == Ri) {
      measure_t.push_back(0);
      measure_f.push_back(0);
      continue;
    }

    measure_t.push_back(
        static_cast<double>(std::abs(2 * t_diverse[i] - case_count)) /
        static_cast<double>(case_count));
    measure_f.push_back(
        static_cast<double>(std::abs(2 * f_diverse[i] - case_f_count)) /
        static_cast<double>(case_f_count));
  }

  /* s3. splitting based on the measures */
  var_bitset left_consider, right_consider;
  left_consider.resize(consider.size(), false);  // init
  right_consider.resize(consider.size(), false); // init

  auto t_indexes = sort_indexes(measure_t);
  auto f_indexes = sort_indexes(measure_f);
  auto index_base = consider.count();

  for (size_t i = 0; i < consider.size(); i++) {
    if (!consider[i])
      continue;
    if (t_indexes[i] + f_indexes[i] - 2 * index_base > index_base)
      right_consider.set(i, true);
    else
      left_consider.set(i, true);
  }

  subroot->left = create_sub_guide_tree(samples, left_consider);
  subroot->right = create_sub_guide_tree(samples, right_consider);
  return subroot;
}

btree program::create_mutate_guide_tree() {
  vbitset_vec_t samples = gen_N_models(10); // TODO set here
  var_bitset mask = locate_diffs(samples);
  btree res;
  res.root = create_sub_guide_tree(samples, mask);
  return res;
}
