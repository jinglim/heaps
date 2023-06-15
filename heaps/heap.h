// Base class for a Heap data structure.

#ifndef HEAPS_HEAP_H_
#define HEAPS_HEAP_H_

#include <utility>

// An element in a Heap. Each elements consists of a T value and an int key.
template <typename T> using HeapElement = std::pair<T, int>;

// Base class for a Heap data structure.
// Implementations should implement these virtual methods
template <typename T> class Heap {
public:
  virtual ~Heap() {}

  // Returns number of elements.
  virtual int size() const = 0;

  // Returns true if it's empty.
  bool empty() const;

  // Adds a value with its associated key.
  virtual void Add(T value, int key) = 0;

  // Updates an element with a lower value.
  virtual void ReduceValue(T new_value, int key) = 0;

  // Looks up a value by key. Returns nullptr if not found.
  virtual const T *LookUp(int key) const = 0;

  // Returns the min element.
  virtual HeapElement<T> Min() const = 0;

  // Pops and returns the minimum value.
  virtual HeapElement<T> PopMinimum() = 0;

  // Print for debugging.
  virtual void PrintTree(std::ostream &out, const std::string &label) const = 0;

  // Validate the invariants.
  virtual void Validate() const = 0;
};

template <typename T> bool Heap<T>::empty() const { return size() == 0; }

#endif /* HEAPS_HEAP_H_ */
