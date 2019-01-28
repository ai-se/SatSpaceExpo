#pragma once

#include "commons/clause/clause.h"
#include "commons/dudg/DUDG.h"
#include "commons/udg/UDG.h"
#include <map>
#include <set>
#include <string.h>
#include <vector>

enum key_sig { NOT_KEY, IS_KEY, BEYOND_CONSIDERED };
typedef std::map<clause *, key_sig> kmap_t;

class program {
  std::vector<clause> clauses;
  std::map<int, std::set<clause *>> vars2clauses_map;
  kmap_t key_map;
  DUDG<int> c_udg;
  std::set<int> vars;

  void parse_cnf(std::string input_file);
  void erase_clause(int index);
  kmap_t reduce_key_clause_ratio(double ratio, kmap_t &key_map);
  double get_kmap_objs(kmap_t &key_map);
  std::string get_model_string(z3::model &model, z3::context &c);

public:
  int vars_num;
  program(std::string input_file) { parse_cnf(input_file); }
  void get_trivial_model(int trivial[]);
  kmap_t find_key_clauses();
  void get_model_match_key(kmap_t &key_map);
};
