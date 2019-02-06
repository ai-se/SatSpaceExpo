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

/**
 * exterior
 * how many clauses left to be solve when variable set vs are set
 * For FAST
 * LESS THE BETTER
 *
 *  * interior
 * FOR DIVERSITY
 * TODO possible improvement by the following paper
 * "Approximate Counting by Sampling the Backtrack-free Search Space" Gogate et
 * al.
 * LESS THE BETTER
 *
 * @param  vs [description]
 * @return    [description]
 */
std::pair<int, double> program::get_vs_ex_interior(vset_t &vs) {
  cpset_t can_set;
  for (var_t v : vs)
    can_set = can_set + vars2clauses_map[v];
  int exterior = all_clause_ps.size() - can_set.size();

  int sample_repeats = 100;
  double avg_passed = 0;

  for (int i = 0; i < sample_repeats; i++) {
    int passed = 0;
    for (var_t v : vs) {
      passed += rand() > 0.5 ? true_match[v].size() : false_match[v].size();
    }
    avg_passed += (double)passed / can_set.size();
  }

  double interior = 1 - avg_passed / sample_repeats;

  return std::pair<int, double>{exterior, interior};
}

std::vector<vset_t> program::find_kernal_vars(const vset_t &considering) {
  // step 1 - randomly create vset
  double ratio = 0.2;   // TODO set ratio (sample unit size) here
  int samples_cnt = 50; // TODO set sampling size here

  std::vector<vset_t> Xs;
  // std::cout << "Seeking kernal vals ..." << std::endl;
  for (int i = 1; i <= samples_cnt; i++) {
    // print_progress((double)i / samples_cnt); // for debugging...
    vset_t X = random_pickup(considering, (int)(ratio * considering.size()));
    Xs.push_back(std::move(X));
  }
  // std::cout << std::endl;

  // step 2 - get the objectives
  typedef std::pair<int, double> objs_t;
  std::vector<objs_t> objs;
  // std::cout << "Fetching ex/interior objectives ..." << std::endl;
  // double tmp_cnt = 1.0;
  for (auto &X : Xs) {
    // print_progress((tmp_cnt++) / Xs.size()); // for debuging...
    objs.push_back(get_vs_ex_interior(X));
  }
  // std::cout << std::endl;

  // step 3 - get the pareto frontier
  std::set<int> pfs = get_two_objs_PF(objs);

  // debug("end s3");
  // std::cout << pfs.size() << std::endl;
  // step 4 - finalization, create the res
  std::vector<vset_t> res;
  for (int pfi : pfs)
    res.push_back(Xs[pfi]);

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

/* The clauses has been install in the opt*/
z3_model_vec_t program::solve(z3::optimize &opt, vset_t &unsolved_vars,
                              exprs_t &exprs, decls_t &decls, int gen_size) {
  // handling the leaves
  if (static_cast<double>(unsolved_vars.size()) /
          static_cast<double>(vars.size()) <
      0.3) {
    z3_model_vec_t result;
    while (gen_size-- > 0) {
      z3::check_result has_correct = opt.check();
      if (has_correct != z3::sat)
        break;
      z3::model m = opt.get_model();
      result.push_back(m);
      dont_gen_m_again(opt, m, exprs, decls, unsolved_vars);
    }

    return result;
  }

  std::vector<vset_t> kernals = find_kernal_vars(unsolved_vars);
  int varsity_for_each_kernal = 1;
  z3_model_vec_t res;

  for (vset_t &kernal : kernals) {
    opt.push(); // state 1
    for (int r = varsity_for_each_kernal; r > 0; r--) {
      z3::check_result has_correct = opt.check();
      if (has_correct != z3::sat)
        continue;
      opt.push(); // state 2
      z3::model m = opt.get_model();
      /** TODO need to separation?
        to compirm ... the Z3 solver can automatically boost up via given
       some fix variables
       */
      for (var_t v : kernal)
        opt.add(model_of_v(m, v, decls) == Z3_L_TRUE ? exprs.at(v)
                                                     : !exprs.at(v));
      auto rest_vars = unsolved_vars - kernal;
      auto sub_res = solve(opt, rest_vars, exprs, decls,
                           gen_size / (static_cast<int>(kernals.size()) *
                                       varsity_for_each_kernal));
      res.insert(res.end(), sub_res.begin(), sub_res.end());
      opt.pop(); // recover state 2
      dont_gen_m_again(opt, m, exprs, decls, kernal);
    }          // end r-->0
    opt.pop(); // recover state 1
  }            // end for kernal

  return res;
}

z3_model_vec_t program::solve() {
  // create exprs and decls
  exprs_t exprs;
  decls_t decls;
  for (var_t v : vars) {
    z3::expr l =
        c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort());
    exprs.insert(std::make_pair(v, l));
    decls.insert(std::make_pair(v, l.decl()));
  }

  z3::optimize opt(c);
  for (auto &each_clause : clauses) {
    z3::expr_vector V(c);

    for (var_t v : each_clause.vs)
      V.push_back(v > 0 ? exprs.at(v) : !exprs.at(-v));
    opt.add(mk_or(V));
  }

  auto res = solve(opt, vars, exprs, decls, 10000); // TODO change gen_size here

  return res;
}

