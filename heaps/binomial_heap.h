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
template <typename T> class BinomialHeapNode {
public:
  BinomialHeapNode(T key, int id)
      : key_(key), id_(id), dimension_(0), parent_(nullptr), child_(nullptr),
        right_(nullptr) {}

  // Used as a sentinal node only.
  BinomialHeapNode() : right_(nullptr) {}

  const T &key() const { return key_; }
  void set_key(T key) { key_ = key; }

  int id() const { return id_; }
  void set_id(int id) { id_ = id; }

  short dimension() const { return dimension_; }

  BinomialHeapNode<T> *parent() const { return parent_; }

  // Returns true if this is one of the root nodes.
  bool is_root() const { return parent_ == nullptr; }

  // Returns the highest dimension child (with dimension = dimension() - 1).
  BinomialHeapNode<T> *child() const { return child_; }
  void clear_child() { child_ = nullptr; }

  // Returns the next sibling.
  // The sibling has a lower dimesion, except the root list which has ascending
  // dimensions.
  BinomialHeapNode<T> *right() const { return right_; }
  void set_right(BinomialHeapNode<T> *right) { right_ = right; }

  // Delete the entire tree rooted at this node.
  static void DeleteTree(BinomialHeapNode<T> *node) {
    if (node != nullptr) {
      DeleteTree(node->child_);
      DeleteTree(node->right_);
      delete node;
    }
  }

  // Remove the children of this node and return them in a list in ascending
  // dimension order. Used for merging with root node list.
  BinomialHeapNode<T> *DetachChildren();

  // Debug information about this node.
  std::string DebugString() const;

  // Print the subtree under this node.
  void PrintTree(std::ostream &out, const std::string &label) const;

  // Print out node information recursively.
  void PrintTree(std::ostream &out, int level) const;

  // Validate the fields.
  void Validate(std::unordered_set<int> *seen_ids) const;

  // Merge two trees a and b.
  static BinomialHeapNode<T> *MergeTrees(BinomialHeapNode<T> *a,
                                         BinomialHeapNode<T> *b);

  // Merge two list of trees and their siblings (in ascending dimension).
  static BinomialHeapNode<T> *MergeTreeLists(BinomialHeapNode<T> *a,
                                             BinomialHeapNode<T> *b);

  static BinomialHeapNode<T> *AddToTreeList(BinomialHeapNode<T> *tree,
                                            BinomialHeapNode<T> *tree_list);

private:
  T key_;

  // An int id that uniquely identifies this node.
  int id_;

  // Dimension of the tree at this node.
  short dimension_;

  // Points to the parent node.
  BinomialHeapNode<T> *parent_;

  // Points to the highest dimension child.
  BinomialHeapNode<T> *child_;

  // Points to next sibling.
  BinomialHeapNode<T> *right_;
};

