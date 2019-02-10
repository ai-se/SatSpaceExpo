#include "commons/clause/clause.h"
#include "commons/dudg/DUDG.h"
#include "commons/program/program.h"
#include "commons/utility/utility.h"
#include <iostream>
#include <set>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>

void test_udg(int argc, char *argv[]) {
  /* Testing the disjoint set*/
  UDG<int> t;
  for (int i = 1; i < 8; i++)
    t.add_node(i);

  t.add_edge(1, 3);
  t.add_edge(1, 6);
  t.add_edge(1, 7);
  t.add_edge(2, 3);
  t.add_edge(5, 6);

  std::vector<std::set<int>> cc = t.de_components();
  for (std::set<int> i : cc)
    debug(i);
}

void test_key_clause_searching(int argc, char *argv[]) {
  std::string model = "Benchmarks/polynomial.sk_7_25.cnf";
  // std::string model = "Benchmarks/test.cnf";
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "L"))
      model = "Benchmarks/enqueueSeqSK.sk_10_42.cnf";
    if (!strcmp(argv[i], "test"))
      model = "Benchmarks/test.cnf";
    if (!strcmp(argv[i], "M"))
      model = "Benchmarks/ActivityService.sk_11_27.cnf";
  }
  program p_test(model);
  p_test.create_mutate_guide_tree();
}

void test_bit_op() {
  vbitset_vec_t sample;
  sample.push_back(var_bitset(std::string("1000")));
  sample.push_back(var_bitset(std::string("1010")));
  sample.push_back(var_bitset(std::string("0011")));

  std::cout << locate_diffs(sample) << std::endl;
}

int main(int argc, char *argv[]) {
  // srand (time(NULL));
  srand(2019);
  test_key_clause_searching(argc, argv);
  // test_udg(argc, argv);
  // test_bit_op();
  return 0;
}
