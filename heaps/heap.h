// Base class for a Heap data structure.

#ifndef HEAPS_HEAP_H_
#define HEAPS_HEAP_H_

#include <utility>

// An element in a Heap. Each elements consists of a T key and an unique int
// identifier.
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

  // Adds an element with the given key and unique id.
  virtual void Add(T key, int id) = 0;

  // Updates an element with a lower key.
  virtual void ReduceKey(T new_key, int id) = 0;

  // Looks up a key by id. Returns nullptr if not found.
  virtual const T *LookUp(int id) const = 0;

  // Returns the min element.
  virtual HeapElement<T> Min() const = 0;

  // Pops and returns the minimum key.
  virtual HeapElement<T> PopMinimum() = 0;

  // Print for debugging.
  virtual void PrintTree(std::ostream &out, const std::string &label) const = 0;

  // Validate the invariants.
  virtual void Validate() const = 0;
};

template <typename T> bool Heap<T>::empty() const { return size() == 0; }

#endif /* HEAPS_HEAP_H_ */