template <typename T>
BinomialHeapNode<T> *BinomialHeapNode<T>::MergeTrees(BinomialHeapNode<T> *a,
                                                     BinomialHeapNode<T> *b) {
  DCHECK(a->dimension_ == b->dimension_);

  if (b->key_ < a->key_) {
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
BinomialHeapNode<T> *
BinomialHeapNode<T>::AddToTreeList(BinomialHeapNode<T> *tree,
                                   BinomialHeapNode<T> *tree_list) {
  auto dim = tree->dimension();

  // Iterate through the tree list until we find the place to insert or
  // merge the `tree` node.
  BinomialHeapNode<T> *prev_root = nullptr;
  BinomialHeapNode<T> *next_root;
  for (auto *curr_root = tree_list; curr_root != nullptr;
       curr_root = next_root) {
    next_root = curr_root->right_;

    auto curr_root_dim = curr_root->dimension();
    if (curr_root_dim < dim) {
      prev_root = curr_root;
    } else if (curr_root_dim == dim) {
      // Detach the curr_root from the list.
      if (prev_root != nullptr) {
        prev_root->right_ = next_root;
      } else {
        tree_list = next_root;
      }
      curr_root->right_ = nullptr;

      // Merging yields a carry tree of a higher dimension.
      // Continue to merge the tree.
      tree = BinomialHeapNode<T>::MergeTrees(curr_root, tree);
      dim++;
    } else {
      // Insert tree into this list.
      tree->right_ = curr_root;
      break;
    }
  }

  if (prev_root != nullptr) {
    prev_root->right_ = tree;
    return tree_list;
  } else {
    return tree;
  }
}

template <typename T>
BinomialHeapNode<T> *
BinomialHeapNode<T>::MergeTreeLists(BinomialHeapNode<T> *a,
                                    BinomialHeapNode<T> *b) {
  BinomialHeapNode<T> *node_a = a;
  BinomialHeapNode<T> *node_b = b;

  BinomialHeapNode<T> *result;
  BinomialHeapNode<T> **merge_tail = &result;

  while (true) {
    if (node_a == nullptr) {
      *merge_tail = node_b;
      break;
    }
    if (node_b == nullptr) {
      *merge_tail = node_a;
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
      BinomialHeapNode<T> *carry =
          BinomialHeapNode<T>::MergeTrees(node_a, node_b);
      if (next_node_a == nullptr) {
        node_a = carry;
        node_b = next_node_b;
      } else if (node_b == nullptr) {
        node_a = next_node_a;
        node_b = carry;
      } else {
        node_a = AddToTreeList(carry, next_node_a);
        node_b = next_node_b;
      }
      continue;
    }

    // Append the lower dimension node to the merged list.
    if (node_a->dimension() < node_b->dimension()) {
      *merge_tail = node_a;
      merge_tail = &node_a->right_;
      node_a = node_a->right_;
    } else {
      *merge_tail = node_b;
      merge_tail = &node_b->right_;
      node_b = node_b->right_;
    }
  }

  return result;
}

template <typename T>
BinomialHeapNode<T> *BinomialHeapNode<T>::DetachChildren() {
  BinomialHeapNode<T> *prev_child = nullptr;
  BinomialHeapNode<T> *child = child_;
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
void BinomialHeapNode<T>::PrintTree(std::ostream &out,
                                    const std::string &label) const {
  out << label << ":" << std::endl;
  PrintTree(out, 0);
  out << std::endl;
}

template <typename T> std::string BinomialHeapNode<T>::DebugString() const {
  std::stringstream out;
  out << key_ << " [id:" << id_ << "][dim:" << dimension_ << "]";

  if (parent_ != nullptr) {
    out << "[parent:" << parent_->key_ << "]";
  } else {
    out << "[parent:null]";
  }

  if (right_ != nullptr) {
    out << "[right=" << right_->key_ << "]";
  }
  if (child_ != nullptr) {
    out << "[child=" << child_->key_ << "]";
  }

  return out.str();
}

template <typename T>
void BinomialHeapNode<T>::PrintTree(std::ostream &out, int level) const {
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
void BinomialHeapNode<T>::Validate(std::unordered_set<int> *seen_ids) const {
  if (seen_ids != nullptr) {
    CHECK(seen_ids->insert(id_).second);
  }

  if (dimension_ > 0) {
    CHECK(child_->parent_ == this);
    CHECK(child_->dimension_ == dimension_ - 1);
    child_->Validate(seen_ids);

    if (!is_root()) {
      CHECK(right_->parent_ == parent_);
      CHECK(right_->dimension_ == dimension_ - 1);
      right_->Validate(seen_ids);
    }
  } else {
    CHECK(child_ == nullptr);
    if (!is_root()) {
      CHECK(right_ == nullptr);
    }
  }
}

template <typename T> class BinomialHeap : public Heap<T> {
public:
  BinomialHeap() : root_(nullptr) {}
  ~BinomialHeap() { BinomialHeapNode<T>::DeleteTree(root_); }

  static Factory<Heap<T>> factory() {
    return Factory<Heap<T>>("Binomial Heap",
                            []() { return new BinomialHeap<T>{}; });
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
  void SiftUp_(BinomialHeapNode<T> *node);

  // Returns the minimum element, and sets the previous sibling of the min
  // element.
  BinomialHeapNode<T> *Min_(BinomialHeapNode<T> **prev_node) const;

  // Linked list of root nodes starting from lowest dimension.
  BinomialHeapNode<T> *root_;

  // Map of each id to the node.
  std::unordered_map<int, BinomialHeapNode<T> *> id_to_node_;
};

template <typename T> void BinomialHeap<T>::Add(T key, int id) {
  BinomialHeapNode<T> *node = new BinomialHeapNode<T>{key, id};
  CHECK(id_to_node_.emplace(id, node).second);

  // If no root. Make this the root.
  if (root_ == nullptr) {
    root_ = node;
  } else {
    root_ = BinomialHeapNode<T>::AddToTreeList(node, root_);
  }
}

template <typename T> void BinomialHeap<T>::ReduceKey(T new_key, int id) {
  auto *node = id_to_node_[id];
  node->set_key(new_key);
  SiftUp_(node);
}

template <typename T> const T *BinomialHeap<T>::LookUp(int id) const {
  const auto it = id_to_node_.find(id);
  if (it == id_to_node_.end()) {
    return nullptr;
  }
  return &it->second->key();
}

template <typename T> HeapElement<T> BinomialHeap<T>::Min() const {
  BinomialHeapNode<T> *unused_node;
  const auto *min_node = Min_(&unused_node);
  return std::make_pair(min_node->key(), min_node->id());
}

template <typename T>
BinomialHeapNode<T> *
BinomialHeap<T>::Min_(BinomialHeapNode<T> **prev_node) const {
  DCHECK(size() > 0);

  BinomialHeapNode<T> *prev = root_;
  BinomialHeapNode<T> *min_root = root_;
  BinomialHeapNode<T> *min_root_prev = nullptr;
  for (auto *root = root_->right(); root != nullptr;
       prev = root, root = root->right()) {
    if (root->key() < min_root->key()) {
      min_root = root;
      min_root_prev = prev;
    }
  }
  *prev_node = min_root_prev;
  return min_root;
}

template <typename T> HeapElement<T> BinomialHeap<T>::PopMinimum() {
  BinomialHeapNode<T> *prev_node;
  BinomialHeapNode<T> *min_root = Min_(&prev_node);

  if (prev_node != nullptr) {
    prev_node->set_right(min_root->right());
  } else {
    root_ = min_root->right();
  }

  auto *children = min_root->DetachChildren();
  root_ = BinomialHeapNode<T>::MergeTreeLists(root_, children);

  auto result = std::make_pair(min_root->key(), min_root->id());
  id_to_node_.erase(min_root->id());
  delete min_root;
  return result;
}

template <typename T> void BinomialHeap<T>::SiftUp_(BinomialHeapNode<T> *node) {
  T key = node->key();
  int id = node->id();
  while (true) {
    auto *parent = node->parent();

    // Done if parent is root or has smaller key.
    if (parent == nullptr || !(key < parent->key())) {
      break;
    }

    // Move the parent down.
    node->set_key(parent->key());
    node->set_id(parent->id());
    id_to_node_[parent->id()] = node;

    node = parent;
  }

  // Finally place element at pos.
  node->set_key(key);
  node->set_id(id);
  id_to_node_[id] = node;
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
  std::unordered_set<int> seen_ids;
  for (auto *root = root_; root != nullptr; root = root->right()) {
    CHECK(root->is_root());
    CHECK(root->dimension() > prev_dimension);
    root->Validate(&seen_ids);
    prev_dimension = root->dimension();
  }

  if (seen_ids.size() != size()) {
    for (const auto &entry : id_to_node_) {
      if (seen_ids.find(entry.first) == seen_ids.end()) {
        LOG(ERROR) << "Id not seen: " << entry.first << std::endl;
      }
    }
    LOG(FATAL) << "Some ids are missing";
  }
}

#endif /* HEAPS_BINOMIAL_HEAP_H_ */
