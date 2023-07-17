#include <string>
#include <unordered_map>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/flags.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"

#include "base/perf.h"
#include "heaps/binary_heap.h"
#include "heaps/binomial_heap.h"
#include "heaps/pairing_heap.h"
#include "heaps/two_three_heap.h"
#include "heaps/weak_heap.h"

ABSL_FLAG(std::string, heap, "",
          "one of {binary_heap, binomial_heap, pairing_heap, two_three_heap, "
          "weak_heap}");

namespace {
// Parameters for a Heap Performance Test.
struct PerfTestParams {
  PerfTestParams(Factory<Heap<int>> heap_factory)
      : heap_factory(heap_factory), num_elements(100), num_operations(100) {}

  Factory<Heap<int>> heap_factory;
  int num_elements;
  int num_operations;
};

std::ostream &operator<<(std::ostream &out, const PerfTestParams &params) {
  out << "PerfTestParams(num elements: " << params.num_elements
      << " num operations: " << params.num_operations << ")";
  return out;
}

// Performance Test for adding elements to a Heap.
class AddPerfTestRunner : public PerfTestRunner<PerfTestParams> {
public:
  void Run(PerfTimer *timer, const PerfTestParams &params) const override {
    std::unique_ptr<Heap<int>> heap_pointer(params.heap_factory());
    auto *heap = heap_pointer.get();

    timer->Start();
    for (int i = 0; i < params.num_elements; ++i) {
      int key = std::rand();
      heap->Add(key, i);
    }
    timer->Stop();
    timer->Report("Add");
  }
};

// Performance test for popping from a Heap.
class PopMinimumPerfTestRunner : public PerfTestRunner<PerfTestParams> {
public:
  void Run(PerfTimer *timer, const PerfTestParams &params) const override {
    std::unique_ptr<Heap<int>> heap_pointer(params.heap_factory());
    auto *heap = heap_pointer.get();

    for (int i = 0; i < params.num_elements; ++i) {
      int key = std::rand();
      heap->Add(key, i);
    }

    timer->Start();
    for (int i = 0; i < params.num_elements; ++i) {
      heap->PopMinimum();
    }
    timer->Stop();
    timer->Report("PopMinimum");
  }
};

// Performance test for adding and popping from a Heap. ie. Sorting.
class AddAndPopMinimumPerfTestRunner : public PerfTestRunner<PerfTestParams> {
public:
  void Run(PerfTimer *timer, const PerfTestParams &params) const override {
    std::unique_ptr<Heap<int>> heap_pointer(params.heap_factory());
    auto *heap = heap_pointer.get();

    timer->Start();
    for (int i = 0; i < params.num_elements; ++i) {
      int key = std::rand();
      heap->Add(key, i);
    }
    for (int i = 0; i < params.num_elements; ++i) {
      heap->PopMinimum();
    }
    timer->Stop();
    timer->Report("AddAndPopMinimum");
  }
};

// Performance test for reducing a value in a Heap.
class ReduceKeyPerfTestRunner : public PerfTestRunner<PerfTestParams> {
public:
  void Run(PerfTimer *timer, const PerfTestParams &params) const override {
    std::unique_ptr<Heap<int>> heap_pointer(params.heap_factory());
    auto *heap = heap_pointer.get();

    for (int i = 0; i < params.num_elements; ++i) {
      int key = std::rand();
      heap->Add(key, i);
    }

    timer->Start();
    for (int i = 0; i < params.num_operations; ++i) {
      int index = std::rand() % heap->size();
      int key = *heap->LookUp(index);
      int new_key = key - (std::rand() % 100);
      if (new_key <= 0) {
        new_key = 0;
      }
      heap->ReduceKey(new_key, index);
    }
    timer->Stop();
    timer->Report("ReduceKey");
  }
};

// Performance test for all operations on a Heap.
class AllOperationsPerfTestRunner : public PerfTestRunner<PerfTestParams> {
public:
  void Run(PerfTimer *timer, const PerfTestParams &params) const override {
    std::unique_ptr<Heap<int>> heap_pointer(params.heap_factory());
    auto *heap = heap_pointer.get();

    int id_counter = 0;
    int num_pops = 0;
    int num_adds = 0;
    int num_reduce_keys = 0;

    timer->Start();
    for (int i = 0; i < params.num_operations; ++i) {
      if (!heap->empty()) {
        heap->PopMinimum();
        num_pops++;
      }
      {
        int value = std::rand();
        heap->Add(value, id_counter++);
        num_adds++;
      }
      if (heap->size() < params.num_elements) {
        int key = std::rand();
        heap->Add(key, id_counter++);
        num_adds++;
      }

      {
        int id = std::rand() % id_counter;
        auto result = heap->LookUp(id);
        if (result != nullptr) {
          int new_key = *result - (std::rand() % 1000);
          if (new_key <= 0) {
            new_key = 0;
          }
          heap->ReduceKey(new_key, id);
          num_reduce_keys++;
        }
      }
    }

    while (!heap->empty()) {
      heap->PopMinimum();
      num_pops++;
    }

    timer->Stop();

    std::stringstream description;
    description << "adds: " << num_adds << ", pops: " << num_pops
                << ", reduce-keys:" << num_reduce_keys;
    timer->Report("AllOperations(" + description.str() + ")");
  }
};

// Run the performance test several times and compute the average.
void RunOnePerfTestAve(const PerfTestRunner<PerfTestParams> *runner,
                       const PerfTestParams &params, int num_runs) {
  auto run = [runner, &params]() -> std::pair<long, std::string> {
    const unsigned kRandomSeed = 12345;
    std::srand(kRandomSeed);
    PerfTimer timer;
    runner->Run(&timer, params);
    return std::make_pair(timer.TotalDurationMs(), timer.Report());
  };

  // Warm up.
  auto warmup_result = run();

  long total_time_ms = 0;
  for (int i = 0; i < num_runs; ++i) {
    total_time_ms += run().first;
  }

  long ave_time_ms = total_time_ms / num_runs;
  std::cout << "(" << num_runs << " runs) " << ave_time_ms << " ms. "
            << warmup_result.second << std::endl;
}

void RunPerfTests(Factory<Heap<int>> factory) {
  PerfTestParams params(factory);
  params.num_elements = 50000;
  params.num_operations = 200000;
  const int num_runs = 10;

  std::cout << "Params: " << params << std::endl;
  {
    AddPerfTestRunner runner;
    RunOnePerfTestAve(&runner, params, num_runs);
  }
  {
    PopMinimumPerfTestRunner runner;
    RunOnePerfTestAve(&runner, params, num_runs);
  }
  {
    AddAndPopMinimumPerfTestRunner runner;
    RunOnePerfTestAve(&runner, params, num_runs);
  }
  {
    ReduceKeyPerfTestRunner runner;
    RunOnePerfTestAve(&runner, params, num_runs);
  }
  {
    AllOperationsPerfTestRunner runner;
    RunOnePerfTestAve(&runner, params, num_runs);
  }
}

} // namespace

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  absl::InitializeLog();

  std::unordered_map<std::string, Factory<Heap<int>>> heap_factories{
      {"binary_heap", BinaryHeap<int>::factory()},
      {"binomial_heap", BinomialHeap<int>::factory()},
      {"weak_heap", WeakHeap<int>::factory()},
      {"pairing_heap", PairingHeap<int>::factory()},
      {"two_three_heap", TwoThreeHeap<int>::factory()}};

  std::string heap_flag = absl::GetFlag(FLAGS_heap);
  auto it = heap_factories.find(heap_flag);
  if (it == heap_factories.end()) {
    LOG(FATAL) << "Unknown heap: " << heap_flag;
  }

  auto factory = it->second;
  std::cout << std::endl << "Perf Testing " << factory.name() << std::endl;
  RunPerfTests(factory);

  return 0;
}
