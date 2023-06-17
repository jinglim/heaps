// Binomial Heap.
//
// See https://en.wikipedia.org/wiki/Binomial_heap

#ifndef HEAPS_BINOMIAL_HEAP_H_
#define HEAPS_BINOMIAL_HEAP_H_

#include <iostream>
#include <sstream>
#include <unordered_set>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "base/factory.h"
#include "heaps/heap.h"

// A node used in Binomial Heaps.
template <typename T> class BinomialNode {
public:
  BinomialNode(T value, int key)
      : value_(value), key_(key), dimension_(0), parent_(nullptr),
        child_(nullptr), right_(nullptr) {}

  // Used as a sentinal node only.
  BinomialNode() : right_(nullptr) {}

  const T &value() const { return value_; }
  void set_value(T value) { value_ = value; }

  int key() const { return key_; }
  void set_key(int key) { key_ = key; }

  short dimension() const { return dimension_; }

  BinomialNode<T> *parent() const { return parent_; }

  // Returns true if this is one of the root nodes.
  bool is_root() const { return parent_ == nullptr; }

  // Returns the highest dimension child (with dimension = dimension() - 1).
  BinomialNode<T> *child() const { return child_; }
  void clear_child() { child_ = nullptr; }

  // Returns the next sibling.
  // The sibling has a lower dimesion, except the root list which has ascending
  // dimensions.
  BinomialNode<T> *right() const { return right_; }
  void set_right(BinomialNode<T> *right) { right_ = right; }

  // Delete the entire tree rooted at this node.
  void DeleteTree() {
    delete right_;
    delete child_;
  }

  // Remove the children of this node and return them in a list in ascending
  // dimension order. Used for merging with root node list.
  BinomialNode<T> *DetachChildren();

  // Debug information about this node.
  std::string DebugString() const;

  // Print the subtree under this node.
  void PrintTree(std::ostream &out, const std::string &label) const;

  // Print out node information recursively.
  void PrintTree(std::ostream &out, int level) const;

  // Validate the fields.
  void Validate(std::unordered_set<int> *seen_keys) const;

  // Merge two trees a and b.
  static BinomialNode<T> *MergeTrees(BinomialNode<T> *a, BinomialNode<T> *b);

  // Merge two list of trees and their siblings (in ascending dimension).
  static BinomialNode<T> *MergeTreeList(BinomialNode<T> *a, BinomialNode<T> *b);

private:
  T value_;

  // A key that uniquely identifies this node.
  int key_;

  // Dimension of the tree at this node.
  short dimension_;

  // Points to the parent node.
  BinomialNode<T> *parent_;

  // Points to the highest dimension child.
  BinomialNode<T> *child_;

  // Points to next sibling.
  BinomialNode<T> *right_;
};

template <typename T>
BinomialNode<T> *BinomialNode<T>::MergeTrees(BinomialNode<T> *a,
                                             BinomialNode<T> *b) {
  DCHECK(a->dimension_ == b->dimension_);

  if (b->value_ < a->value_) {
    auto *temp = b;
    b = a;
    a = temp;
  }

  // Make b a child of a.
  b->right_ = a->child_;
  b->parent_ = a;
  a->child_ = b;
  a->dimension_++;
  return a;
}

template <typename T>
BinomialNode<T> *BinomialNode<T>::MergeTreeList(BinomialNode<T> *a,
                                                BinomialNode<T> *b) {
  BinomialNode<T> *node_a = a;
  BinomialNode<T> *node_b = b;

  BinomialNode<T> merged_sentinel;
  BinomialNode<T> *merged = &merged_sentinel;

  while (true) {
    if (node_a == nullptr) {
      merged->right_ = node_b;
      break;
    }
    if (node_b == nullptr) {
      merged->right_ = node_a;
      break;
    }

    if (node_a->dimension() == node_b->dimension()) {
      // Detach nodes a and b from the siblings to prepare to merge both
      // subtrees.
      auto *next_node_a = node_a->right_;
      node_a->right_ = nullptr;
      auto *next_node_b = node_b->right_;
      node_b->right_ = nullptr;

      // Merging yields a carry tree of a higher dimension.
      // Merge the carry tree with either a or b.
      BinomialNode<T> *carry = BinomialNode<T>::MergeTrees(node_a, node_b);
      if (next_node_a == nullptr) {
        node_a = carry;
        node_b = next_node_b;
      } else if (node_b == nullptr) {
        node_a = next_node_a;
        node_b = carry;
      } else {
        node_a = MergeTreeList(carry, next_node_a);
        node_b = next_node_b;
      }
      continue;
    }

    // Append the lower dimension node to the merged list.
    if (node_a->dimension() < node_b->dimension()) {
      merged->right_ = node_a;
      merged = node_a;
      node_a = node_a->right_;
    } else {
      merged->right_ = node_b;
      merged = node_b;
      node_b = node_b->right_;
    }
  }

  return merged_sentinel.right_;
}

