// Pairing Heap.
//
// See https://en.wikipedia.org/wiki/Pairing_heap

#ifndef HEAPS_PAIRING_HEAP_H_
#define HEAPS_PAIRING_HEAP_H_

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base/factory.h"
#include "heaps/heap.h"

// A node used in Pairing Heaps.
template <typename T> class PairingHeapNode {
public:
  PairingHeapNode(T value, int key)
      : value_(value), key_(key), child_(nullptr), left_(nullptr),
        right_(nullptr) {}

  static void DeleteTree(PairingHeapNode<T> *node) {
    if (node != nullptr) {
      DeleteTree(node->child_);
      DeleteTree(node->right_);
      delete node;
    }
  }

  const T &value() const { return value_; }
  void set_value(T value) { value_ = value; }

  int key() const { return key_; }

  // Returns the child node.
  PairingHeapNode<T> *child() const { return child_; }

  // Returns the previous sibling, or the parent if it has no previous sibling.
  PairingHeapNode<T> *left() const { return left_; }

  // Returns the next sibling.
  PairingHeapNode<T> *right() const { return right_; }

  // Add a child to this node.
  void AddChild(PairingHeapNode<T> *child);

  // Remove the node from its parent.
  void DetachFromParent();

  // Debug information about this node.
  std::string DebugString() const;

  // Print the subtree under this node.
  void PrintTree(std::ostream &out, const std::string &label) const;

  // Print out node information recursively.
  void PrintTree(std::ostream &out, int level) const;

  // Validate the fields.
  void Validate(std::unordered_set<int> *seen_keys) const;

  // Merge two trees.
  static PairingHeapNode<T> *MergeTrees(PairingHeapNode<T> *a,
                                        PairingHeapNode<T> *b);

  // Merge a list of trees.
  static PairingHeapNode<T> *MergeTreeList(PairingHeapNode<T> *tree_list);

private:
  T value_;

  // A key that uniquely identifies this node.
  int key_;

  // Points to the first child.
  PairingHeapNode<T> *child_;

  // Points to the previous sibling. If it has no previous sibling,
  // this points to its parent.
  PairingHeapNode<T> *left_;

  // Points to next sibling.
  PairingHeapNode<T> *right_;
};

template <typename T>
void PairingHeapNode<T>::AddChild(PairingHeapNode<T> *child) {
  if (child_ != nullptr) {
    child_->left_ = child;
  }
  child->left_ = this;
  child->right_ = child_;
  child_ = child;
}

template <typename T>
PairingHeapNode<T> *PairingHeapNode<T>::MergeTrees(PairingHeapNode<T> *a,
                                                   PairingHeapNode<T> *b) {
  if (a->value_ < b->value_) {
    a->AddChild(b);
    return a;
  } else {
    b->AddChild(a);
    return b;
  }
}

template <typename T>
PairingHeapNode<T> *
PairingHeapNode<T>::MergeTreeList(PairingHeapNode<T> *tree_list) {
  if (tree_list == nullptr) {
    return nullptr;
  }

  // Merge pairs from left to right.
  PairingHeapNode<T> *merged_head = nullptr;
  auto *node = tree_list;
  do {
    auto *next = node->right_;

    if (next == nullptr) {
      node->right_ = merged_head;
      merged_head = node;
      break;
    }

    auto *next_next = next->right_;
    auto *merged = MergeTrees(node, next);
    merged->right_ = merged_head;
    merged_head = merged;

    node = next_next;
  } while (node != nullptr);

  // Merge pairs from right to left.
  node = merged_head->right_;
  merged_head->right_ = nullptr;
  while (node != nullptr) {
    auto *next = node->right_;
    node->right_ = nullptr;
    merged_head = MergeTrees(node, merged_head);
    node = next;
  }

  merged_head->left_ = nullptr;
  return merged_head;
}

template <typename T> void PairingHeapNode<T>::DetachFromParent() {
  if (left_->child_ == this) {
    // This is the first child. left_ is the parent.
    left_->child_ = right_;
  } else {
    left_->right_ = right_;
  }
  if (right_ != nullptr) {
    right_->left_ = left_;
  }
  left_ = nullptr;
  right_ = nullptr;
}

