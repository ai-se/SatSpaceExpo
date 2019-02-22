#pragma GCC diagnostic ignored "-Wreorder"

#include "commons/utility/utility.h"
#include <fstream>
#include <map>
#include <sat/sat_solver.h>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <z3++.h>

class QuickSampler {
  std::string input_file;

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
  timer clock;

  std::ofstream results_file;

public:
  QuickSampler(std::string input, int max_samples, double max_time)
      : opt(c), input_file(input), max_samples(max_samples),
        max_time(max_time) {}

  void run() {
    clock.startnow();
    parse_cnf();
    results_file.open("samples/" +
                      input_file.substr(input_file.find_last_of("/") + 1) +
                      ".qs.samples");
    while (true) {
      opt.push();
      for (int v : ind) {
        if (rand() % 2)
          opt.add(literal(v), 1);
        else
          opt.add(!literal(v), 1);
      }
      if (!solve())
        break;
      z3::model m = opt.get_model();
      opt.pop();

      sample(m);
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

  void sample(z3::model m) {
    std::unordered_set<std::string> initial_mutations;
    std::string m_string = model_string(m);
    output(m_string, 0);
    opt.push();
    for (size_t i = 0; i < ind.size(); ++i) {
      int v = ind[i];
      if (m_string[i] == '1')
        opt.add(literal(v), 1);
      else
        opt.add(!literal(v), 1);
    }

    std::unordered_map<std::string, int> mutations;
    for (size_t i = 0; i < ind.size(); ++i) {
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
          std::unordered_map<std::string, int> new_mutations;
          new_mutations[new_string] = 1;
          output(new_string, 1);
          flips += 1;
          for (auto it : mutations) {
            if (it.second >= 6)
              continue;
            std::string candidate;
            for (size_t j = 0; j < ind.size(); ++j) {
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
        }
      } else {
        unsat_vars.insert(i);
      }
      opt.pop();
    }
    epochs += 1;
    opt.pop();
  }

  void output(std::string sample, int nmut) {
    samples += 1;
    results_file << sample << '\n';
  }

  void finish() {
    results_file.close();
    exit(0);
  }

  bool solve() {
    double elapsed = clock.duration();
    if (elapsed > max_time) {
      finish();
    }
    if (samples >= max_samples) {
      finish();
    }

    z3::check_result result = opt.check();
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

  z3::expr literal(int v) {
    return c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort());
  }

  bool check_sample(sat::solver &solver, std::string &line) {
    solver.user_push();
    int k = 0;
    while (!in.eof()) {
      sat::literal_vector lits;
      lits.reset();
      if (c == '0') {
        lits.push_back(sat::literal(indsup[k], true));
      } else if (c == '1') {
        lits.push_back(sat::literal(indsup[k], false));
      } else {
        printf("#%c,%d#", c, c);
        abort();
      }
      solver.mk_clause(lits.size(), lits.c_ptr());

      in >> c;
      ++k;
    }
    lbool r = solver.check();
    bool result = false;
    switch (r) {
    case l_true:
      result = true;
      break;
    case l_undef:
      std::cout << "unknown\n";
      break;
    case l_false:
      break;
    }

    solver.user_pop(1);
    return result;
  }
};

int main(int argc, char *argv[]) {
  int max_samples = 1000;
  double max_time = 7200.0;

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

  QuickSampler qs(model, max_samples, max_time);
  qs.run();
  return 0;
}