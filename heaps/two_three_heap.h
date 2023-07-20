#ifndef HEAPS_TWO_THREE_HEAP_H_
#define HEAPS_TWO_THREE_HEAP_H_

#include <sstream>
#include <unordered_set>

#include "absl/log/check.h"
#include "base/factory.h"
#include "heaps/heap.h"

// Node class used in 2-3 Heaps.
//
// Each node has a dimension. If it has dimension N, it has children trunks
// that have dimensions 0 .. N-1. The trunks are linked together in a cyclic
// linked list using left_ and right_ fields.
//
// A trunk has one or two nodes, one is primary, and the other secondary.
// The partner_ field links the two nodes to each other.
template <typename T> class TwoThreeNode {
public:
  TwoThreeNode(T key, int id)
      : key_(key), id_(id), dimension_(0), is_secondary_(false),
        partner_(nullptr), parent_(nullptr), child_(nullptr), left_(this),
        right_(this) {}

  // For the root only.
  TwoThreeNode()
      : id_(-1), dimension_(0), is_secondary_(false), partner_(nullptr),
        parent_(nullptr), child_(nullptr), left_(this), right_(this) {}

  ~TwoThreeNode() {
    if (partner_ != nullptr) {
      partner_->partner_ = nullptr;
      delete partner_;
    }
    if (child_ != nullptr) {
      auto *child = child_;
      do {
        auto *next = child->right_;
        delete child;
        child = next;
      } while (child != child_);
    }
  }

  const T &key() const { return key_; }
  void set_key(T key) { key_ = key; }

  int id() const { return id_; }
  short dimension() const { return dimension_; }

  bool is_root() const { return parent_->parent_ == nullptr; }

  bool has_siblings() const { return right_ != this; }

  TwoThreeNode<T> *partner() const { return partner_; }
  bool is_secondary() const { return is_secondary_; }

  TwoThreeNode<T> *parent() const { return parent_; }
  void set_parent(TwoThreeNode<T> *parent) { parent_ = parent; }
  void clear_parent() { parent_ = nullptr; }

  TwoThreeNode<T> *child() const { return child_; }
  void set_child(TwoThreeNode<T> *child) { child_ = child; }
  void clear_child() { child_ = nullptr; }

  // Sibling nodes that have the same parent, organized in a cyclic linked list.
  TwoThreeNode<T> *left() const { return left_; }
  TwoThreeNode<T> *right() const { return right_; }

  // Attach a secondary partner to this primary node.
  void AttachPartner(TwoThreeNode<T> *partner);

  // Detach this secondary node from the primary node, so it can be inserted
  // somewhere else.
  void DetachFromTrunk();

  // Add a new child tree (of same dimension).
  void AddChild(TwoThreeNode<T> *new_child);

  // Detach this node from the parent.
  void DetachFromParent();

  // Replace a child of this node with another.
  void ReplaceChild(TwoThreeNode<T> *old_child, TwoThreeNode<T> *new_child);

  // Swap this node with its partner. Update the links.
  void SwapPartner();

  // Move the partner node to become the child node.
  void SwitchPartnerToChild();

  // Move the child node to become the partner node.
  void SwitchChildToPartner();

  // Merges two trees of dimendion D, return a pair of
  // <merged_tree, carry_tree>, where merged_tree has dimension D and
  // carry_tree has dimension D + 1.
  static std::pair<TwoThreeNode<T> *, TwoThreeNode<T> *>
  MergeTrees(TwoThreeNode<T> *a, TwoThreeNode<T> *b);

  // Debug information about this node.
  std::string DebugString() const;

  // Print the subtree under this node.
  void PrintTree(std::ostream &out, const std::string &label) const;

  // Print out node information recursively.
  void PrintTree(std::ostream &out, int level) const;

  // Validate the tree structure under this node.
  void Validate(std::unordered_set<int> *seen_ids = nullptr) const;

private:
  T key_;

  // An id that uniquely identifies this node.
  int id_;

  // Dimension of the tree at this node.
  short dimension_;

  // If true, then [partner_, this] are the nodes of this trunk.
  // Otherwise [this, partner_].
  bool is_secondary_;

  // This points to other node in trunk; the other points back to this.
  // This may be nullptr.
  TwoThreeNode<T> *partner_;

  // Points to the parent node.
  // This should not be a nullptr unless it's a sentinel node.
  TwoThreeNode<T> *parent_;

  // Points to the highest dimension child.
  // If this is a secondary node, this should be nullptr.
  TwoThreeNode<T> *child_;

  // Points to other second nodes, in a cyclic linked list.
  // If this is a secondary node, this should be nullptr.
  TwoThreeNode<T> *left_;
  TwoThreeNode<T> *right_;
};

