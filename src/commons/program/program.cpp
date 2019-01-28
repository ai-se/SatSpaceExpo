#include "commons/utility/utility.h"
#include "program.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <z3++.h>

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
  for (clause &each_clause : clauses) {
    for (int v : each_clause.avs)
      vars2clauses_map[v].insert(&each_clause);
  }


  // Link the UDG here
  for (auto &each_clause : clauses)
    for (int i : each_clause.avs)
      vars.insert(i);
  for (auto &v : vars)
    c_udg.add_node(v);

  for (auto &each_clause : clauses)
    for (size_t i = 1; i < each_clause.vs.size(); ++i)
      c_udg.add_edge(std::abs(each_clause.vs[0]), std::abs(each_clause.vs[i]));

  std::cout << "INFO : |E| = " << c_udg.get_edge_num() << std::endl;
  std::cout << "INIT : Loading " << input_file << " done." << std::endl;
}

void program::get_trivial_model(int trivial[]) {
  std::memset(trivial, 0, (vars_num + 1) * sizeof(int));
  for (auto &each_clause : clauses) {
    int v;
    if ((v = each_clause.get_determined())) {
      trivial[std::abs(v)] = v > 0 ? 1 : -1;
    }
  }
}

/**
 * Randomly assign the key clauses
 * Gurantee NOT to assign the unkey clause to key clauses
 * The ratio is related to the overall clauses
 * @param ratio [description]
 */
kmap_t program::reduce_key_clause_ratio(double ratio, kmap_t &key_map) {
  kmap_t res;
  int key_counts = 0;
  for (clause &each_clause : clauses)
    if (key_map[&each_clause])
      key_counts++;
  double prev_ratio = ((double)key_counts) / clauses.size();
  double reduction_ratio = ratio / prev_ratio;

  for (clause &each_clause : clauses)
    if (key_map[&each_clause] && rand() / double(RAND_MAX) > reduction_ratio)
      res[&each_clause] = NOT_KEY; // removing the key tag
    else
      res[&each_clause] = key_map[&each_clause]; // remain the same

  return res;
}

/**
 * remove the clause of Index index.
 * swap that with last one, and them remove the last one
 * @param index [description]
 */
void program::erase_clause(int index) {
  std::swap(clauses[index], clauses[clauses.size() - 1]);
  clauses.pop_back();
}

/**
 * Given a key_map, calculate the evaluation of the map
 * The evaluation is LOWER THE BETTER
 * Objective = stdard_dev(size of each component)
 * @param  key_map
 * @return         [description]
 */
double program::get_kmap_objs(kmap_t &key_map) {
  // reset the c_udg edges
  c_udg.clear_edges();
  for (auto &each_clause : clauses) {
    if (key_map[&each_clause])
      continue;
    for (size_t i = 1; i < each_clause.vs.size(); ++i)
      c_udg.add_edge(std::abs(each_clause.vs[0]), std::abs(each_clause.vs[i]));
  }
  // print(c_udg.get_edge_num());

  std::vector<std::set<int>> vars_clouds = c_udg.de_components();
  // int evaluation = 1;
  std::vector<int> cloud_sizes;
  // note: the complexity is NOT O(n^3)
  for (auto &each_var_cloud : vars_clouds) {
    std::set<clause *> this_clause_cloud;
    for (int each_var : each_var_cloud) {
      for (auto &contain_clause : vars2clauses_map[each_var])
        if (!key_map[contain_clause])
          this_clause_cloud.insert(contain_clause);
    }
    cloud_sizes.push_back(this_clause_cloud.size());
  }
  print(cloud_sizes);
  return std_dev(cloud_sizes);
}

kmap_t program::find_key_clauses() {
  timer timer_of_this;
  std::cout << "INFO : Searching for key clauses." << std::endl;
  // starting for trival plan -- all as key
  kmap_t full_map;
  for (clause &each_clause : clauses)
    full_map[&each_clause] = IS_KEY;

  // reduction path 90-70-50-30-10%
  // sampling plan size = 10 (or based on the running time / program
  // complexity?)
  kmap_t last_map = full_map, best;
  for (double ratio : {0.9, 0.7, 0.5, 0.3, 0.1}) {
  // for (double ratio : {0.7}) {
    std::cout << "Ratio " << ratio << "====" << std::endl;
    int sample_size = 30;
    double map_obj, best_obj = std::numeric_limits<double>::max();
    kmap_t best;
    for (int s = 0; s < sample_size; ++s) {
      kmap_t map_plan = reduce_key_clause_ratio(ratio, last_map);
      map_obj = get_kmap_objs(map_plan);
      // print(map_obj);
      if (map_obj < best_obj) { // update the current best
        best_obj = map_obj;
        best = map_plan;
      } // end update
    }   // end sample
    last_map = best;
    std::cout << best_obj << std::endl;
  }
  std::cout << "INFO : Done in " << timer_of_this.duration() << " secs."
            << std::endl;
  return best;
}

/**
 * figure out the model which matchs the key clauses only
 * @param key_map [description]
 */
void program::get_model_match_key(kmap_t &key_map) {
  // Using microsoft open-source Z3 solver
  z3::context c;
  z3::optimize opt(c);
  z3::expr_vector exp(c);

  for (auto &each_clause : clauses) {
    // if (key_map[&each_clause])
    exp.push_back(each_clause.toExpr(c));
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
  // printing the models
  // std::cout << get_model_string(m_match_key_clauses, c) << std::endl;
}

std::string program::get_model_string(z3::model &model, z3::context &c) {
  std::string s;
  // z3::context c;
  for (int v : vars) {
    z3::func_decl decl(
        c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort())
            .decl());
    z3::expr b = model.get_const_interp(decl);
    if (b.bool_value() == Z3_L_TRUE)
      s += "1";
    else
      s += "0";
  }

  return s;
}
