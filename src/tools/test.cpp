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
  // srand (time(NULL));
  srand(1);
  std::string model = "Benchmarks/polynomial.sk_7_25.cnf";
  // std::string model = "Benchmarks/test.cnf";
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "L") == 0)
      model = "Benchmarks/enqueueSeqSK.sk_10_42.cnf";
    if (strcmp(argv[i], "test") == 0)
      model = "Benchmarks/test.cnf";
  }
  program p_test(model);
  p_test.solve();
}

int main(int argc, char *argv[]) {
  test_key_clause_searching(argc, argv);
  // test_udg(argc, argv);
  return 0;
}
