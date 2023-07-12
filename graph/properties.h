#ifndef GRAPH_PROPERTIES_H_
#define GRAPH_PROPERTIES_H_

#include "graph/graph.h"

// A set of T values keyed by an int key.
// Can be used for weights for nodes or edges.
template <typename T> class Properties {
public:
  explicit Properties(T default_value) : default_value_(default_value) {}
  Properties(Properties &&other)
      : properties_(std::move(other.properties_)),
        default_value_(other.default_value_) {}

  // Sets a value at a given index.
  void Set(int index, T value) {
    if (index >= properties_.size()) {
      properties_.resize(index + 1, default_value_);
    }
    properties_[index] = value;
  }

  // Gets a value at a given index.
  T Get(int index) const {
    if (index < properties_.size()) {
      return properties_[index];
    }
    return default_value_;
  }

private:
  std::vector<T> properties_;
  T default_value_;
};

#endif /* GRAPH_PROPERTIES_H_ */