template <typename T>
void TwoThreeNode<T>::AttachPartner(TwoThreeNode<T> *partner) {
  DCHECK(!is_secondary_);

  partner->partner_ = this;
  partner_ = partner;
  partner_->parent_ = parent_;
  partner_->is_secondary_ = true;
}

template <typename T> void TwoThreeNode<T>::DetachFromTrunk() {
  DCHECK(is_secondary_);
  is_secondary_ = false;
  partner_->partner_ = nullptr;
  partner_ = nullptr;
  parent_ = nullptr;
}

template <typename T>
void TwoThreeNode<T>::AddChild(TwoThreeNode<T> *new_child) {
  DCHECK(!new_child->is_secondary_);
  DCHECK(!(new_child->key_ < key_));
  DCHECK(new_child->dimension_ == dimension_);

  dimension_++;

  new_child->parent_ = this;
  auto *new_child_partner = new_child->partner_;
  if (new_child_partner != nullptr) {
    new_child_partner->parent_ = this;
  }

  // Add to siblings cyclic linked list.
  if (child_ != nullptr) {
    new_child->right_ = child_;
    new_child->left_ = child_->left_;
    child_->left_->right_ = new_child;
    child_->left_ = new_child;
  }
  child_ = new_child;
}

template <typename T> void TwoThreeNode<T>::DetachFromParent() {
  DCHECK(!is_secondary_);

  if (!has_siblings()) {
    parent_->child_ = nullptr;
  } else {
    parent_->child_ = right_;

    // Remove links to siblings.
    left_->right_ = right_;
    right_->left_ = left_;
    left_ = this;
    right_ = this;
  }

  parent_->dimension_--;
  parent_ = nullptr;
  if (partner_ != nullptr) {
    partner_->parent_ = nullptr;
  }
}

template <typename T>
void TwoThreeNode<T>::ReplaceChild(TwoThreeNode<T> *old_child,
                                   TwoThreeNode<T> *new_child) {
  if (old_child->has_siblings()) {
    new_child->left_ = old_child->left_;
    new_child->right_ = old_child->right_;

    old_child->left_->right_ = new_child;
    old_child->right_->left_ = new_child;

    old_child->left_ = old_child;
    old_child->right_ = old_child;
  }

  new_child->parent_ = this;
  auto *new_child_partner = new_child->partner_;
  if (new_child_partner != nullptr) {
    new_child_partner->parent_ = this;
  }

  old_child->parent_ = nullptr;
  auto *old_child_partner = old_child->partner_;
  if (old_child_partner != nullptr) {
    old_child_partner->parent_ = nullptr;
  }

  if (child_ == old_child) {
    child_ = new_child;
  }
}

template <typename T> void TwoThreeNode<T>::SwapPartner() {
  DCHECK(!is_secondary_);

  // Update the sibling list so that it links to this node rather than the
  // partner.
  if (has_siblings()) {
    partner_->left_ = left_;
    partner_->right_ = right_;
    left_->right_ = partner_;
    right_->left_ = partner_;

    left_ = this;
    right_ = this;
  }

  if (parent_->child_ == this) {
    parent_->child_ = partner_;
  }

  partner_->is_secondary_ = false;
  is_secondary_ = true;
}

template <typename T> void TwoThreeNode<T>::SwitchPartnerToChild() {
  auto *partner = partner_;
  partner->DetachFromTrunk();
  AddChild(partner);
}

template <typename T> void TwoThreeNode<T>::SwitchChildToPartner() {
  DCHECK(partner_ == nullptr);
  auto *child = child_;
  child->DetachFromParent();
  AttachPartner(child);
}

