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

static std::vector<std::string> benchmark_models{
    "Benchmarks/Blasted_Real/blasted_case47.cnf", // 0
    "Benchmarks/Blasted_Real/blasted_case110.cnf",
    "Benchmarks/V7/s820a_7_4.cnf",
    "Benchmarks/V15/s820a_15_7.cnf",
    "Benchmarks/V3/s1238a_3_2.cnf",
    "Benchmarks/V3/s1196a_3_2.cnf", // 5
    "Benchmarks/V15/s832a_15_7.cnf",
    "Benchmarks/Blasted_Real/blasted_case_1_b12_2.cnf",
    "Benchmarks/Blasted_Real/blasted_squaring16.cnf",
    "Benchmarks/Blasted_Real/blasted_squaring7.cnf",
    "Benchmarks/70.sk_3_40.cnf", // 10
    "Benchmarks/ProcessBean.sk_8_64.cnf",
    "Benchmarks/56.sk_6_38.cnf",
    "Benchmarks/35.sk_3_52.cnf",
    "Benchmarks/80.sk_2_48.cnf",
    "Benchmarks/7.sk_4_50.cnf", // 15
    "Benchmarks/doublyLinkedList.sk_8_37.cnf",
    "Benchmarks/19.sk_3_48.cnf",
    "Benchmarks/29.sk_3_45.cnf",
    "Benchmarks/isolateRightmost.sk_7_481.cnf",
    "Benchmarks/17.sk_3_45.cnf", // 20
    "Benchmarks/81.sk_5_51.cnf",
    "Benchmarks/LoginService2.sk_23_36.cnf",
    "Benchmarks/sort.sk_8_52.cnf",
    "Benchmarks/parity.sk_11_11.cnf",
    "Benchmarks/77.sk_3_44.cnf", // 25
    "Benchmarks/20.sk_1_51.cnf",
    "Benchmarks/enqueueSeqSK.sk_10_42.cnf",
    "Benchmarks/karatsuba.sk_7_41.cnf",
    "Benchmarks/diagStencilClean.sk_41_36.cnf",
    "Benchmarks/tutorial3.sk_4_31.cnf"};

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

  vbitset_vec_t samples;
  /* load samples for file */
  std::ifstream loading_file;
  loading_file.open("memo/" + model.substr(model.find_last_of("/") + 1) +
                    ".memo");
  std::string line;
  while (getline(loading_file, line))
    samples.push_back(var_bitset(line));
  loading_file.close();

  btree T = p_test.create_mutate_guide_tree(samples);
  p_test.mutate_the_seed_with_tree(T, samples[4], samples);
  // p_test.exp_start_from_samples(samples);
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
  return 0;
}