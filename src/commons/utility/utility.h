#pragma once

#include <iostream>
#include <string>
#include <set>
#include <vector>

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

void print(int s);
void print(double s);
void print(std::string s);
void print(std::set<int> s);
void print(std::vector<int> s);


template <class T> void debug(T s);

double std_dev(std::vector<int> v, double avg);
double std_dev(std::vector<int> v);