template <typename T>
std::pair<TwoThreeNode<T> *, TwoThreeNode<T> *>
TwoThreeNode<T>::MergeTrees(TwoThreeNode<T> *a, TwoThreeNode<T> *b) {
  DCHECK(!a->has_siblings());
  DCHECK(!b->has_siblings());
  DCHECK(a->dimension_ == b->dimension_);

  // Make a the smaller tree.
  if (b->key_ < a->key_) {
    std::swap(a, b);
  }

  a->parent_ = nullptr;
  auto *a_partner = a->partner_;
  auto *b_partner = b->partner_;

  if (a_partner == nullptr) {
    // Case 1: merge trunks [a, nullptr], [b, nullptr].
    // Return [a, b].
    if (b_partner == nullptr) {
      a->AttachPartner(b);
      return std::make_pair(a, nullptr);
    }

    // Case 2: merge trunks [a, nullptr], [b, b_partner].
    // Return carry tree: a -> [b, b_partner].
    a->AddChild(b);
    return std::make_pair(nullptr, a);
  }

  // Case 3: merge trunks [a, a_partner], [b, nullptr].
  // Return a -> ([a_partner, b] or [b, a_partner])
  if (b_partner == nullptr) {
    a_partner->DetachFromTrunk();

    if (a_partner->key_ < b->key_) {
      a_partner->AttachPartner(b);
      a->AddChild(a_partner);
    } else {
      b->AttachPartner(a_partner);
      a->AddChild(b);
    }
    return std::make_pair(nullptr, a);
  }

  // Case 4: merge trunks [a, a_partner], [b, b_partner].
  // Return a -> [b, b_partner] as carry tree (dimension + 1)
  // and [a_partner] (same dimension).
  a_partner->DetachFromTrunk();
  a->AddChild(b);
  return std::make_pair(a_partner, a);
}

// Debug information about this node.
template <typename T> std::string TwoThreeNode<T>::DebugString() const {
  std::stringstream out;
  out << key_ << " [id:" << id_ << "][dim:" << dimension_ << "]";

  if (parent_ != nullptr) {
    if (is_root()) {
      out << "[root]";
    } else {
      out << "[parent:" << parent_->key_ << "]";
    }
  } else {
    out << "[parent:null]";
  }

  if (is_secondary_) {
    out << "[2nd of " << partner_->key_ << "]";
  } else if (partner_ != nullptr) {
    out << "[partner=" << partner_->key_ << "]";
  }
  if (left_ == this) {
    out << "[left=self]";
  } else {
    out << "[left=" << left_->key_ << "]";
  }
  if (right_ == this) {
    out << "[right=self]";
  } else {
    out << "[right=" << right_->key_ << "]";
  }
  if (child_ != nullptr) {
    out << "[child=" << child_->key_ << "]";
  }

  return out.str();
}

template <typename T>
void TwoThreeNode<T>::PrintTree(std::ostream &out,
                                const std::string &label) const {
  out << label << ":" << std::endl;
  PrintTree(out, 0);
  out << std::endl;
}

// Print out node information recursively.
template <typename T>
void TwoThreeNode<T>::PrintTree(std::ostream &out, int level) const {
  for (int i = 0; i < level; ++i) {
    out << "| ";
  }
  out << DebugString() << std::endl;

  if (child_ != nullptr) {
    child_->PrintTree(out, level + 1);
  }

  if (!is_secondary_ && partner_ != nullptr) {
    partner_->PrintTree(out, level);
  }

  if (parent_ != nullptr && parent_->child_ == this) {
    for (auto *right = right_; right != this; right = right->right_) {
      right->PrintTree(out, level);
    }
  }
}

template <typename T>
void TwoThreeNode<T>::Validate(std::unordered_set<int> *seen_ids) const {
  if (seen_ids != nullptr) {
    CHECK(seen_ids->insert(id_).second);
  }

  if (partner_ == nullptr) {
    CHECK(!is_secondary_);
  }
  if (is_secondary_) {
    CHECK(right_ == this);
    CHECK(left_ == this);
  }

  if (!is_secondary_ && partner_ != nullptr) {
    CHECK(!(partner_->key_ < key_));
    CHECK(partner_->partner_ == this);
    CHECK(partner_->parent_ == parent_);
    CHECK(partner_->dimension_ == dimension_);
    CHECK(partner_->is_secondary_);
    partner_->Validate(seen_ids);
  }

  if (dimension_ > 0) {
    const auto *child = child_;
    int child_dim = dimension_ - 1;
    do {
      CHECK(child != nullptr);
      CHECK(child_dim >= 0);

      CHECK(!(child->key_ < key_));
      CHECK(!child->is_secondary_);
      CHECK(child->dimension() == child_dim);
      CHECK(child->right_->left_ == child);
      CHECK(child->parent_ == this);
      child->Validate(seen_ids);

      child = child->right_;
      child_dim--;
    } while (child != child_);
  }
}

