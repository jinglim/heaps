#include "base/perf.h"

PerfTimer::PerfTimer() : started_(false), total_ms_(0) {}

void PerfTimer::Start() {
  assert(!started_);
  start_time_ = clock_.now();
}

void PerfTimer::Stop() {
  stop_time_ = clock_.now();
  started_ = false;

  total_ms_ += std::chrono::duration_cast<std::chrono::microseconds>(
                   stop_time_ - start_time_)
                   .count();
}

void PerfTimer::Report(const std::string &report) { report_ = report; }
