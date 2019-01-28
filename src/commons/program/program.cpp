#include "commons/utility/utility.h"
#include "program.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <z3++.h>

clausep_set_t operator-(const clausep_set_t &all, const clausep_set_t &except) {
  clausep_set_t remain;
  for (auto &i : all)
    if (except.find(i) == except.end())
      remain.insert(i);
  return remain;
}

std::set<int> get_clauses_set_vars(clausep_set_t &css) {
  std::set<int> res;
  for (auto &each_clause_p : css)
    for (int i : each_clause_p->avs)
      res.insert(i);
  return res;
}

clausep_set_t exclude_by_priori_knowledge(clausep_set_t &base,
                                          std::set<int> &true_set,
                                          std::set<int> &false_set) {
  clausep_set_t res;
  for (auto &each_clause_p : base) {
    // check out whether to be surpassed
    bool surpassed = false;
    for (int i : each_clause_p->vs) {
      if (i > 0 && true_set.find(i) != true_set.end()) {
        surpassed = true;
        break;
      }
      if (i < 0 && false_set.find(-i) != false_set.end()) {
        surpassed = true;
        break;
      }
    }
    if (surpassed)
      continue;
    res.insert(each_clause_p);
  }
  return res;
}

/**
 * Parsing the cnf file as the program.
 * Does not return anything set up in the private variables
 * @param input_file name of cnf file
 */
void program::parse_cnf(std::string input_file) {
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
    for (int v : each_clause.vs) {
      vars2clauses_map[std::abs(v)].insert(&each_clause);
      if (v > 0)
        true_match[v].insert(&each_clause);
      else
        false_match[-v].insert(&each_clause);
    }

  // Link the UDG here
  for (auto &each_clause : clauses)
    for (int i : each_clause.avs)
      c_udg.add_node(i);

  for (auto &each_clause : clauses)
    for (size_t i = 1; i < each_clause.vs.size(); ++i)
      c_udg.add_edge(std::abs(each_clause.vs[0]), std::abs(each_clause.vs[i]));

  std::cout << "INIT : Loading " << input_file << " done." << std::endl;
}

/**
 * Randomly assign the key clauses
 * Gurantee NOT to assign to key clauses beyond the condiered
 * The ratio is related to the considered clauses
 * @param ratio [description]
 */
clausep_set_t program::reduce_key_clause_ratio(double ratio,
                                               clausep_set_t from) {
  clausep_set_t res;
  for (auto &c : from)
    if ((double)rand() / RAND_MAX < ratio)
      res.insert(c);
  return res;
}

std::vector<clausep_set_t> program::separate_clauses(clausep_set_t &overall,
                                                     std::set<int> true_set,
                                                     std::set<int> false_set) {
  c_udg.clear_edges();
  for (auto &each_clause_p : overall) {
    for (size_t i = 1; i < each_clause_p->vs.size(); ++i) {
      // TODO consider the true_set/false_set here? is it necessary?
      c_udg.add_edge(std::abs(each_clause_p->vs[0]),
                     std::abs(each_clause_p->vs[i]));
    }
  }

  std::vector<std::set<int>> vars_clouds = c_udg.de_components();

  std::vector<clausep_set_t> res;
  // note: the complexity is NOT O(n^3)
  for (auto &each_var_cloud : vars_clouds) {
    clausep_set_t this_clause_cloud;
    for (int each_var : each_var_cloud) {

      for (auto &contain_clause : vars2clauses_map[each_var])
        if (overall.find(contain_clause) != overall.end()) {
          this_clause_cloud.insert(contain_clause);
        }
    }
    res.push_back(this_clause_cloud);
  }
  return res;
}

clausep_set_t program::find_key_clauses() {
  timer timer_of_this;
  std::cout << "INFO : Searching for key clauses." << std::endl;

  // step 1: sample base -- ALL clauses
  clausep_set_t all_clauses;
  for (clause &each_clause : clauses) {
    all_clauses.insert(&each_clause);
  }

  // step 2: random assign key clauses with ratio key
  clausep_set_t key_plan;
  for (size_t repeat = 0; repeat < 20; repeat++) {
    double ratio = 0.3;
    key_plan = reduce_key_clause_ratio(ratio, all_clauses);
    auto determined_vars = get_clauses_set_vars(key_plan);
    auto key_model = get_model_match_key(key_plan);

    // step 3: get prior knowledge
    std::set<int> true_set, false_set;
    for (int i : determined_vars) {
      if (exam_model(key_model, i))
        true_set.insert(i);
      else
        false_set.insert(i);
    }

    // step 4: exclue the known knowledge, and separte clauses
    auto remain = all_clauses - key_plan;
    auto remain2 = exclude_by_priori_knowledge(remain, true_set, false_set);
    std::cout << get_clauses_set_vars(remain2).size() << std::endl;
    // step 5: separation
    auto clouds = separate_clauses(remain2, true_set, false_set);
    clouds.push_back(key_plan);
    std::vector<int> cloudsize;
    for (auto &each_cloud_set : clouds)
      cloudsize.push_back(each_cloud_set.size());

    print(cloudsize);
    double map_obj = std_dev(cloudsize);
  }

  std::cout << "INFO : Done in " << timer_of_this.duration() << " secs."
            << std::endl;
  return key_plan;
}

/**
 * figure out the model which matchs the key clauses only
 * use microsoft open-source Z3 solver
 */
z3::model program::get_model_match_key(const clausep_set_t &keyset) {
  z3::optimize opt(c);
  z3::expr_vector exp(c);

  for (auto &each_clause : keyset) {
    exp.push_back(each_clause->toExpr(c));
  }
  z3::expr formula = mk_and(exp);
  opt.add(formula);

  opt.push();
  z3::check_result has_correct = opt.check();
  // TODO what if (has_correct != z3::sat) ??
  if (has_correct != z3::sat)
    std::cout << "WARNING : No valid model" << std::endl;
  z3::model m_match_key_clauses = opt.get_model();
  opt.pop();

  return m_match_key_clauses;
}

bool program::exam_model(z3::model &model, int v) {
  z3::func_decl decl(
      c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort())
          .decl());
  z3::expr b = model.get_const_interp(decl);
  return b.bool_value() == Z3_L_TRUE;
}

// std::string program::get_model_string(z3::model &model, z3::context &c) {
//   std::string s;
//   // TODO ATTENTION may contain zeros which are dumped.
//   for (int v = 1; v <= vars_num; v++) {
//     bool tmp = exam_model(v, model, c);
//     s += tmp ? "1" : "0";
//   }
//
//   return s;
// }