// 2-3 Heap is a heap data structure that allows the min key to be
// computed in O(log n) time, and a key to be decreased in O(1)
// amortized time.
template <typename T> class TwoThreeHeap : public Heap<T> {
public:
  static Factory<Heap<T>> factory() {
    return Factory<Heap<T>>("2-3 Heap", []() { return new TwoThreeHeap<T>(); });
  }

  TwoThreeHeap() : max_root_dim_(-1) {}

  virtual ~TwoThreeHeap() {
    for (auto *sentinel : sentinels_) {
      delete sentinel;
    }
  }

  // Number of nodes in the heap.
  virtual int size() const override {
    return static_cast<int>(id_to_node_.size());
  }

  // Add an element with the given key and unique id.
  virtual void Add(T key, int id) override;

  // Returns the element as a (key, id) pair.
  virtual std::pair<T, int> Min() const override;

  // Remove the min element and return its (key, id).
  // Amortized code: O(log n).
  virtual std::pair<T, int> PopMinimum() override;

  // Decrease the key of a node.
  virtual void ReduceKey(T new_key, int id) override;

  // Looks up a key by id. Returns nullptr if not found.
  virtual const T *LookUp(int id) const override;

  // Print the heap in tree format.
  virtual void PrintTree(std::ostream &out,
                         const std::string &label) const override;

  // Validate the structure of the heap.
  virtual void Validate() const override;

private:
  // Returns the node with the min key.
  TwoThreeNode<T> *Min_() const;

  // Inserts a new tree to the heap.
  void InsertRoot_(TwoThreeNode<T> *tree);

  // Remove a subtree from the heap and make the necessary adjustments to
  // neighboring nodes to keep the structure.
  void RemoveTree_(TwoThreeNode<T> *tree);

  // Make a trunk from the given subtrees.
  TwoThreeNode<T> *MakeTrunk_(TwoThreeNode<T> *a, TwoThreeNode<T> *b);

  // Returns the root node for the given dimension.
  TwoThreeNode<T> *Root_(short dim);

  // Sets the given node as a root for that dimension.
  void SetRoot_(TwoThreeNode<T> *root);

  // Remove the root for the given dimension.
  void ClearRoot_(short dim);

  // Increase the number of sentinel nodes if necessary.
  void ExtendSentinels_(short dim);

  // Sentinel nodes for each dimension.
  // sentinels_[dimension]->child() gives the root of the tree for that
  // dimension.
  std::vector<TwoThreeNode<T> *> sentinels_;

  // Highest dimension of the roots.
  int max_root_dim_;

  // Map of each id to the node.
  std::unordered_map<int, TwoThreeNode<T> *> id_to_node_;
};

template <typename T> void TwoThreeHeap<T>::Add(T key, int id) {
  TwoThreeNode<T> *node = new TwoThreeNode<T>{key, id};
  InsertRoot_(node);
  CHECK(id_to_node_.emplace(id, node).second);
}

template <typename T> std::pair<T, int> TwoThreeHeap<T>::Min() const {
  const auto *min_node = Min_();
  return std::make_pair(min_node->key(), min_node->id());
}

template <typename T> std::pair<T, int> TwoThreeHeap<T>::PopMinimum() {
  DCHECK(size() > 0);

  TwoThreeNode<T> *min_root = Min_();

  // Take care of the partner node.
  TwoThreeNode<T> *partner = min_root->partner();
  if (partner != nullptr) {
    partner->DetachFromTrunk();
    SetRoot_(partner);
  } else {
    ClearRoot_(min_root->dimension());
  }

  // Re-insert the child nodes.
  TwoThreeNode<T> *child;
  while ((child = min_root->child()) != nullptr) {
    child->DetachFromParent();
    InsertRoot_(child);
  }

  auto result = std::make_pair(min_root->key(), min_root->id());
  id_to_node_.erase(min_root->id());
  delete min_root;
  return result;
}

