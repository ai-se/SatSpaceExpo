#pragma once

#include "commons/clause/clause.h"
#include "commons/dudg/DUDG.h"
#include "commons/udg/UDG.h"
#include <map>
#include <set>
#include <string.h>
#include <vector>

typedef std::set<clause *> clausep_set_t;

clausep_set_t operator-(const clausep_set_t &all, const clausep_set_t &except);
std::set<int> get_clauses_set_vars(clausep_set_t &css);
clausep_set_t exclude_by_priori_knowledge(clausep_set_t &base,
                                          std::set<int> &true_set,
                                          std::set<int> &false_set);

class program {
  z3::context c;
  std::vector<clause> clauses;
  std::map<int, clausep_set_t> vars2clauses_map;
  std::map<int, clausep_set_t> true_match; // if var of int(key) set to true,
                                           // then set of caluses can be matched
  std::map<int, clausep_set_t>
      false_match; // similar to the true_match... set to false
  DUDG<int> c_udg;

  void parse_cnf(std::string input_file);
  clausep_set_t reduce_key_clause_ratio(double ratio, clausep_set_t from);
  std::vector<clausep_set_t> separate_clauses(clausep_set_t &overall,
                                              std::set<int> true_set,
                                              std::set<int> false_set);
  // std::string get_model_string(z3::model &model, z3::context &c);
  bool exam_model(z3::model &model, int v);

public:
  int vars_num;
  program(std::string input_file) { parse_cnf(input_file); }
  clausep_set_t find_key_clauses();
  z3::model get_model_match_key(const clausep_set_t &keyset);
};