template <typename T> std::string PairingHeapNode<T>::DebugString() const {
  std::stringstream out;
  out << value_ << " [key:" << key_ << "]";

  if (left_ != nullptr) {
    out << "[left:" << left_->value_ << "]";
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
void PairingHeapNode<T>::PrintTree(std::ostream &out,
                                   const std::string &label) const {
  out << label << ":" << std::endl;
  PrintTree(out, 0);
  out << std::endl;
}

template <typename T>
void PairingHeapNode<T>::PrintTree(std::ostream &out, int level) const {
  for (int i = 0; i < level; ++i) {
    out << "| ";
  }
  out << DebugString() << std::endl;

  if (child_ != nullptr) {
    child_->PrintTree(out, level + 1);
  }

  if (right_ != nullptr) {
    right_->PrintTree(out, level);
  }
}

template <typename T>
void PairingHeapNode<T>::Validate(std::unordered_set<int> *seen_keys) const {
  if (seen_keys != nullptr) {
    CHECK(seen_keys->insert(key_).second);
  }

  if (child_ != nullptr) {
    CHECK(child_->left_ == this);
    child_->Validate(seen_keys);
  }

  if (right_ != nullptr) {
    CHECK(right_->left_ == this);
    right_->Validate(seen_keys);
  }
}

template <typename T> class PairingHeap : public Heap<T> {
public:
  PairingHeap() : root_(nullptr) {}
  ~PairingHeap() { PairingHeapNode<T>::DeleteTree(root_); }

  static Factory<Heap<T>> factory() {
    return Factory<Heap<T>>("Pairing Heap",
                            []() { return new PairingHeap<T>{}; });
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
  // The min root node. Maybe null.
  PairingHeapNode<T> *root_;

  // Map of each key to the node.
  std::unordered_map<int, PairingHeapNode<T> *> key_to_node_;
};

template <typename T> void PairingHeap<T>::Add(T value, int key) {
  PairingHeapNode<T> *node = new PairingHeapNode<T>{value, key};
  CHECK(key_to_node_.emplace(key, node).second);

  // If no root. Make this the root.
  if (root_ == nullptr) {
    root_ = node;
  } else {
    root_ = PairingHeapNode<T>::MergeTrees(root_, node);
  }
}

template <typename T> void PairingHeap<T>::ReduceValue(T new_value, int key) {
  auto *node = key_to_node_[key];
  DCHECK(!(node->value() < new_value));
  node->set_value(new_value);

  if (node == root_) {
    return;
  }
  node->DetachFromParent();
  root_ = PairingHeapNode<T>::MergeTrees(root_, node);
}

template <typename T> const T *PairingHeap<T>::LookUp(int key) const {
  const auto it = key_to_node_.find(key);
  if (it == key_to_node_.end()) {
    return nullptr;
  }
  return &it->second->value();
}

template <typename T> HeapElement<T> PairingHeap<T>::Min() const {
  DCHECK(size() > 0);
  return std::make_pair(root_->value(), root_->key());
}

template <typename T> HeapElement<T> PairingHeap<T>::PopMinimum() {
  DCHECK(size() > 0);

  auto *min_root = root_;

  auto *children = min_root->child();
  root_ = PairingHeapNode<T>::MergeTreeList(children);

  auto result = std::make_pair(min_root->value(), min_root->key());
  key_to_node_.erase(min_root->key());
  delete min_root;
  return result;
}

template <typename T>
void PairingHeap<T>::PrintTree(std::ostream &out,
                               const std::string &label) const {
  out << std::endl
      << "-- Heap (" << size() << ") " << label << " --" << std::endl;
  for (const auto *root = root_; root != nullptr; root = root->right()) {
    out << "Tree" << std::endl;
    root->PrintTree(out, 1);
  }
  out << std::endl;
}

template <typename T> void PairingHeap<T>::Validate() const {
  std::unordered_set<int> seen_keys;
  if (root_ != nullptr) {
    CHECK(root_->left() == nullptr);
    CHECK(root_->right() == nullptr);

    root_->Validate(&seen_keys);
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

#endif /* HEAPS_PAIRING_HEAP_H_ */
