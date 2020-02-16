#include "commons/utility/utility.h"
#include <fstream>
#include <map>
#include <set>
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
  z3::context c2;
  z3::optimize opt;
  z3::optimize opt4Verify;
  std::vector<int> ind;
  std::set<int> vars; // sorted
  std::unordered_set<int> unsat_vars;
  int epochs = 0;
  int flips = 0;
  int samples = 0;
  int solver_calls = 0;
  std::set<std::string> res_strs;

  std::ofstream results_file;

public:
  QuickSampler(std::string input, int max_samples, double max_time)
      : opt(c), opt4Verify(c2), input_file(input), max_samples(max_samples),
        max_time(max_time) {}

  void run() {
    clock_gettime(CLOCK_REALTIME, &start_time);
    srand(start_time.tv_sec);
    parse_cnf();
    results_file.open(
        "memo/" + input_file.substr(input_file.find_last_of("/") + 1) + ".qs");
    while (true) {
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

      sample(m);
      print_stats(false);
    }
  }

  void print_stats(bool simple) {
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    double elapsed = duration(&start_time, &end);
    std::cout << "Samples " << samples << '\n';
    std::cout << "Execution time " << elapsed << '\n';
    if (simple)
      return;
    std::cout << "Solver time: " << solver_time << '\n';
    std::cout << "Epochs " << epochs << ", Flips " << flips << ", Unsat "
              << unsat_vars.size() << ", Calls " << solver_calls << '\n';
  }

  void parse_cnf() {
    z3::expr_vector exp(c);
    z3::expr_vector exp2(c2);
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
        z3::expr_vector clause2(c2);
        int v;
        while (!iss.eof()) {
          iss >> v;
          if (v > 0) {
            clause.push_back(literal(v));
            clause2.push_back(literal2(v));
          } else if (v < 0) {
            clause.push_back(!literal(-v));
            clause2.push_back(!literal2(-v));
          }
          v = abs(v);
          if (v != 0) {
            vars.insert(v);
          }
          if (!has_ind && v != 0)
            indset.insert(v);
          if (v > max_var)
            max_var = v;
        }
        if (clause.size() > 0) {
          exp.push_back(mk_or(clause));
          exp2.push_back(mk_or(clause2));
        }
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
    z3::expr formula2 = mk_and(exp2);
    opt.add(formula);
    opt4Verify.add(formula2);
  }

  void sample(z3::model m) {
    std::unordered_set<std::string> initial_mutations;
    std::string m_string = model_string(m);
    std::cout << m_string << " STARTING\n";
    output(m_string, 0);
    opt.push();
    for (int i = 0; i < ind.size(); ++i) {
      int v = ind[i];
      if (m_string[i] == '1')
        opt.add(literal(v), 1);
      else
        opt.add(!literal(v), 1);
    }

    std::unordered_map<std::string, int> mutations;
    for (int i = 0; i < ind.size(); ++i) {
      if (unsat_vars.find(i) != unsat_vars.end())
        continue;
      opt.push();
      int v = ind[i];
      if (m_string[i] == '1')
        opt.add(!literal(v));
      else
        opt.add(literal(v));
      if (solve()) {
        z3::model new_model = opt.get_model();
        std::string new_string = model_string(new_model);
        if (initial_mutations.find(new_string) == initial_mutations.end()) {
          initial_mutations.insert(new_string);
          // std::cout << new_string << '\n';
          std::unordered_map<std::string, int> new_mutations;
          new_mutations[new_string] = 1;
          output(new_string, 1);
          flips += 1;
          for (auto it : mutations) {
            if (it.second >= 6)
              continue;
            std::string candidate;
            for (int j = 0; j < ind.size(); ++j) {
              bool a = m_string[j] == '1';
              bool b = it.first[j] == '1';
              bool c = new_string[j] == '1';
              if (a ^ ((a ^ b) | (a ^ c)))
                candidate += '1';
              else
                candidate += '0';
            }
            if (mutations.find(candidate) == mutations.end() &&
                new_mutations.find(candidate) == new_mutations.end()) {
              new_mutations[candidate] = it.second + 1;
              output(candidate, it.second + 1);
            }
          }
          for (auto it : new_mutations) {
            mutations[it.first] = it.second;
          }
        } else {
          // std::cout << new_string << " repeated\n";
        }
      } else {
        std::cout << "unsat\n";
        unsat_vars.insert(i);
      }
      opt.pop();
      print_stats(true);
    }
    epochs += 1;
    opt.pop();
  }

  std::string valid_solution(std::string sample) {
    opt4Verify.push();
    exprs_t exprs2;
    decls_t decls2;
    for (var_t v : vars) {
      z3::expr l = literal2(v);
      exprs2.insert(std::make_pair(v, l));
      decls2.insert(std::make_pair(v, l.decl()));
    }

    for (int i = 0; i < ind.size(); ++i) {
      int v = ind[i];
      if (sample[i] == '1')
        opt4Verify.add(exprs2.at(v), 1);
      else
        opt4Verify.add(!exprs2.at(v), 1);
    }

    bool issat = opt4Verify.check() == z3::sat;
    z3::model mt = opt4Verify.get_model();
    std::string s;

    if (issat && res_strs.find(sample) == res_strs.end()) {
      res_strs.insert(sample);
      for (int v : vars) {
        z3::expr b = mt.get_const_interp(decls2.at(v));
        if (b.bool_value() == Z3_L_TRUE) {
          s += "1";
        } else {
          s += "0";
        }
      }
    } else {
      s = "-1";
    }
    opt4Verify.pop();
    return s;
  }

  void output(std::string sample, int nmut) {
    std::string complete_str = valid_solution(sample);
    if (complete_str.compare("-1") != 0) {
      results_file << complete_str << '\n';
    }
  }

  void finish() {
    print_stats(false);
    results_file.close();
    exit(0);
  }

  bool solve() {
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    double elapsed = duration(&start_time, &start);
    if (elapsed > max_time) {
      std::cout << "Stopping: timeout\n";
      finish();
    }
    if (samples >= max_samples) {
      std::cout << "Stopping: samples\n";
      finish();
    }

    z3::check_result result = opt.check();
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    solver_time += duration(&start, &end);
    solver_calls += 1;

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

  z3::expr literal2(int v) {
    return c2.constant(c2.str_symbol(std::to_string(v).c_str()),
                       c2.bool_sort());
  }
};

int main(int argc, char *argv[]) {
  std::string model = "Benchmarks/polynomial.sk_7_25.cnf";
  double max_time = 60.0;

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

  int max_samples = 100000;
  QuickSampler s(model, max_samples, max_time);
  s.run();
  return 0;
}