template <typename T> void TwoThreeHeap<T>::ReduceKey(T new_key, int id) {
  auto *node = id_to_node_[id];
  node->set_key(new_key);

  // Check if we need to reparent.
  if (node->is_root() || !(new_key < node->parent()->key())) {

    // If the node is secondary and is now smaller, make it primary.
    auto *partner = node->partner();
    if (node->is_secondary() && new_key < partner->key()) {
      partner->SwapPartner();
    }
    return;
  }

  // Detach the node and re-insert it to the appropriate root node.
  RemoveTree_(node);

  DCHECK(node->parent() == nullptr);
  DCHECK(!node->has_siblings());

  InsertRoot_(node);
}

template <typename T> const T *TwoThreeHeap<T>::LookUp(int id) const {
  const auto it = id_to_node_.find(id);
  if (it == id_to_node_.end()) {
    return nullptr;
  }
  return &it->second->key();
}

template <typename T>
void TwoThreeHeap<T>::PrintTree(std::ostream &out,
                                const std::string &label) const {
  out << std::endl
      << "-- Heap (" << size() << ") " << label << " --" << std::endl;
  for (int i = 0; i <= max_root_dim_; ++i) {
    const auto *sentinel = sentinels_[i];
    const auto *root = sentinel->child();
    if (root != nullptr) {
      out << "Tree #" << root->dimension() << std::endl;
      root->PrintTree(out, 1);
    }
  }
  out << std::endl;
}

