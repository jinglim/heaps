// Binary Heap.
//
// See https://en.wikipedia.org/wiki/Binary_heap

#ifndef HEAPS_BINARY_HEAP_H_
#define HEAPS_BINARY_HEAP_H_

#include <iostream>
#include <unordered_map>
#include <vector>

#include "base/factory.h"
#include "heaps/heap.h"

// A Binary Heap that keeps track of the elements by their keys, allowing
// lookup by key, and reducing the values.
template <typename T> class BinaryHeap : public Heap<T> {
public:
  // A factory for this heap.
  static Factory<Heap<T>> factory() {
    return Factory<Heap<T>>("Binary Heap",
                            []() { return new BinaryHeap<T>{}; });
  };

  // Returns number of elements.
  virtual int size() const override {
    return static_cast<int>(elements_.size());
  }

  // Adds a value with the associated key.
  virtual void Add(T value, int key) override;

  // Updates an element with a lower value.
  virtual void ReduceValue(T new_value, int key) override;

  // Looks up a value by its key. Returns nullptr if not found.
  virtual const T *LookUp(int key) const override;

  // Returns the minimum element.
  virtual HeapElement<T> Min() const override;

  // Pops and returns the minimum value.
  virtual HeapElement<T> PopMinimum() override;

  // Print for debugging.
  virtual void PrintTree(std::ostream &out,
                         const std::string &label) const override;

  // Validate the structure of the heap.
  virtual void Validate() const override;

private:
  // Move element at `pos` upwards until its parent is smaller.
  void SiftUp_(int pos);

  // Move element at `pos` downwards until it's smaller than the child.
  void SiftDown_(int pos);

  // Set a value at particular position and update index.
  void SetElement_(int pos, HeapElement<T> element);

  // Print the heap.
  void Print_(int pos, std::ostream &out, int level) const;

  // Vector containing {T, int key} pairs.
  std::vector<HeapElement<T>> elements_;

  // A map from the element int key to its index in `elements_` vector.
  std::unordered_map<int, int> key_to_index_;
};

template <typename T> void BinaryHeap<T>::Add(T value, int key) {
  int pos = static_cast<int>(elements_.size());
  elements_.emplace_back(HeapElement<T>{value, key});
  CHECK(key_to_index_.insert(std::make_pair(key, pos)).second);
  SiftUp_(pos);
}

template <typename T> void BinaryHeap<T>::ReduceValue(T new_value, int key) {
  auto it = key_to_index_.find(key);
  CHECK(it != key_to_index_.end());
  int index = it->second;
  CHECK(!(elements_[index].first < new_value));
  elements_[index].first = new_value;
  SiftUp_(index);
}

template <typename T> const T *BinaryHeap<T>::LookUp(int key) const {
  const auto it = key_to_index_.find(key);
  if (it == key_to_index_.end()) {
    return nullptr;
  }
  return &elements_[it->second].first;
}

template <typename T> HeapElement<T> BinaryHeap<T>::Min() const {
  return elements_.front();
}

template <typename T> HeapElement<T> BinaryHeap<T>::PopMinimum() {
  DCHECK(!elements_.empty());
  key_to_index_.erase(elements_[0].second);
  auto min = std::move(elements_[0]);

  if (elements_.size() == 1) {
    elements_.pop_back();
    return min;
  }

  // Move last element to the head of the heap and sift down.
  SetElement_(0, std::move(elements_.back()));
  elements_.pop_back();
  SiftDown_(0);
  return min;
}

template <typename T>
void BinaryHeap<T>::PrintTree(std::ostream &out,
                              const std::string &label) const {
  out << "Heap(" << label << "):" << std::endl;
  Print_(0, out, 1);
}

template <typename T> void BinaryHeap<T>::Validate() const {
  for (int pos = 1; pos < elements_.size(); ++pos) {
    int parent = (pos - 1) / 2;
    CHECK(!(elements_[pos].first < elements_[parent].first));
  }
  for (int pos = 0; pos < elements_.size(); ++pos) {
    const auto &element = elements_[pos];
    CHECK(key_to_index_.find(element.second)->second == pos);
  }
  CHECK(key_to_index_.size() == elements_.size());
}

template <typename T> void BinaryHeap<T>::SiftUp_(int pos) {
  auto element = std::move(elements_[pos]);

  while (pos > 0) {
    int parent = (pos - 1) / 2;
    const auto &parent_element = elements_[parent];

    // Done if parent is smaller.
    if (!(element.first < parent_element.first)) {
      break;
    }

    // Move the parent down.
    SetElement_(pos, std::move(parent_element));
    pos = parent;
  }

  // Finally place element at pos.
  SetElement_(pos, std::move(element));
}

template <typename T> void BinaryHeap<T>::SiftDown_(int pos) {
  auto element = std::move(elements_[pos]);
  int child = pos * 2 + 1;
  while (child < elements_.size()) {
    // If right is smaller, then set child to right.
    if (child + 1 < elements_.size() &&
        elements_[child + 1].first < elements_[child].first) {
      child++;
    }

    // Done if the child element is not smaller.
    const auto &child_element = elements_[child];
    if (!(child_element.first < element.first)) {
      break;
    }

    // Move child element up to parent pos.
    SetElement_(pos, std::move(child_element));

    pos = child;
    child = child * 2 + 1;
  }

  // Finally place element at pos.
  SetElement_(pos, std::move(element));
}

template <typename T>
void BinaryHeap<T>::SetElement_(int pos, HeapElement<T> element) {
  key_to_index_[element.second] = pos;
  elements_[pos] = std::move(element);
}

template <typename T>
void BinaryHeap<T>::Print_(int pos, std::ostream &out, int level) const {
  for (int i = 0; i < level; ++i) {
    out << "| ";
  }
  const auto &element = elements_[pos];
  out << "[" << element.first << ",key:" << element.second << "] " << std::endl;

  for (int child = 1; child <= 2; child++) {
    int child_pos = pos * 2 + child;
    if (child_pos < elements_.size()) {
      Print_(child_pos, out, level + 1);
    }
  }
}

#endif /* HEAPS_BINARY_HEAP_H_ */
