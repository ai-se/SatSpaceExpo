#pragma once

#include <iostream>
#include <set>
#include <string>
#include <vector>

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
};

void debug(int s);
void debug(double s);
void debug(std::string s);
void debug(std::set<int> s);
void debug(std::vector<int> s);

double std_dev(std::vector<int> v, double avg);
double std_dev(std::vector<int> v);

std::set<int> get_two_objs_PF(std::vector<std::pair<int, double>>);

std::set<int> random_pickup(std::set<int> src, int cnt);

void print_progress(double percentage);
