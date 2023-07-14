#ifndef BASE_PERF_H_
#define BASE_PERF_H_

#include <chrono>
#include <string>

// Timer for performance measurement.
class PerfTimer {
public:
  PerfTimer();

  // Start the timer.
  void Start();

  // Stop the timer.
  void Stop();

  // Time taken between Start and Stop.
  long TotalDurationMs() const { return total_ms_; }

  // Returns a report tagged with the timing.
  std::string Report() const { return report_; }

  // An optional report can be tagged with the timing.
  void Report(const std::string &report);

private:
  std::chrono::steady_clock clock_;
  std::chrono::time_point<std::chrono::steady_clock> start_time_;
  std::chrono::time_point<std::chrono::steady_clock> stop_time_;

  bool started_;
  long total_ms_;
  std::string report_;
};

// A Performance test.
template <typename T> class PerfTestRunner {
public:
  virtual void Run(PerfTimer *timer, const T &params) const = 0;
};

#endif /* BASE_PERF_H_ */
