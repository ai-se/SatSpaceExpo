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
  // timer p1;
  samples = p_test.gen_N_models(10);
  // std::cout << model.substr(model.find_last_of("/") + 1) << " & ";
  // std::cout << p_test.vars_num << " & ";
  // std::cout << p_test.clauses.size() << " & ";
  // std::cout << p1.duration() << "\\\\" << std::endl;
  std::ofstream memo_file;
  memo_file.open("memo/" + model.substr(model.find_last_of("/") + 1) + ".memo");
  for (auto &sample : samples)
    memo_file << sample << std::endl;
  memo_file.close();
}

void verifying_memos(std::string model) {
  program p_test(model);
  // std::ifstream gen_input;
  std::ifstream memo_input;
  // gen_input.open("memo/" + model.substr(model.find_last_of("/") + 1) +
  //                ".qs.valid2");
  memo_input.open("memo/" + model.substr(model.find_last_of("/") + 1) +
                  ".memo");

  vbitset_vec_t gens, memos;
  std::string line;
  // int limit = INT_MAX;
  // while (getline(gen_input, line) && limit-- > 0)
  //   if (line[0] != '#')
  //     gens.push_back(var_bitset(line));
  // gen_input.close();

  while (getline(memo_input, line))
    if (line[0] != '#')
      memos.push_back(var_bitset(line));
  memo_input.close();

  // std::cout << "FROM MEMO: " << locate_diffs(memos).count() << std::endl;
  // std::cout << "FROM QS: " << locate_diffs(gens).count() << std::endl;

  // plotting the delta distribution
  std::map<var_bitset, int> delta_cnt;
  for (size_t i = 0; i < memos.size(); i++)
    for (size_t j = i + 1; j < memos.size(); j++) {
      auto d = memos[i] ^ memos[j];
      if (delta_cnt.count(d))
        delta_cnt[d] += 1;
      else
        delta_cnt.insert(std::make_pair(d, 1));
    }

  std::ofstream ofs("delta_CNT.info", std::ofstream::out | std::ofstream::app);
  for (auto &it : delta_cnt) {
    ofs << it.second << " ";
  }
  ofs << std::endl;
  ofs.close();
}

void test_solver(std::string model, double max_time) {
  program p_test(model);
  std::cout << " | " << model.substr(model.find_last_of("/") + 1) << " | "
            << p_test.vars_num << " | " << std::endl;

  vbitset_vec_t samples;

  std::ifstream loading_file;
  loading_file.open("memo/" + model.substr(model.find_last_of("/") + 1) +
                    ".memo");
  std::string line;
  while (getline(loading_file, line))
    if (line[0] != '#')
      samples.push_back(var_bitset(line));
  loading_file.close();
  // randomly pick 100 samples
  // vbitset_vec_t tmp_samples;
  // for (auto i : rnd_pick_idx(samples.size(), 100))
  //   tmp_samples.push_back(samples[i]);
  // samples = std::move(tmp_samples);
  // std::cout << "Loading samples done." << std::endl;

  timer P1;
  std::ofstream r_ofs;
  r_ofs.open("memo/" + model.substr(model.find_last_of("/") + 1) +
             ".me.valid2");
  p_test.solve(samples, r_ofs, max_time);
  P1.show_duration("sampling requires");
  r_ofs.close();
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
  double max_time = 180;

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
    if (!strcmp(argv[i], "-t"))
      max_time = double(atoi(argv[i + 1]));
  }

  // srand (time(NULL));
  srand(201903);
  // pre_memo(model);
  test_solver(model, max_time);
  // verifying_memos(model);
  // test_udg(argc, argv);
  // test_bit_op();
  // for (auto m : benchmark_models)
  //   std::cout << m << " ";
  return 0;
}