template <typename T> void TwoThreeHeap<T>::Validate() const {
  int dimension = 0;
  std::unordered_set<int> seen_ids;
  for (int i = 0; i <= max_root_dim_; ++i) {
    const auto *sentinel = sentinels_[i];
    const auto *root = sentinel->child();
    if (root != nullptr) {
      CHECK(root->parent() == sentinel);
      CHECK(root->dimension() == dimension);
      root->Validate(&seen_ids);
    }
    dimension++;
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

template <typename T> TwoThreeNode<T> *TwoThreeHeap<T>::Min_() const {
  DCHECK(size() > 0);

  // Find the min amongst the tree roots.
  TwoThreeNode<T> *min_node = nullptr;
  for (auto *sentinel : sentinels_) {
    auto *root = sentinel->child();
    if (root == nullptr) {
      continue;
    }
    if (min_node == nullptr || root->key() < min_node->key()) {
      min_node = root;
    }
  }
  return min_node;
}

template <typename T> void TwoThreeHeap<T>::InsertRoot_(TwoThreeNode<T> *tree) {
  DCHECK(tree->parent() == nullptr);
  DCHECK(!tree->has_siblings());

  // If no root. Make this the root.
  short dim = tree->dimension();
  TwoThreeNode<T> *root = Root_(dim);
  if (root == nullptr) {
    SetRoot_(tree);
    return;
  }

  auto result = TwoThreeNode<T>::MergeTrees(root, tree);
  if (result.first != nullptr) {
    SetRoot_(result.first);
  } else {
    ClearRoot_(dim);
  }
  if (result.second != nullptr) {
    InsertRoot_(result.second);
  }
}

template <typename T> void TwoThreeHeap<T>::RemoveTree_(TwoThreeNode<T> *tree) {
  auto *parent = tree->parent();
  int dim = tree->dimension();

  // If it has a partner, just detach from the trunk.
  auto *partner = tree->partner();
  if (partner != nullptr) {
    if (partner->is_secondary()) {
      tree->SwapPartner();
    }
    tree->DetachFromTrunk();
    return;
  }

  if (tree->is_root()) {
    ClearRoot_(tree->dimension());
    tree->DetachFromParent();
    return;
  }

  // Examine trunk of parent's partner.
  auto *pp = parent->partner();
  if (pp != nullptr && pp->dimension() == dim) {

    // Reorder the trunk nodes.
    auto *pp_child = pp->child();
    if (pp_child->partner() != nullptr) {
      tree->DetachFromParent();
      DCHECK(parent->dimension() == dim);

      // Convert pp_child trunk into parent-child relationship.
      pp_child->DetachFromParent();
      pp_child->SwitchPartnerToChild();

      // Convert [parent, pp] trunk into parent-child relationship.
      if (parent->is_secondary()) {
        pp->SwitchPartnerToChild();
        pp->AttachPartner(pp_child);
      } else {
        parent->SwitchPartnerToChild();
        parent->AttachPartner(pp_child);
      }
      return;
    }

    // Make pp primary.
    if (pp->is_secondary()) {
      pp->DetachFromTrunk();
      pp->SwitchChildToPartner();
      parent->ReplaceChild(tree, pp);
    } else {
      tree->DetachFromParent();
      parent->DetachFromTrunk();
      pp_child->AttachPartner(parent);
      if (parent->key() < pp_child->key()) {
        pp_child->SwapPartner();
      }
    }
    return;
  }

  auto *left = tree->left();

  if (left->dimension() == dim + 1) {
    auto *lp = left->partner();

    auto *left_child = left->child();
    auto *left_child_partner = left_child->partner();
    if (left_child_partner != nullptr) {

      // Convert left_child trunk into parent-child relationship.
      left_child->DetachFromParent();
      left_child->SwitchPartnerToChild();

      // Make trunk with [left_child, lp], reattach to parent.
      if (lp != nullptr) {
        lp->DetachFromTrunk();
      }
      auto *trunk = MakeTrunk_(left_child, lp);
      parent->ReplaceChild(left, trunk);

      // Move `left`.
      parent->ReplaceChild(tree, left);

      return;
    }

    if (lp != nullptr) {
      auto *lp_child = lp->child();
      if (lp_child->partner() != nullptr) {

        // Convert left_child trunk into parent-child relationship.
        lp_child->DetachFromParent();
        lp_child->SwitchPartnerToChild();

        // Move [left, lp_child] trunk and move `lp` under `parent`.
        lp->DetachFromTrunk();
        left->AttachPartner(lp_child);
        parent->ReplaceChild(tree, lp);
        return;
      }

      lp->DetachFromTrunk();
      lp->SwitchChildToPartner();

      parent->ReplaceChild(tree, lp);
      return;
    }

    // Create [left, left_child] trunk to replace `tree`.
    RemoveTree_(left);

    left->SwitchChildToPartner();
    parent->ReplaceChild(tree, left);
    return;
  }

  DCHECK(tree->partner() == nullptr);
  RemoveTree_(parent);
  tree->DetachFromParent();
  InsertRoot_(parent);
}

template <typename T>
TwoThreeNode<T> *TwoThreeHeap<T>::MakeTrunk_(TwoThreeNode<T> *a,
                                             TwoThreeNode<T> *b) {
  if (b == nullptr) {
    DCHECK(a->partner() == nullptr);
    return a;
  }
  if (b->key() < a->key()) {
    b->AttachPartner(a);
    return b;
  }
  a->AttachPartner(b);
  return a;
}

template <typename T> TwoThreeNode<T> *TwoThreeHeap<T>::Root_(short dim) {
  ExtendSentinels_(dim);
  return sentinels_[dim]->child();
}

template <typename T> void TwoThreeHeap<T>::SetRoot_(TwoThreeNode<T> *root) {
  short dim = root->dimension();
  ExtendSentinels_(dim);
  auto *sentinel = sentinels_[dim];
  sentinel->set_child(root);

  root->set_parent(sentinel);
  auto *root_partner = root->partner();
  if (root_partner != nullptr) {
    root_partner->set_parent(sentinel);
  }

  if (dim > max_root_dim_) {
    max_root_dim_ = dim;
  }
}

template <typename T> void TwoThreeHeap<T>::ClearRoot_(short dim) {
  sentinels_[dim]->clear_child();
  if (dim == max_root_dim_) {
    do {
      max_root_dim_--;
    } while (max_root_dim_ >= 0 &&
             sentinels_[max_root_dim_]->child() == nullptr);
  }
}

template <typename T> void TwoThreeHeap<T>::ExtendSentinels_(short dim) {
  for (int size = sentinels_.size(); size <= dim; size++) {
    sentinels_.push_back(new TwoThreeNode<T>());
  }
}

#endif /* HEAPS_TWO_THREE_HEAP_H_ */
