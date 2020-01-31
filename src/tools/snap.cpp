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

/* generating and memorizing the samples for testing */
void pre_memo(std::string model) {
  program p_test(model);
  vbitset_vec_t samples;
  samples = p_test.gen_N_models(10);
  std::ofstream memo_file;
  memo_file.open("memo/" + model.substr(model.find_last_of("/") + 1) + ".memo");
  for (auto &sample : samples)
    memo_file << sample << std::endl;
  memo_file.close();
}

void verifying_memos(std::string model) {
  program p_test(model);
  std::ifstream memo_input;
  // gen_input.open("memo/" + model.substr(model.find_last_of("/") + 1) +
  //                ".qs.valid2");
  memo_input.open("memo/" + model.substr(model.find_last_of("/") + 1) +
                  ".memo");

  vbitset_vec_t gens, memos;
  std::string line;

  while (getline(memo_input, line))
    if (line[0] != '#')
      memos.push_back(var_bitset(line));
  memo_input.close();

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
  vbitset_vec_t samples = p_test.gen_N_models(100);

  timer P1;
  std::ofstream r_ofs;
  r_ofs.open("memo/" + model.substr(model.find_last_of("/") + 1) +
             ".me.valid2");
  p_test.solve(samples, r_ofs, max_time);
  P1.show_duration("sampling requires");
  r_ofs.close();
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
  test_solver(model, max_time);
  return 0;
}