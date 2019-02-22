#pragma once

#include "commons/clause/clause.h"
#include <boost/dynamic_bitset.hpp>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <z3++.h>

typedef int var_t;                    // variable
typedef std::set<clause_t *> cpset_t; // clause_t point set
typedef std::set<var_t> vset_t;       // variable set
typedef std::map<var_t, z3::expr> exprs_t;
typedef std::map<var_t, z3::func_decl> decls_t;
typedef std::vector<z3::model> z3_model_vec_t;
typedef boost::dynamic_bitset<> var_bitset;
typedef std::vector<var_bitset> vbitset_vec_t;

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

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

  void startnow() { clock_gettime(CLOCK_REALTIME, &start_time); }

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
std::vector<size_t> rnd_pick_idx(size_t length, int cnt);
std::set<int> first_N_elements(std::set<int> src, int cnt);

void print_progress(double percentage);

var_bitset locate_diffs(vbitset_vec_t &inputs);

std::vector<size_t> sort_indexes(const std::vector<double> &v);

var_bitset truncate_bitset(var_bitset &truncating, var_bitset &mask);
size_t hamming_dist(var_bitset &v1, var_bitset &v2);