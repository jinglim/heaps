// Tests Heap implementations by running through all Heap operations.

#include <chrono>

#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/flags.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"

#include "heaps/binary_heap.h"
#include "heaps/binomial_heap.h"
#include "heaps/fibonacci_heap.h"
#include "heaps/pairing_heap.h"
#include "heaps/two_three_heap.h"
#include "heaps/weak_heap.h"

namespace {

const unsigned kRandomSeed = 12346789;

// If true, print out the heap operations.
const bool kDebugPrintOperations = false;

// If true, print out the heap in tree format after each operation.
const bool kDebugPrintTree = false;

// Keeps track of a set of int ids.
class IdSet {
public:
  size_t size() const { return ids_.size(); }

  // Add an id to the set.
  void Add(int id) { ids_.push_back(id); }

  // Remove an id.
  void Remove(int id) {
    for (int i = 0; i < ids_.size(); i++) {
      if (ids_[i] == id) {
        int last_id = ids_.back();
        ids_.pop_back();
        if (ids_.size() > 0) {
          ids_[i] = last_id;
        }
        return;
      }
    }
    LOG(FATAL) << "Id not found";
  }

  // Returns a random id.
  int RandomId() const {
    CHECK(size() > 0);
    return ids_[rand() % size()];
  }

private:
  std::vector<int> ids_;
};

// Runs tests on a give Heap data structure.
template <typename T> class HeapTester {
public:
  HeapTester(std::unique_ptr<Heap<T>> heap) : heap_(std::move(heap)) {}

  void Add(T key, int id) {
    if (kDebugPrintOperations) {
      LOG(INFO) << "[Test] Add: " << key;
    }
    heap_->Add(key, id);
    CheckHeap_();

    ids_.Add(id);
    CHECK(heap_->size() == ids_.size());
    const T *lookup_key = heap_->LookUp(id);
    CHECK(lookup_key != nullptr && *lookup_key == key);
  }

  std::pair<T, int> PopMinimum() {
    auto min = heap_->Min();
    if (kDebugPrintOperations) {
      LOG(INFO) << "[Test] PopMinimum: " << min.first;
    }

    auto min2 = heap_->PopMinimum();
    CheckHeap_();
    CHECK(min == min2);

    ids_.Remove(min2.second);
    CHECK(heap_->size() == ids_.size());

    return min;
  }

  // Perform a ReduceKey operation on the heap.
  void ReduceKey(T new_key, int id) {
    if (kDebugPrintOperations) {
      LOG(INFO) << "[Test] ReduceKey: " << new_key;
    }

    heap_->ReduceKey(new_key, id);
    CheckHeap_();

    CHECK(*heap_->LookUp(id) == new_key);
    CHECK(heap_->size() == ids_.size());
  }

  // Randomly reduce a key in the heap.
  void RandomReduceKey() {
    CHECK(ids_.size() > 0);
    int id = ids_.RandomId();
    const T *key = heap_->LookUp(id);
    CHECK(key != nullptr);
    int new_key = *key - (std::rand() % (*key / 4));
    if (new_key < 0) {
      new_key = 0;
    }
    ReduceKey(new_key, id);
  }

  // Tests Add and Pop operations on the heap.
  void TestAddAndPop(int num_elements) {
    CHECK(heap_->empty());
    for (int i = 0; i < num_elements; i++) {
      int key = i * 10;
      Add(key, i);
      CHECK(heap_->Min().first == 0);
    }
    CHECK(!heap_->empty());

    for (int i = 0; i < num_elements; i++) {
      auto min = PopMinimum();
      CHECK(min.first == i * 10);
      CHECK(min.second == i);
    }
    Clear_();
  }

  // Tests ReduceKey operations on the heap.
  void TestReduceKey(int num_elements) {
    for (int i = 0; i < num_elements; ++i) {
      Add(i * 100, i);
    }

    for (int i = 0; i < num_elements; ++i) {
      int pos = std::rand() % num_elements;
      int key = *heap_->LookUp(pos);
      int new_key = key * 3 / 4;

      ReduceKey(new_key, pos);
    }
    Clear_();
  }

  void TestRandomOperations(int num_elements, int num_operations) {
    for (int i = 0; i < num_operations; ++i) {
      if (heap_->size() < num_elements) {
        int key = std::rand();
        Add(key, i);
      }

      RandomReduceKey();

      if (heap_->size() > 0 && std::rand() % 4 == 0) {
        PopMinimum();
      }
      if (heap_->size() > 0 && std::rand() % 4 == 0) {
        PopMinimum();
      }

      if (heap_->size() > 0) {
        RandomReduceKey();
      }
    }

    for (int i = 0; i < num_operations; ++i) {
      RandomReduceKey();
    }

    Clear_();
  }

private:
  // Check that the heap is well formed.
  void CheckHeap_() {
    if (kDebugPrintTree) {
      heap_->PrintTree(std::cerr, "[CheckHeap_]");
    }
    heap_->Validate();
  }

  // Pop all the nodes.
  void Clear_() {
    while (!heap_->empty()) {
      PopMinimum();
    }
  }

  std::unique_ptr<Heap<T>> heap_;
  IdSet ids_;
};

// Run all tests on heaps created by the given heap factory.
void RunTests(Factory<Heap<int>> factory) {
  std::srand(kRandomSeed);
  {
    const int num_elements = 1000;
    HeapTester<int> tester(factory());
    tester.TestAddAndPop(num_elements);
  }
  {
    const int num_elements = 1000;
    HeapTester<int> tester(factory());
    tester.TestReduceKey(num_elements);
  }
  {
    const int num_elements = 1000;
    const int num_operations = 10000;
    HeapTester<int> tester(factory());
    tester.TestRandomOperations(num_elements, num_operations);
  }
}

// Run heap tests for all the heap implementations.
void RunAllHeapTests() {
  std::vector<Factory<Heap<int>>> heap_factories{
      BinaryHeap<int>::factory(),   BinomialHeap<int>::factory(),
      WeakHeap<int>::factory(),     PairingHeap<int>::factory(),
      TwoThreeHeap<int>::factory(), FibonacciHeap<int>::factory(),
  };

  for (const auto &factory : heap_factories) {
    LOG(INFO) << "Testing " << factory.name();
    RunTests(factory);
  }
  LOG(INFO) << "Done";
}

} // namespace

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  absl::InitializeLog();

  RunAllHeapTests();

  return 0;
}
