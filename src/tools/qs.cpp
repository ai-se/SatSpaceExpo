#include "commons/utility/utility.h"
#include <fstream>
#include <map>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <z3++.h>

class QuickSampler {
  std::string input_file;

  struct timespec start_time;
  double solver_time = 0.0;
  int max_samples;
  double max_time;

  z3::context c;
  z3::optimize opt;
  std::vector<int> ind;
  std::unordered_set<int> unsat_vars;
  int epochs = 0;
  int flips = 0;
  int samples = 0;
  int solver_calls = 0;

  std::ofstream results_file;

public:
  QuickSampler(std::string input, int max_samples, double max_time)
      : opt(c), input_file(input), max_samples(max_samples),
        max_time(max_time) {}

  void run() {
    clock_gettime(CLOCK_REALTIME, &start_time);
    srand(start_time.tv_sec);
    parse_cnf();
    results_file.open(input_file + ".samples");
    int n = 6;
    while (n-- > 0) {
      std::cout << n << std::endl;
      opt.push();
      for (int v : ind) {
        if (rand() % 2)
          opt.add(literal(v), 1);
        else
          opt.add(!literal(v), 1);
      }
      if (!solve()) {
        std::cout << "Could not find a solution!\n";
        exit(0);
      }
      z3::model m = opt.get_model();
      opt.pop();

      //   sample(m);
    }
  }

  void parse_cnf() {
    z3::expr_vector exp(c);
    std::ifstream f(input_file);
    if (!f.is_open()) {
      std::cout << "Error opening input file\n";
      abort();
    }
    std::unordered_set<int> indset;
    bool has_ind = false;
    int max_var = 0;
    std::string line;
    while (getline(f, line)) {
      std::istringstream iss(line);
      if (line.find("c ind ") == 0) {
        std::string s;
        iss >> s;
        iss >> s;
        int v;
        while (!iss.eof()) {
          iss >> v;
          if (v && indset.find(v) == indset.end()) {
            indset.insert(v);
            ind.push_back(v);
            has_ind = true;
          }
        }
      } else if (line[0] != 'c' && line[0] != 'p') {
        z3::expr_vector clause(c);
        int v;
        while (!iss.eof()) {
          iss >> v;
          if (v > 0)
            clause.push_back(literal(v));
          else if (v < 0)
            clause.push_back(!literal(-v));
          v = abs(v);
          if (!has_ind && v != 0)
            indset.insert(v);
          if (v > max_var)
            max_var = v;
        }
        if (clause.size() > 0)
          exp.push_back(mk_or(clause));
      }
    }
    f.close();
    if (!has_ind) {
      for (int lit = 0; lit <= max_var; ++lit) {
        if (indset.find(lit) != indset.end()) {
          ind.push_back(lit);
        }
      }
    }
    z3::expr formula = mk_and(exp);
    opt.add(formula);
  }

  bool solve() {
    z3::check_result result = opt.check();
    return result == z3::sat;
  }

  std::string model_string(z3::model model) {
    std::string s;

    for (int v : ind) {
      z3::func_decl decl(literal(v).decl());
      z3::expr b = model.get_const_interp(decl);
      if (b.bool_value() == Z3_L_TRUE) {
        s += "1";
      } else {
        s += "0";
      }
    }
    return s;
  }

  double duration(struct timespec *a, struct timespec *b) {
    return (b->tv_sec - a->tv_sec) + 1.0e-9 * (b->tv_nsec - a->tv_nsec);
  }

  z3::expr literal(int v) {
    return c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort());
  }
};

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

  int max_samples = 10000;
  QuickSampler s(model, max_samples, max_time);
  s.run();
  return 0;
}