// Fibonacci Heap.
//
// See https://en.wikipedia.org/wiki/Fibonacci_heap

#ifndef HEAPS_FIBONACCI_HEAP_H_
#define HEAPS_FIBONACCI_HEAP_H_

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "base/factory.h"
#include "heaps/heap.h"

// A node used in Fibonnaci Heaps.
template <typename T> class FibonacciHeapNode {
public:
  FibonacciHeapNode(T key, int id)
      : key_(key), id_(id), degree_(0), marked_(false), parent_(nullptr),
        child_(nullptr), left_(this), right_(this) {}

  // Sentinel node.
  FibonacciHeapNode()
      : id_(-1), degree_(0), marked_(false), parent_(nullptr), child_(nullptr),
        left_(this), right_(this) {}

  const T &key() const { return key_; }
  void set_key(T key) { key_ = key; }

  int id() const { return id_; }

  // Number of children.
  int degree() const { return degree_; }

  FibonacciHeapNode<T> *parent() const { return parent_; }
  void clear_parent() { parent_ = nullptr; }

  FibonacciHeapNode<T> *child() const { return child_; }

  FibonacciHeapNode<T> *left() const { return left_; }
  void set_left(FibonacciHeapNode<T> *node) { left_ = node; }

  FibonacciHeapNode<T> *right() const { return right_; }
  void set_right(FibonacciHeapNode<T> *node) { right_ = node; }

  // Clear references to left and right siblings.
  void clear_siblings() {
    left_ = this;
    right_ = this;
  }

  // Whether a child has been cut from it.
  bool marked() const { return marked_; }
  void set_mark() { marked_ = true; }
  void clear_mark() { marked_ = false; }

  // Delete the entire tree rooted at this node.
  static void DeleteTree(FibonacciHeapNode<T> *node) {
    if (node != nullptr) {
      auto *child = node->child_;
      if (child != nullptr) {
        do {
          auto *right = child->right_;
          DeleteTree(child);
          child = right;
        } while (child != node->child_);
      }
      delete node;
    }
  }

  // Append a sibling to this node.
  void AddSibling(FibonacciHeapNode<T> *node);

  // Detach from its siblings.
  void DetachFromSiblings();

  // Add a child node (increasing the degree).
  void AddChild(FibonacciHeapNode<T> *node);

  // Cut this node from its parents/siblings.
  void Cut();

  // Debug information about this node.
  std::string DebugString() const;

  // Print out node information recursively.
  void PrintTree(std::ostream &out, int level) const;

  // Validate the fields.
  void Validate(std::unordered_set<int> *seen_ids) const;

private:
  T key_;

  // An int id that uniquely identifies this node.
  int id_;

  // Number of children.
  short degree_;

  // Whether a child has been cut from this node.
  bool marked_;

  // Points to the parent node.
  FibonacciHeapNode<T> *parent_;

  // Points to the first child.
  FibonacciHeapNode<T> *child_;

  // Points to prev sibling.
  FibonacciHeapNode<T> *left_;

  // Points to next sibling.
  FibonacciHeapNode<T> *right_;
};

template <typename T>
void FibonacciHeapNode<T>::AddSibling(FibonacciHeapNode<T> *node) {
  node->left_ = left_;
  node->right_ = this;
  left_->right_ = node;
  left_ = node;
}

template <typename T> void FibonacciHeapNode<T>::DetachFromSiblings() {
  left_->right_ = right_;
  right_->left_ = left_;
  left_ = this;
  right_ = this;
}

template <typename T>
void FibonacciHeapNode<T>::AddChild(FibonacciHeapNode<T> *node) {
  DCHECK(node->left_ == node);
  DCHECK(node->right_ == node);
  if (child_ != nullptr) {
    node->left_ = child_->left_;
    node->right_ = child_;
    child_->left_->right_ = node;
    child_->left_ = node;
  }
  child_ = node;
  node->parent_ = this;
  ++degree_;
}

template <typename T> void FibonacciHeapNode<T>::Cut() {
  if (parent_ != nullptr) {
    if (parent_->child_ == this) {
      if (left_ == this) {
        parent_->child_ = nullptr;
      } else {
        parent_->child_ = right_;
      }
    }
    parent_->degree_--;
    parent_ = nullptr;
  }

  left_->right_ = right_;
  right_->left_ = left_;
  left_ = this;
  right_ = this;
}

