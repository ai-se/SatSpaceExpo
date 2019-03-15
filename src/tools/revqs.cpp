#include "commons/program/program.h"
#include "commons/utility/utility.h"
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

/**
 * This tool is for recovering the full string of the valid results found by
 *quicksampler
 **/
int main(int argc, char *argv[]) {
  std::string model = "Benchmarks/polynomial.sk_7_25.cnf";
  double max_time = 200; // max time for simulating the whole processdure

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

  program pg(model);
  z3::context c;

  std::string file =
      "memo/" + model.substr(model.find_last_of("/") + 1) + ".qs.valid";
  std::ifstream ifs(file);
  std::ofstream ofs(file + "2");

  // create exprs and decls
  exprs_t exprs;
  decls_t decls;
  for (var_t v : pg.vars) {
    z3::expr l =
        c.constant(c.str_symbol(std::to_string(v).c_str()), c.bool_sort());
    exprs.insert(std::make_pair(v, l));
    decls.insert(std::make_pair(v, l.decl()));
  }
  // END create exprs and decls

  // load the whole model
  z3::optimize opt(c);
  for (auto &clause : pg.clauses) {
    z3::expr_vector V(c);
    for (var_t v : clause.vs)
      V.push_back(v > 0 ? exprs.at(v) : !exprs.at(-v));
    opt.add(mk_or(V));
  }
  // END load the whole model

  // start handling
  std::string line;
  timer P1;
  int valid_count = 0;
  while (std::getline(ifs, line)) {
    if (line[0] == '#') { // updating the time, stop when necessary
      std::string trans_str;
      std::istringstream iss(line);
      iss >> trans_str;
      ofs << "# ";
      iss >> trans_str; // time
      double dur = P1.duration() + atof(trans_str.c_str());
      ofs << dur << " ";
      iss >> trans_str; // valid_#
      ofs << valid_count << " / ";
      iss >> trans_str;
      iss >> trans_str; // overall_sample#
      ofs << trans_str << std::endl;

      if (dur > max_time)
        break;

      continue;
    }

    opt.push();
    auto curr = pg.indv.begin();
    for (size_t i = 0; i < line.length(); i++) {
      opt.add(line[i] == '1' ? exprs.at(*curr) : !exprs.at(*curr));
      curr++;
    }
    if (opt.check() == z3::sat) {
      valid_count++;
      z3::model m = opt.get_model();
      ofs << pg.read_model(m, decls) << std::endl;
    }
    opt.pop();
  }

  ifs.close();
  ofs.close();
}