template <typename T> BinomialNode<T> *BinomialNode<T>::DetachChildren() {
  BinomialNode<T> *prev_child = nullptr;
  BinomialNode<T> *child = child_;
  while (child != nullptr) {
    auto *next = child->right_;
    child->parent_ = nullptr;
    child->right_ = prev_child;
    prev_child = child;
    child = next;
  }
  return prev_child;
}

template <typename T>
void BinomialNode<T>::PrintTree(std::ostream &out,
                                const std::string &label) const {
  out << label << ":" << std::endl;
  PrintTree(out, 0);
  out << std::endl;
}

template <typename T> std::string BinomialNode<T>::DebugString() const {
  std::stringstream out;
  out << value_ << " [key:" << key_ << "][dim:" << dimension_ << "]";

  if (parent_ != nullptr) {
    out << "[parent:" << parent_->value_ << "]";
  } else {
    out << "[parent:null]";
  }

  if (right_ != nullptr) {
    out << "[right=" << right_->value() << "]";
  }
  if (child_ != nullptr) {
    out << "[child=" << child_->value() << "]";
  }

  return out.str();
}

template <typename T>
void BinomialNode<T>::PrintTree(std::ostream &out, int level) const {
  for (int i = 0; i < level; ++i) {
    out << "| ";
  }
  out << DebugString() << std::endl;

  if (child_ != nullptr) {
    child_->PrintTree(out, level + 1);
  }

  if (!is_root() && right_ != nullptr) {
    right_->PrintTree(out, level);
  }
}

template <typename T>
void BinomialNode<T>::Validate(std::unordered_set<int> *seen_keys) const {
  if (seen_keys != nullptr) {
    CHECK(seen_keys->insert(key_).second);
  }

  if (dimension_ > 0) {
    CHECK(child_->parent_ == this);
    CHECK(child_->dimension_ == dimension_ - 1);
    child_->Validate(seen_keys);

    if (!is_root()) {
      CHECK(right_->parent_ == parent_);
      CHECK(right_->dimension_ == dimension_ - 1);
      right_->Validate(seen_keys);
    }
  } else {
    CHECK(child_ == nullptr);
    if (!is_root()) {
      CHECK(right_ == nullptr);
    }
  }
}

template <typename T> class BinomialHeap : public Heap<T> {
  typedef Heap<T> super;

public:
  BinomialHeap() : root_(nullptr) {}
  ~BinomialHeap() {
    if (root_ != nullptr) {
      root_->DeleteTree();
    }
  }

  static Factory<Heap<T>> factory() {
    return Factory<Heap<T>>("Binomial Heap",
                            []() { return new BinomialHeap<T>{}; });
  }

  // Returns number of elements.
  virtual int size() const override {
    return static_cast<int>(key_to_node_.size());
  }

  // Adds an value with the associated key.
  virtual void Add(T value, int key) override;

  // Updates with a lower value.
  virtual void ReduceValue(T new_value, int key) override;

  // Looks up a value by key. Returns nullptr if not found.
  virtual const T *LookUp(int key) const override;

  // Returns the min element.
  virtual HeapElement<T> Min() const override;

  // Pops and returns the minimum value.
  virtual HeapElement<T> PopMinimum() override;

  // Print for debugging.
  virtual void PrintTree(std::ostream &out,
                         const std::string &label) const override;

  // Validate the invariants.
  virtual void Validate() const override;

private:
  void SiftUp_(BinomialNode<T> *node);

  // Returns the minimum element, and sets the previous sibling of the min
  // element.
  BinomialNode<T> *Min_(BinomialNode<T> **prev_node) const;

  // Linked list of root nodes starting from lowest dimension.
  BinomialNode<T> *root_;

  // Map of each key to the node.
  std::unordered_map<int, BinomialNode<T> *> key_to_node_;
};

