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

// cpset_t exclude_by_priori_knowledge(cpset_t &base, vset_t &true_set,
//                                     vset_t &false_set) {
//   cpset_t res;
//   for (auto &each_clause_p : base) {
//     // check out whether to be surpassed
//     bool surpassed = false;
//     for (var_t i : each_clause_p->vs) {
//       if (i > 0 && true_set.find(i) != true_set.end()) {
//         surpassed = true;
//         break;
//       }
//       if (i < 0 && false_set.find(-i) != false_set.end()) {
//         surpassed = true;
//         break;
//       }
//     }
//     if (surpassed)
//       continue;
//     res.insert(each_clause_p);
//   }
//   return res;
// }

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

  // // Link the UDG here
  // for (auto &each_clause : clauses)
  //   for (var_t i : each_clause.avs)
  //     c_udg.add_node(i);
  //
  // for (auto &each_clause : clauses)
  //   for (size_t i = 1; i < each_clause.vs.size(); ++i)
  //     c_udg.add_edge(std::abs(each_clause.vs[0]),
  //     std::abs(each_clause.vs[i]));

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

// /**
//  * Randomly assign the key clauses
//  * Gurantee NOT to assign to key clauses beyond the condiered
//  * The ratio is related to the considered clauses
//  * @param ratio [description]
//  */
// cpset_t program::reduce_key_clause_ratio(double ratio, cpset_t from) {
//   cpset_t res;
//   for (auto &c : from)
//     if ((double)rand() / RAND_MAX < ratio)
//       res.insert(c);
//   return res;
// }
//
// std::vector<cpset_t>
// program::separate_clauses(cpset_t &overall, vset_t true_set, vset_t
// false_set) {
//   c_udg.clear_edges();
//   for (auto &each_clause_p : overall) {
//     for (size_t i = 1; i < each_clause_p->vs.size(); ++i) {
//       // TODO consider the true_set/false_set here? is it necessary?
//       c_udg.add_edge(std::abs(each_clause_p->vs[0]),
//                      std::abs(each_clause_p->vs[i]));
//     }
//   }
//
//   std::vector<vset_t> vars_clouds = c_udg.de_components();
//
//   std::vector<cpset_t> res;
//   // note: the complexity is NOT O(n^3)
//   for (auto &each_var_cloud : vars_clouds) {
//     cpset_t this_clause_cloud;
//     for (int each_var : each_var_cloud) {
//
//       for (auto &contain_clause : vars2clauses_map[each_var])
//         if (overall.find(contain_clause) != overall.end()) {
//           this_clause_cloud.insert(contain_clause);
//         }
//     }
//     res.push_back(this_clause_cloud);
//   }
//   return res;
// }

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

std::vector<vset_t> program::find_kernal_vars() {
  // step 1 - randomly create vset
  double ratio = 0.2;   // TODO set ratio (sample unit size) here
  int samples_cnt = 50; // TODO set sampling size here

  std::vector<vset_t> Xs;
  std::cout << "Seeking kernal vals ..." << std::endl;
  for (int i = 1; i <= samples_cnt; i++) {
    print_progress((double)i / samples_cnt); // for debugging...
    vset_t X = random_pickup(vars, (int)(ratio * vars_num));
    Xs.push_back(std::move(X));
  }
  std::cout << std::endl;

  // step 2 - get the objectives
  typedef std::pair<int, double> objs_t;
  std::vector<objs_t> objs;
  std::cout << "Fetching ex/interior objectives ..." << std::endl;
  double tmp_cnt = 1.0;
  for (auto &X : Xs) {
    print_progress((tmp_cnt++) / Xs.size()); // for debuging...
    objs.push_back(get_vs_ex_interior(X));
  }
  std::cout << std::endl;

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

/**
 * figure out the model
 * use microsoft open-source Z3 solver
 */
z3::model program::gen_valid_model(z3::optimize &opt, exprs_t &exprs) {
  for (auto &each_clause : clauses) {
    z3::expr_vector V(c);

    for (var_t v : each_clause.vs)
      V.push_back(v > 0 ? exprs.at(v) : !exprs.at(-v));
    opt.add(mk_or(V));
  }

  z3::check_result has_correct = opt.check();

  if (has_correct != z3::sat) {
    std::cout << "WARNING : No valid model" << std::endl;
    abort();
  }
  z3::model m = opt.get_model();
  return m;
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

void program::solve() {
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

  std::vector<vset_t> kernals = find_kernal_vars();
  for (const vset_t &kernal : kernals) { // for each group of kernal variables
    // step 1 - find valid model over all variables
    z3::model m = gen_valid_model(opt, exprs);
    opt.push(); // state 1 - model solved

    // step 2 - separtation!
    // s2.1 figure out the satisfied clauses
    cpset_t satisfied;
    for (var_t k : kernal) {
      if (model_of_v(m, k, decls) == Z3_L_TRUE)
        satisfied = satisfied + true_match.at(k);
      else
        satisfied = satisfied + false_match.at(k);
    }

    // s2.2 link to get c_udg; avoid satisfied clauses; avoid seted vars
    DUDG<var_t> c_udg;
    for (var_t v : vars - kernal)
      c_udg.add_node(v);

    for (auto &each_clause : clauses) {
      if (satisfied.find(&each_clause) != satisfied.end()) // existed
        continue;
      var_t lst = -1;
      for (var_t v : each_clause.avs) {
        if (kernal.find(v) != kernal.end()) // v exist in kernal
          continue;
        if (lst != -1)
          c_udg.add_edge(v, lst);
        lst = v;
      } // end v
    }   // end each_clause

    // s2.3 decomponents
    std::vector<vset_t> sub_model_vars = c_udg.de_components();
    std::vector<cpset_t> sub_models;
    for (vset_t &vs : sub_model_vars)
      sub_models.push_back(get_vars_defined_clauses(vs));

    // s2.4 solving each submodels
    timer XTimer;
    for (cpset_t &sub_model : sub_models) { // for each sub_model
      z3::optimize optp(c);                 // opt pi
      for (auto &each_clause : sub_model) {
        z3::expr_vector V(c);
        for (var_t v : each_clause->vs) {
          if (kernal.find(v) != kernal.end()) // v is included in the kernal
            continue;
          V.push_back(v > 0 ? exprs.at(v) : !exprs.at(-v));
        }
        optp.add(mk_or(V));
      } // end of constructing the submodel
      // z3::check_result has_correct =
      optp.check();
      // generate another hundreds of valid sub models
      auto checking_vars = vars - kernal;
      int r = 0;
      while (r++ < 100) {
        z3::model m = optp.get_model();
        dont_gen_m_again(optp, m, exprs, decls, checking_vars);
        auto has_valid = optp.check();
        if (has_valid != z3::sat)
          break;
        // TODO here combine the models??? do that later with z3::optimize
        // cached
      }
    } // end sub_model
    std::cout << "XXX == " << XTimer.duration() << std::endl;
  } // end for &kernal
}
