// Weak Heap.
//
// https://en.wikipedia.org/wiki/Weak_heap

#ifndef HEAPS_WEAK_HEAP_H_
#define HEAPS_WEAK_HEAP_H_

#include <iostream>
#include <unordered_map>
#include <vector>

#include "base/factory.h"
#include "heaps/heap.h"

// WeakHeap is a multi-way tree stored as a binary tree using the
// "right-child left-sibling" convention.
template <typename T> class WeakHeap : public Heap<T> {
public:
  // A factory for this heap.
  static Factory<Heap<T>> factory() {
    return Factory<Heap<T>>("Weak Heap", []() { return new WeakHeap<T>{}; });
  };

  // Returns number of elements.
  virtual int size() const override {
    return static_cast<int>(elements_.size());
  }

  // Adds an element with key and unique int id.
  virtual void Add(T key, int id) override;

  // Updates an element with a lower key.
  virtual void ReduceKey(T new_key, int id) override;

  // Looks up a key by its id. Returns nullptr if not found.
  virtual const T *LookUp(int id) const override;

  // Returns the minimum element.
  virtual HeapElement<T> Min() const override;

  // Pops and returns the minimum key.
  virtual HeapElement<T> PopMinimum() override;

  // Print the subtree under this node.
  void PrintTree(std::ostream &out, const std::string &label) const override;

  // Validate the structure of the heap.
  virtual void Validate() const override;

private:
  // Move element at `pos` upwards until its parent is smaller.
  void SiftUp_(int pos);

  // Move the top element downwards until the constraints are satisfied.
  void SiftDown_();

  // Set an element at particular position and update index.
  void SetElement_(int pos, HeapElement<T> &&element);

  // Print the heap.
  void Print_(int pos, std::ostream &out, int level) const;

  // Print the heap recursively.
  void PrintTree_(int pos, std::ostream &out, int level) const;

  // Vector containing HeapElements.
  std::vector<HeapElement<T>> elements_;

  // Vector of ints indicating whether to reverse the left/right child.
  // If reverse_children_[i] = 0, then element at (i * 2) is its sibling and
  // the element at (i * 2 + 1) is the child. If 1, it's the reverse.
  std::vector<char> reverse_children_;

  // A map from the element int id to its index in `elements_` vector.
  std::unordered_map<int, int> id_to_index_;
};

template <typename T> void WeakHeap<T>::Add(T key, int id) {
  int pos = static_cast<int>(elements_.size());
  CHECK(id_to_index_.emplace(id, pos).second);
  elements_.emplace_back(key, id);
  reverse_children_.push_back(0);
  SiftUp_(pos);
}

template <typename T> void WeakHeap<T>::SiftUp_(int pos) {
  auto element = std::move(elements_[pos]);

  while (pos > 0) {
    // Get to the ancestor parent.
    int ancestor = pos;
    int is_right_child;
    do {
      is_right_child = ancestor & 1;
      ancestor /= 2;
    } while (reverse_children_[ancestor] == is_right_child);

    // Done if parent is smaller.
    auto &ancestor_element = elements_[ancestor];
    if (!(element.first < ancestor_element.first)) {
      break;
    }

    // Move the parent down.
    SetElement_(pos, std::move(ancestor_element));
    pos = ancestor;
  }

  // Finally place the element at pos.
  SetElement_(pos, std::move(element));
}

template <typename T> void WeakHeap<T>::SiftDown_() {
  if (elements_.size() <= 1) {
    return;
  }
  auto top_element = std::move(elements_[0]);

  // Traverse to the last child of root.
  int pos = 1;
  do {
    pos = pos * 2 + reverse_children_[pos];
  } while (pos < elements_.size());

  // Traverse the siblings up to the root.
  for (pos /= 2; pos > 0; pos /= 2) {
    if (!(elements_[pos].first < top_element.first)) {
      continue;
    }

    // Swap elements_[pos] and top_element.
    auto temp = std::move(elements_[pos]);
    SetElement_(pos, std::move(top_element));
    top_element = std::move(temp);

    // Reverse the left/right children.
    reverse_children_[pos] = 1 - reverse_children_[pos];
  }

  SetElement_(0, std::move(top_element));
}

template <typename T> const T *WeakHeap<T>::LookUp(int id) const {
  const auto it = id_to_index_.find(id);
  if (it == id_to_index_.end()) {
    return nullptr;
  }
  return &elements_[it->second].first;
}

template <typename T> HeapElement<T> WeakHeap<T>::Min() const {
  return elements_.front();
}

template <typename T> HeapElement<T> WeakHeap<T>::PopMinimum() {
  DCHECK(!elements_.empty());
  id_to_index_.erase(elements_[0].second);
  auto min_element = std::move(elements_[0]);

  if (elements_.size() == 1) {
    elements_.pop_back();
  } else {
    // Move last element to the head of the heap and sift down.
    SetElement_(0, std::move(elements_.back()));
    elements_.pop_back();
    SiftDown_();
  }
  return std::move(min_element);
}

template <typename T> void WeakHeap<T>::ReduceKey(T new_key, int id) {
  auto it = id_to_index_.find(id);
  CHECK(it != id_to_index_.end());
  int index = it->second;
  CHECK(!(elements_[index].first < new_key));
  elements_[index].first = new_key;
  SiftUp_(index);
}

template <typename T>
void WeakHeap<T>::SetElement_(int pos, HeapElement<T> &&element) {
  id_to_index_[element.second] = pos;
  elements_[pos] = std::move(element);
}

template <typename T>
void WeakHeap<T>::PrintTree(std::ostream &out, const std::string &label) const {
  out << std::endl
      << "-- Heap (" << size() << ") " << label << " --" << std::endl;

  if (size() > 0) {
    Print_(0, out, 0);
    if (size() > 1) {
      PrintTree_(1, out, 1);
    }
  }
  out << std::endl;
}

template <typename T>
void WeakHeap<T>::Print_(int pos, std::ostream &out, int level) const {
  for (int i = 0; i < level; ++i) {
    out << "| ";
  }
  const auto &element = elements_[pos];
  out << element.first << " [pos: " << pos << "][id:" << element.second
      << "][reverse: " << static_cast<int>(reverse_children_[pos]) << "]"
      << std::endl;
}

template <typename T>
void WeakHeap<T>::PrintTree_(int pos, std::ostream &out, int level) const {
  Print_(pos, out, level);

  int child_pos = pos * 2;
  int sibling_pos = child_pos;
  if (reverse_children_[pos]) {
    sibling_pos++;
  } else {
    child_pos++;
  }

  if (child_pos < elements_.size()) {
    PrintTree_(child_pos, out, level + 1);
  }
  if (sibling_pos < elements_.size()) {
    PrintTree_(sibling_pos, out, level);
  }
}

template <typename T> void WeakHeap<T>::Validate() const {
  if (elements_.size() > 0) {
    CHECK(reverse_children_[0] == 0);
  }

  for (int pos = 1; pos < elements_.size(); ++pos) {
    // Get the ancestor parent.
    int ancestor = pos;
    int is_right_child;
    do {
      is_right_child = ancestor & 1;
      ancestor /= 2;
    } while (reverse_children_[ancestor] == is_right_child);

    CHECK(!(elements_[pos].first < elements_[ancestor].first));
  }
  for (int pos = 0; pos < elements_.size(); ++pos) {
    CHECK(id_to_index_.find(elements_[pos].second)->second == pos);
  }

  CHECK(id_to_index_.size() == elements_.size());
}

#endif /* HEAPS_WEAK_HEAP_H_ */