template <typename T> std::string FibonacciHeapNode<T>::DebugString() const {
  std::stringstream out;
  out << key_ << " [id:" << id_ << "][deg:" << degree_ << "][marked:" << marked_
      << "]";

  if (parent_ != nullptr) {
    out << "[parent:" << parent_->key_ << "]";
  } else {
    out << "[parent:null]";
  }
  if (left_ != nullptr) {
    out << "[left=" << left_->key_ << "]";
  }
  if (right_ != nullptr) {
    out << "[right=" << right_->key_ << "]";
  }
  return out.str();
}

template <typename T>
void FibonacciHeapNode<T>::PrintTree(std::ostream &out, int level) const {
  for (int i = 0; i < level; ++i) {
    out << "| ";
  }
  out << DebugString() << std::endl;

  if (child_ != nullptr) {
    const auto *child = child_;
    do {
      child->PrintTree(out, level + 1);
      child = child->right_;
    } while (child != child_);
  }
}

template <typename T>
void FibonacciHeapNode<T>::Validate(std::unordered_set<int> *seen_ids) const {
  if (seen_ids != nullptr) {
    CHECK(seen_ids->insert(id_).second);
  }

  if (child_ != nullptr) {
    int num_children = 0;
    const auto *child = child_;
    do {
      CHECK(child->parent_ == this);
      CHECK(child->right_->left_ == child);
      CHECK(child->left_->right_ == child);
      child->Validate(seen_ids);
      child = child->right_;
      num_children++;
    } while (child != child_);
    CHECK(degree_ == num_children);
  }
}

template <typename T> class FibonacciHeap : public Heap<T> {
public:
  FibonacciHeap() : min_root_(nullptr) {}

  ~FibonacciHeap() {
    auto *root = roots_.right();
    while (root != &roots_) {
      auto *next_root = root->right();
      FibonacciHeapNode<T>::DeleteTree(root);
      root = next_root;
    }
  }

  static Factory<Heap<T>> factory() {
    return Factory<Heap<T>>("Fibonacci Heap",
                            []() { return new FibonacciHeap<T>{}; });
  }

  // Returns number of elements.
  virtual int size() const override {
    return static_cast<int>(id_to_node_.size());
  }

  // Adds an elementn with given key and unique int key.
  virtual void Add(T key, int id) override;

  // Updates with a lower key.
  virtual void ReduceKey(T new_key, int id) override;

  // Looks up a key by id. Returns nullptr if not found.
  virtual const T *LookUp(int id) const override;

  // Returns the min element.
  virtual HeapElement<T> Min() const override;

  // Pops and returns the minimum key.
  virtual HeapElement<T> PopMinimum() override;

  // Print for debugging.
  virtual void PrintTree(std::ostream &out,
                         const std::string &label) const override;

  // Validate the invariants.
  virtual void Validate() const override;

private:
  // Merge a root into roots_by_degree_.
  void MergeRoot_(FibonacciHeapNode<T> *root);

  // Used for merging roots of the same degree.
  std::vector<FibonacciHeapNode<T> *> roots_by_degree_;

  // Current root node with min key.
  FibonacciHeapNode<T> *min_root_;

  // Linked list of new root nodes.
  FibonacciHeapNode<T> roots_;

  // Map of each id to the node.
  std::unordered_map<int, FibonacciHeapNode<T> *> id_to_node_;
};

template <typename T> void FibonacciHeap<T>::Add(T key, int id) {
  FibonacciHeapNode<T> *node = new FibonacciHeapNode<T>{key, id};
  CHECK(id_to_node_.emplace(id, node).second);

  roots_.AddSibling(node);
  if (min_root_ == nullptr || key < min_root_->key()) {
    min_root_ = node;
  }
}

template <typename T>
void FibonacciHeap<T>::MergeRoot_(FibonacciHeapNode<T> *root) {
  while (true) {
    int degree = root->degree();
    if (roots_by_degree_.size() < degree + 1) {
      roots_by_degree_.resize(degree + 1, nullptr);
    }

    auto *root2 = roots_by_degree_[degree];
    if (root2 == nullptr) {
      roots_by_degree_[degree] = root;
      break;
    }

    // Merge with existing tree of same degree. The degree will increase
    // by 1, and it may need to be merged with another tree.
    roots_by_degree_[degree] = nullptr;

    if (root->key() < root2->key()) {
      root->AddChild(root2);
    } else {
      root2->AddChild(root);
      root = root2;
    }
  }
}

