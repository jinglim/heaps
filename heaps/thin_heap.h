// Thin Heap.
//
// See
// https://www.cs.princeton.edu/courses/archive/spr04/cos423/handouts/thin%20heap.pdf

#ifndef HEAPS_THIN_HEAP_H_
#define HEAPS_THIN_HEAP_H_

#include <iostream>
#include <sstream>
#include <unordered_set>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "base/factory.h"
#include "heaps/heap.h"

// A node used in Thin Heaps.
template <typename T> class ThinHeapNode {
public:
  ThinHeapNode(T key, int id)
      : key_(key), id_(id), rank_(0), child_(nullptr), left_(nullptr),
        right_(nullptr) {}

  const T &key() const { return key_; }
  void set_key(T key) { key_ = key; }

  int id() const { return id_; }

  short rank() const { return rank_; }
  void set_rank(int rank) { rank_ = rank; }

  // A node is thick if its highest ranked child has a rank that is 1 lower
  // than this node.
  bool is_thick() const {
    if (child_ != nullptr) {
      return child_->rank_ + 1 == rank_;
    } else {
      return rank_ == 0;
    }
  }

  // Returns true if this is one of the root nodes.
  bool is_root() const { return left_ == nullptr; }

  // Returns the previous sibling or the parent if this is the first child.
  ThinHeapNode<T> *left() const { return left_; }
  void clear_left() { left_ = nullptr; }

  // Returns the next sibling.
  ThinHeapNode<T> *right() const { return right_; }
  void set_right(ThinHeapNode<T> *right) { right_ = right; }
  void clear_right() { right_ = nullptr; }

  // Returns the highest rank child (with dimension = dimension() - 1 or
  // dimension() - 2)
  ThinHeapNode<T> *child() const { return child_; }

  // Delete the entire tree rooted at this node.
  static void DeleteTree(ThinHeapNode<T> *node) {
    ThinHeapNode<T> *next_node;
    for (; node != nullptr; node = next_node) {
      next_node = node->right_;
      DeleteTree(node->child_);
      delete node;
    }
  }

  // Make this node thick by dropping its rank if necessary.
  void MakeThick() {
    if (child_ != nullptr) {
      rank_ = child_->rank_ + 1;
    } else {
      rank_ = 0;
    }
  }

  // Add a (highest ranked) child and increase the rank of this node.
  void AddChild(ThinHeapNode<T> *child) {
    if (child_ != nullptr) {
      child_->left_ = child;
    }
    child->left_ = this;
    child->right_ = child_;
    child_ = child;
    rank_++;
  }

  // Insert a node before this node.
  void InsertBefore(ThinHeapNode<T> *node) {
    node->right_ = this;
    node->left_ = nullptr;
    left_ = node;
  }

  // Insert a node after this node.
  void InsertAfter(ThinHeapNode<T> *node) {
    node->left_ = this;
    node->right_ = right_;
    if (right_ != nullptr) {
      right_->left_ = node;
    }
    right_ = node;
  }

  // Detach first child without lowering rank of this node.
  ThinHeapNode<T> *DetachFirstChild() {
    DCHECK(is_thick());
    auto *child = child_;
    if (child->right_ != nullptr) {
      child->right_->left_ = this;
    }
    child_ = child->right_;
    child->left_ = nullptr;
    child->right_ = nullptr;
    return child;
  }

  // Cut this node from its siblings and parent, if necessary.
  void Cut() {
    if (left_->child_ == this) {
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

  // Debug information about this node.
  std::string DebugString() const;

  // Print out node information recursively.
  void PrintTree(std::ostream &out, int level) const;

  // Validate the fields.
  void Validate(std::unordered_set<int> *seen_ids) const;

  // Merge two trees into one.
  static ThinHeapNode<T> *MergeTrees(ThinHeapNode<T> *a, ThinHeapNode<T> *b);

private:
  T key_;

  // An int id that uniquely identifies this node.
  int id_;

  // Rank of the tree at this node.
  short rank_;

  // Points to the highest dimension child.
  ThinHeapNode<T> *child_;

  // Points to the left sibling or parent node (if this is the first child).
  ThinHeapNode<T> *left_;

  // Points to next sibling.
  ThinHeapNode<T> *right_;
};

template <typename T>
ThinHeapNode<T> *ThinHeapNode<T>::MergeTrees(ThinHeapNode<T> *a,
                                             ThinHeapNode<T> *b) {
  if (a->key_ < b->key_) {
    a->AddChild(b);
    return a;
  } else {
    b->AddChild(a);
    return b;
  }
}

template <typename T> std::string ThinHeapNode<T>::DebugString() const {
  std::stringstream out;
  out << key_ << " [id:" << id_ << "][rank:" << rank_ << "]";

  if (left_ != nullptr) {
    out << "[left:" << left_->key_ << "]";
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
void ThinHeapNode<T>::PrintTree(std::ostream &out, int level) const {
  for (int i = 0; i < level; ++i) {
    out << "| ";
  }
  out << DebugString() << std::endl;
  for (const auto *child = child_; child != nullptr; child = child->right()) {
    child->PrintTree(out, level + 1);
  }
}

template <typename T>
void ThinHeapNode<T>::Validate(std::unordered_set<int> *seen_ids) const {
  if (seen_ids != nullptr) {
    CHECK(seen_ids->insert(id_).second);
  }

  if (child_ != nullptr) {
    CHECK(child_->left_ == this);

    int child_rank = child_->rank_;
    CHECK(child_rank == rank_ - 1 || child_rank == rank_ - 2);
    for (const auto *child = child_; child != nullptr;
         child = child->right(), child_rank--) {
      CHECK(!child->is_root());
      CHECK(child->rank_ == child_rank);
      CHECK(!(child->key_ < key_));
      child->Validate(seen_ids);
      if (child->right_ != nullptr) {
        CHECK(child->right_->left_ == child);
      }
    }
  } else {
    CHECK(rank_ <= 1);
  }
}

template <typename T> class ThinHeap : public Heap<T> {
public:
  ThinHeap() : min_root_(nullptr), root_(nullptr) {
    roots_by_rank_.resize(1, nullptr);
  }
  ~ThinHeap() { ThinHeapNode<T>::DeleteTree(root_); }

  static Factory<Heap<T>> factory() {
    return Factory<Heap<T>>("Thin Heap", []() { return new ThinHeap<T>{}; });
  }

  // Returns number of elements.
  virtual int size() const override {
    return static_cast<int>(id_to_node_.size());
  }

  // Adds an element with given key and unique int key.
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
  // Merge a tree into `roots_by_rank_`, combining with other trees of the
  // same rank if necessary.
  void MergeRoot_(ThinHeapNode<T> *root);

  // Cut a tree and move it to the list of nodes. Adjust the tree's nearby
  // nodes to keep the heap invariants.
  void CutAndMoveToRoot_(ThinHeapNode<T> *tree);

  // Fix the rank after `tree` is cut.
  void LowerRank_(ThinHeapNode<T> *tree);

  // The minimum root. This points to one of the node in the `root_` linked
  // list.
  ThinHeapNode<T> *min_root_;

  // Linked list of root nodes.
  ThinHeapNode<T> *root_;

  // Roots indexed by their rank.
  std::vector<ThinHeapNode<T> *> roots_by_rank_;

  // Map of each id to the node.
  std::unordered_map<int, ThinHeapNode<T> *> id_to_node_;
};

template <typename T> void ThinHeap<T>::Add(T key, int id) {
  auto *node = new ThinHeapNode<T>{key, id};
  CHECK(id_to_node_.emplace(id, node).second);

  if (min_root_ == nullptr || key < min_root_->key()) {
    min_root_ = node;
  }
  node->set_right(root_);
  root_ = node;
}

template <typename T> void ThinHeap<T>::ReduceKey(T new_key, int id) {
  auto *node = id_to_node_[id];
  node->set_key(new_key);

  if (new_key < min_root_->key()) {
    min_root_ = node;
  }

  if (!node->is_root()) {
    CutAndMoveToRoot_(node);
  }
}

template <typename T>
void ThinHeap<T>::CutAndMoveToRoot_(ThinHeapNode<T> *tree) {
  DCHECK(!tree->is_root());

  // Lower the ranks of the left siblings first.
  LowerRank_(tree);

  // Cut and move the tree to the root list.
  tree->Cut();
  tree->MakeThick();
  tree->set_right(root_);
  root_ = tree;
}

template <typename T> void ThinHeap<T>::LowerRank_(ThinHeapNode<T> *tree) {
  int rank = tree->rank();
  auto *left = tree->left();

  // Iterate through the left siblings until it gets to the parent.
  while (left->child() != tree) {
    // Case (1): `left` is a sibling of `tree` and is thick.
    // Make the first child the right sibling.
    if (left->is_thick()) {
      auto *left_child = left->DetachFirstChild();
      left->InsertAfter(left_child);
      return;
    }

    // Case (2): `left` is thin. Drop its rank, making it thick.
    // Continue with the next left sibling.
    left->set_rank(rank);
    tree = left;
    left = left->left();
    rank++;
  }

  // If this is a root, we just need to update its rank.
  if (left->is_root()) {
    left->set_rank(rank);
    return;
  }

  // If parent `left` was thick before, it is now thin; don't need to
  // update anything.
  if (left->rank() == rank + 1) {
    return;
  }

  // Cut it, and drop the rank.
  CutAndMoveToRoot_(left);
  left->set_rank(rank);
}

template <typename T> const T *ThinHeap<T>::LookUp(int id) const {
  const auto it = id_to_node_.find(id);
  if (it == id_to_node_.end()) {
    return nullptr;
  }
  return &it->second->key();
}

template <typename T> HeapElement<T> ThinHeap<T>::Min() const {
  DCHECK(size() > 0);
  return std::make_pair(min_root_->key(), min_root_->id());
}

template <typename T> HeapElement<T> ThinHeap<T>::PopMinimum() {
  DCHECK(size() > 0);

  // Merge roots into `roots_by_rank_`.
  ThinHeapNode<T> *next_tree;
  for (auto *tree = root_; tree != nullptr; tree = next_tree) {
    next_tree = tree->right();
    tree->clear_right();
    if (tree != min_root_) {
      MergeRoot_(tree);
    }
  }

  // Sort the children of the root by rank.
  for (auto *tree = min_root_->child(); tree != nullptr; tree = next_tree) {
    next_tree = tree->right();
    tree->clear_left();
    tree->clear_right();
    tree->MakeThick();
    MergeRoot_(tree);
  }

  id_to_node_.erase(min_root_->id());
  auto result = std::make_pair(min_root_->key(), min_root_->id());
  delete min_root_;

  // Link up the roots, with min root being the first root.
  min_root_ = nullptr;
  root_ = nullptr;
  for (int i = 0; i < roots_by_rank_.size(); i++) {
    auto *tree = roots_by_rank_[i];
    if (tree != nullptr) {
      roots_by_rank_[i] = nullptr;

      if (min_root_ == nullptr || tree->key() < min_root_->key()) {
        min_root_ = tree;
      }
      tree->set_right(root_);
      root_ = tree;
    }
  }

  return result;
}

template <typename T> void ThinHeap<T>::MergeRoot_(ThinHeapNode<T> *root) {
  auto rank = root->rank();
  while (true) {
    auto *root2 = roots_by_rank_[rank];
    if (root2 == nullptr) {
      roots_by_rank_[rank] = root;
      break;
    }
    roots_by_rank_[rank] = nullptr;

    // The merged root has a higher rank.
    root = ThinHeapNode<T>::MergeTrees(root, root2);
    rank++;
    if (rank >= roots_by_rank_.size()) {
      roots_by_rank_.resize(rank + 1, nullptr);
    }
  }
}

template <typename T>
void ThinHeap<T>::PrintTree(std::ostream &out, const std::string &label) const {
  out << std::endl
      << "-- Heap (" << size() << ") " << label << " --" << std::endl;
  for (const auto *root = root_; root != nullptr; root = root->right()) {
    if (root == min_root_) {
      out << "Min ";
    }
    out << "Tree #" << root->rank() << std::endl;
    root->PrintTree(out, 1);
  }
  out << std::endl;
}

template <typename T> void ThinHeap<T>::Validate() const {
  std::unordered_set<int> seen_ids;
  for (const auto *root = root_; root != nullptr; root = root->right()) {
    CHECK(root->is_root());
    CHECK(!(root->key() < min_root_->key()));
    CHECK(root->rank() >= 0);
    root->Validate(&seen_ids);
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

#endif /* HEAPS_THIN_HEAP_H_ */