template <typename T> void BinomialHeap<T>::Add(T value, int key) {
  BinomialNode<T> *node = new BinomialNode<T>{value, key};
  CHECK(key_to_node_.insert(std::make_pair(key, node)).second);

  // If no root. Make this the root.
  if (root_ == nullptr) {
    root_ = node;
  } else {
    root_ = BinomialNode<T>::MergeTreeList(root_, node);
  }
}

template <typename T> void BinomialHeap<T>::ReduceValue(T new_value, int key) {
  auto *node = key_to_node_[key];
  node->set_value(new_value);
  SiftUp_(node);
}

template <typename T> const T *BinomialHeap<T>::LookUp(int key) const {
  const auto it = key_to_node_.find(key);
  if (it == key_to_node_.end()) {
    return nullptr;
  }
  return &it->second->value();
}

template <typename T> HeapElement<T> BinomialHeap<T>::Min() const {
  BinomialNode<T> *unused_node;
  const auto *min_node = Min_(&unused_node);
  return std::make_pair(min_node->value(), min_node->key());
}

template <typename T>
BinomialNode<T> *BinomialHeap<T>::Min_(BinomialNode<T> **prev_node) const {
  DCHECK(!super::empty());

  BinomialNode<T> *prev = root_;
  BinomialNode<T> *min_root = root_;
  BinomialNode<T> *min_root_prev = nullptr;
  for (auto *root = root_->right(); root != nullptr;
       prev = root, root = root->right()) {
    if (root->value() < min_root->value()) {
      min_root = root;
      min_root_prev = prev;
    }
  }
  *prev_node = min_root_prev;
  return min_root;
}

template <typename T> HeapElement<T> BinomialHeap<T>::PopMinimum() {
  BinomialNode<T> *prev_node;
  BinomialNode<T> *min_root = Min_(&prev_node);

  if (prev_node != nullptr) {
    prev_node->set_right(min_root->right());
  } else {
    root_ = min_root->right();
  }

  auto *children = min_root->DetachChildren();
  root_ = BinomialNode<T>::MergeTreeList(root_, children);

  auto result = std::make_pair(min_root->value(), min_root->key());
  key_to_node_.erase(min_root->key());
  delete min_root;
  return result;
}

template <typename T> void BinomialHeap<T>::SiftUp_(BinomialNode<T> *node) {
  T value = node->value();
  int key = node->key();
  while (true) {
    auto *parent = node->parent();

    // Done if parent is root or has smaller value.
    if (parent == nullptr || !(value < parent->value())) {
      break;
    }

    // Move the parent down.
    node->set_value(parent->value());
    node->set_key(parent->key());
    key_to_node_[parent->key()] = node;

    node = parent;
  }

  // Finally place element at pos.
  node->set_value(value);
  node->set_key(key);
  key_to_node_[key] = node;
}

template <typename T>
void BinomialHeap<T>::PrintTree(std::ostream &out,
                                const std::string &label) const {
  out << std::endl
      << "-- Heap (" << size() << ") " << label << " --" << std::endl;
  for (const auto *root = root_; root != nullptr; root = root->right()) {
    out << "Tree #" << root->dimension() << std::endl;
    root->PrintTree(out, 1);
  }
  out << std::endl;
}

template <typename T> void BinomialHeap<T>::Validate() const {
  int prev_dimension = -1;
  std::unordered_set<int> seen_keys;
  for (auto *root = root_; root != nullptr; root = root->right()) {
    CHECK(root->is_root());
    CHECK(root->dimension() > prev_dimension);
    root->Validate(&seen_keys);
    prev_dimension = root->dimension();
  }

  if (seen_keys.size() != size()) {
    for (const auto &entry : key_to_node_) {
      if (seen_keys.find(entry.first) == seen_keys.end()) {
        LOG(ERROR) << "Key not seen: " << entry.first << std::endl;
      }
    }
    LOG(FATAL) << "Some keys are missing";
  }
}

#endif /* HEAPS_BINOMIAL_HEAP_H_ */
