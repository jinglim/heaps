// Tests Heap implementations by running through all Heap operations.

#include <chrono>

#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/flags.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"

#include "heaps/binary_heap.h"
#include "heaps/binomial_heap.h"
#include "heaps/pairing_heap.h"
#include "heaps/weak_heap.h"

namespace {

const unsigned kRandomSeed = 12346789;

// If true, print out the heap operations.
const bool kDebugPrintOperations = false;

// If true, print out the heap in tree format after each operation.
const bool kDebugPrintHeaps = false;

// Keeps track of a set of int key values.
class Keys {
public:
  size_t size() const { return keys_.size(); }

  // Returns a key at the specific index (between 0 and size() - 1).
  int get(int index) const {
    CHECK(index >= 0 && index < size());
    return keys_[index];
  }

  // Add a key to the set.
  void Add(int key) { keys_.push_back(key); }

  // Remove a key.
  void Remove(int key) {
    for (int i = 0; i < keys_.size(); i++) {
      if (keys_[i] == key) {
        int last_key = keys_.back();
        keys_.pop_back();
        if (keys_.size() > 0) {
          keys_[i] = last_key;
        }
        return;
      }
    }
    LOG(FATAL) << "Key not found";
  }

private:
  std::vector<int> keys_;
};

// Runs tests on a give Heap data structure.
template <typename T> class HeapTester {
public:
  HeapTester(std::unique_ptr<Heap<T>> heap) : heap_(std::move(heap)) {}

  void Add(T value, int key) {
    if (kDebugPrintOperations) {
      LOG(INFO) << "[Test] Add: " << value;
    }
    heap_->Add(value, key);
    CheckHeap_();

    keys_.Add(key);
    CHECK(heap_->size() == keys_.size());
    CHECK(*heap_->LookUp(key) == value);
  }

  std::pair<T, int> PopMinimum() {
    auto min = heap_->Min();
    if (kDebugPrintOperations) {
      LOG(INFO) << "[Test] PopMinimum: " << min.first;
    }

    auto min2 = heap_->PopMinimum();
    CheckHeap_();
    CHECK(min == min2);

    keys_.Remove(min2.second);
    CHECK(heap_->size() == keys_.size());

    return min;
  }

  // Perform a ReduceValue on the heap.
  void ReduceValue(T new_value, int key) {
    if (kDebugPrintOperations) {
      LOG(INFO) << "[Test] ReduceValue: " << new_value;
    }

    heap_->ReduceValue(new_value, key);
    CheckHeap_();

    CHECK(*heap_->LookUp(key) == new_value);
    CHECK(heap_->size() == keys_.size());
  }

  // Randomly reduce a value in the heap.
  void RandomReduceValue() {
    int key = keys_.get(std::rand() % keys_.size());
    int value = *heap_->LookUp(key);
    int new_value = value - (std::rand() % 1000);
    if (new_value < 0) {
      new_value = 0;
    }
    ReduceValue(new_value, key);
  }

  // Tests Add and Pop operations on the heap.
  void TestAddAndPop(int num_elements) {
    CHECK(heap_->empty());
    for (int i = 0; i < num_elements; i++) {
      int value = i * 10;
      Add(value, i);
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

  // Tests ReduceValue operations on the heap.
  void TestReduceValue(int num_elements) {
    for (int i = 0; i < num_elements; ++i) {
      Add(i * 100, i);
    }

    for (int i = 0; i < num_elements; ++i) {
      int pos = std::rand() % 100;
      int value = *heap_->LookUp(pos);
      int new_value = value * 3 / 4;

      ReduceValue(new_value, pos);
    }
    Clear_();
  }

  void TestRandomOperations(int num_elements, int num_operations) {
    for (int i = 0; i < num_operations; ++i) {
      if (heap_->size() < num_elements) {
        int value = std::rand();
        Add(value, i);
      }

      RandomReduceValue();
      RandomReduceValue();

      if (heap_->size() > 0 && std::rand() % 2 == 0) {
        PopMinimum();
      }
    }

    for (int i = 0; i < num_operations; ++i) {
      RandomReduceValue();
    }

    Clear_();
  }

private:
  // Check that the heap is well formed.
  void CheckHeap_() {
    if (kDebugPrintHeaps) {
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
  Keys keys_;
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
    tester.TestReduceValue(num_elements);
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
      BinaryHeap<int>::factory(),
      BinomialHeap<int>::factory(),
      WeakHeap<int>::factory(),
      PairingHeap<int>::factory(),
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