std::string program::read_model(z3::model &m, decls_t &decls,
                                vset_t &printing_vars) {
  std::string res("");
  for (var_t v : printing_vars)
    res += model_of_v(m, v, decls) == Z3_L_TRUE ? '1' : '0';
  return res;
}

z3_model_vec_t program::gen_N_models(int N) {
  // create exprs and decls
  exprs_t exprs;
  decls_t decls;
  for (var_t v : vars) {
    z3::expr l =
        c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort());
    exprs.insert(std::make_pair(v, l));
    decls.insert(std::make_pair(v, l.decl()));
  }

  z3::optimize opt(c);
  for (auto &each_clause : clauses) {
    z3::expr_vector V(c);

    for (var_t v : each_clause.vs)
      V.push_back(v > 0 ? exprs.at(v) : !exprs.at(-v));
    opt.add(mk_or(V));
  }

  z3_model_vec_t res;
  // double total = static_cast<double>(N);
  // vset_t printed_vars;
  // int x = 30;
  // for (var_tcm i : vars) {
  //   printed_vars.insert(i);
  //   if (x-- < 0)
  //     break;
  // }
  opt.push();
  int N1 = 0;
  // timer T1;
  // do {
  //   z3::check_result has_correct = opt.check();
  //   if (has_correct != z3::sat)
  //     break;
  //   z3::model m = opt.get_model();
  //   // std::cout << N << " : " << read_model(m, decls, printed_vars) <<
  //   // std::endl;
  //   res.push_back(m);
  //   dont_gen_m_again(opt, m, exprs, decls);
  // } while (N1++ < 100);
  // std::cout << N1 << " " << T1.duration() << std::endl;

  /* what if I fix the first half of variables?*/
  opt.pop();
  opt.push();

  // for (int i2 : {1, 10, 100, 250, 500, 1000}) {
  for (int i2 : std::vector<int>(1, 0)) {
    // opt.push();
    int N2 = 0;
    opt.check();
    z3::model m = opt.get_model();
    int x = i2;
    // vset_t to_fronzen_vars = vars;

    // frozen_parial_of_m(opt, m, decls, exprs, to_fronzen_vars);
    timer T2;
    do {
      if (opt.check() != z3::sat)
        break;
      m = opt.get_model();
      res.push_back(m);
      dont_gen_m_again(opt, m, exprs, decls);
      print_progress(static_cast<double>(N2) / 100);
      // std::cout << read_model(m, decls, vars) << std::endl;
    } while (N2++ < 14);
    // std::cout << i2 << "::" << N2 << " " << T2.duration() << " "
    //           << static_cast<double>(T2.duration()) / static_cast<double>(N2)
    //           << std::endl;
    // opt.pop();
    // End of experiment...
  }

  // find out the diffs or the model
  std::cout << std::endl << "starts here" << std::endl;
  vset_t not_diffs;
  vset_t diffs;
  for (var_t v : vars) {
    bool to_insert = true;
    auto tt = model_of_v(res[0], v, decls);
    for (auto &m : res)
      if (model_of_v(m, v, decls) != tt) {
        to_insert = false;
        break;
      }
    if (to_insert)
      not_diffs.insert(v);
    else
      diffs.insert(v);
  }

  // fronzen the not_diffs
  opt.pop();
  int repeat = 0;
  std::cout << not_diffs.size() << " " << diffs.size() << std::endl;
  frozen_parial_of_m(opt, res[10], decls, exprs, not_diffs);
  timer NN;
  while (repeat++ < 100) {
    std::cout << '.';
    if (opt.check() != z3::sat)
      break;
    z3::model m = opt.get_model();
    dont_gen_m_again(opt, m, exprs, decls, diffs);
  }
  std::cout << repeat << std::endl;
  std::cout << NN.duration() << std::endl;
  return res;
}