template <typename T> void FibonacciHeap<T>::ReduceKey(T new_key, int id) {
  auto *node = id_to_node_[id];
  node->set_key(new_key);

  // Make it the min_root if necessary.
  if (new_key < min_root_->key()) {
    min_root_ = node;
  }

  // If this is a root node, or if the new key is not smaller than its parent,
  // we're done.
  auto *parent = node->parent();
  if (parent == nullptr || !(new_key < parent->key())) {
    return;
  }

  // Cut the node from its parent.
  node->Cut();
  roots_.AddSibling(node);

  // If the parent has previously cut its child before, it needs to be cut
  // as well.
  do {
    if (!parent->marked()) {
      parent->set_mark();
      break;
    }
    parent->clear_mark();

    // Cut the parent and move to new roots list.
    auto *next_parent = parent->parent();
    parent->Cut();
    roots_.AddSibling(parent);
    parent = next_parent;
  } while (parent != nullptr);
}

template <typename T> const T *FibonacciHeap<T>::LookUp(int id) const {
  const auto it = id_to_node_.find(id);
  if (it == id_to_node_.end()) {
    return nullptr;
  }
  return &it->second->key();
}

template <typename T> HeapElement<T> FibonacciHeap<T>::Min() const {
  DCHECK(size() > 0);
  return std::make_pair(min_root_->key(), min_root_->id());
}

template <typename T> HeapElement<T> FibonacciHeap<T>::PopMinimum() {
  auto result = std::make_pair(min_root_->key(), min_root_->id());

  // Detach the min root.
  auto *child = min_root_->child();
  min_root_->Cut();

  // Clean up
  id_to_node_.erase(min_root_->id());
  delete min_root_;

  // Merge new roots into roots_by_degree_.
  auto *root = roots_.right();
  while (root != &roots_) {
    auto *next_root = root->right();
    root->clear_siblings();
    MergeRoot_(root);
    root = next_root;
  }
  roots_.clear_siblings();

  // Merge children of min_root into roots_by_degree_;.
  if (child != nullptr) {
    root = child;
    do {
      auto *next_root = root->right();
      root->clear_parent();
      root->clear_siblings();
      MergeRoot_(root);
      root = next_root;
    } while (root != child);
  }

  // Find the new minimum among the roots_by_degree_.
  min_root_ = nullptr;
  for (auto *root : roots_by_degree_) {
    if (root != nullptr) {
      DCHECK(root->parent() == nullptr);
      roots_.AddSibling(root);

      if (min_root_ == nullptr || root->key() < min_root_->key()) {
        min_root_ = root;
      }
    }
  }
  roots_by_degree_.clear();

  return result;
}

template <typename T>
void FibonacciHeap<T>::PrintTree(std::ostream &out,
                                 const std::string &label) const {
  out << "-- Heap (" << size() << ") " << label << " --" << std::endl;
  if (min_root_ != nullptr) {
    out << "min:" << min_root_->DebugString() << std::endl;
  }
  for (int i = 0; i < roots_by_degree_.size(); i++) {
    if (roots_by_degree_[i] != nullptr) {
      out << "Deg(" << i << "):" << std::endl;
      roots_by_degree_[i]->PrintTree(out, 1);
    }
  }

  const auto *root = roots_.right();
  if (root != &roots_) {
    out << "new:" << std::endl;
    do {
      root->PrintTree(out, 1);
      root = root->right();
    } while (root != &roots_);
  }
  out << std::endl;
}

template <typename T> void FibonacciHeap<T>::Validate() const {
  if (size() == 0) {
    CHECK(min_root_ == nullptr);
    CHECK(roots_.right() == &roots_);
    CHECK(roots_.left() == &roots_);
    return;
  }

  CHECK(roots_by_degree_.empty());

  std::unordered_set<int> seen_ids;
  for (const auto *root = roots_.right(); root != &roots_;
       root = root->right()) {
    root->Validate(&seen_ids);
  }

  if (seen_ids.size() != size()) {
    for (const auto &entry : id_to_node_) {
      if (seen_ids.find(entry.first) == seen_ids.end()) {
        LOG(ERROR) << "Id not seen: " << entry.first;
      }
    }
    LOG(FATAL) << "Some ids are missing";
  }
}

#endif /* HEAPS_FIBONACCI_HEAP_H_ */
