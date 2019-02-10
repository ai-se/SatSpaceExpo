#pragma once

#include "commons/clause/clause.h"
#include <boost/dynamic_bitset.hpp>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <z3++.h>

typedef int var_t;                  // variable
typedef std::set<clause *> cpset_t; // clause point set
typedef std::set<var_t> vset_t;     // variable set
typedef std::map<var_t, z3::expr> exprs_t;
typedef std::map<var_t, z3::func_decl> decls_t;
typedef std::vector<z3::model> z3_model_vec_t;
typedef boost::dynamic_bitset<> var_bitset;
typedef std::vector<var_bitset> vbitset_vec_t;

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

/**
 * Get the running time (in seconds) of a block of code!
 * Demostration
 * timer t1;
 * // RUN something
 * std::cout << t1.duration() << std::endl; // e.g. 3.223s
 */
struct timer {
  struct timespec start_time, end_time;
  timer() { clock_gettime(CLOCK_REALTIME, &start_time); }

  double duration() {
    clock_gettime(CLOCK_REALTIME, &end_time);
    struct timespec *a = &start_time;
    struct timespec *b = &end_time;
    return (b->tv_sec - a->tv_sec) + 1.0e-9 * (b->tv_nsec - a->tv_nsec);
  }

  void show_duration(std::string name) {
    std::cout << name << "<<< " << duration() << " sec(s)." << std::endl;
  }
};

void debug(int s);
void debug(double s);
void debug(std::string s);
void debug(std::set<int> s);
void debug(std::vector<int> s);

double std_dev(std::vector<int> v, double avg);
double std_dev(std::vector<int> v);

std::set<int> random_pickup(std::set<int> src, int cnt);
std::set<int> first_N_elements(std::set<int> src, int cnt);

void print_progress(double percentage);

var_bitset locate_diffs(vbitset_vec_t &inputs);

std::vector<size_t> sort_indexes(const std::vector<double> &v);
