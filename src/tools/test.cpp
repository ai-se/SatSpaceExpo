#include "commons/clause/clause.h"
#include "commons/dudg/DUDG.h"
#include "commons/program/program.h"
#include "commons/utility/utility.h"
#include <fstream>
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

/* generating and memorizing the samples for testing */
void pre_memo(std::string model) {
  program p_test(model);
  vbitset_vec_t samples;
  samples = p_test.gen_N_models(100);
  std::ofstream memo_file;
  memo_file.open("memo/" + model.substr(model.find_last_of("/") + 1) + ".memo");
  for (auto &sample : samples)
    memo_file << sample << std::endl;
  memo_file.close();
}

void test_solver(std::string model) {
  program p_test(model);
  std::cout << " | " << model.substr(model.find_last_of("/") + 1) << " | "
            << p_test.vars_num << " | " << std::endl;

  vbitset_vec_t samples;

  std::ifstream loading_file;
  loading_file.open("memo/" + model.substr(model.find_last_of("/") + 1) +
                    ".memo");
  std::string line;
  while (getline(loading_file, line))
    samples.push_back(var_bitset(line));
  loading_file.close();
  std::cout << "Loading samples done." << std::endl;

  timer P1;
  std::ofstream r_ofs;
  r_ofs.open("memo/" + model.substr(model.find_last_of("/") + 1) +
             ".me.valid2");
  p_test.solve(samples, r_ofs);
  P1.show_duration("sampling requires");
}

void test_bit_op() {
  // vbitset_vec_t sample;
  // sample.push_back(var_bitset(std::string("1000")));
  // sample.push_back(var_bitset(std::string("1010")));
  // sample.push_back(var_bitset(std::string("0011")));
  // std::cout << locate_diffs(sample) << std::endl;
  var_bitset A(std::string("11010001000"));
  var_bitset B(std::string("11001101001"));
  // auto e = truncate_bitset(truncating, mmmmmmmask);
  std::cout << (A ^ B) << std::endl;
}

int main(int argc, char *argv[]) {
  std::string model = "Benchmarks/polynomial.sk_7_25.cnf";
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "L"))
      model = "Benchmarks/enqueueSeqSK.sk_10_42.cnf";
    if (!strcmp(argv[i], "test"))
      model = "Benchmarks/test.cnf";
    if (!strcmp(argv[i], "M"))
      model = "Benchmarks/ActivityService.sk_11_27.cnf";
    if (!strcmp(argv[i], "-m"))
      model = argv[i + 1];
    if (!strcmp(argv[i], "-i"))
      model = benchmark_models[atoi(argv[i + 1])];
  }

  // srand (time(NULL));
  srand(201902);
  // pre_memo(model);
  test_solver(model);
  // test_udg(argc, argv);
  // test_bit_op();
  // for (auto m : benchmark_models)
  //   std::cout << m << " ";
  return 0;
}