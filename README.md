# Heaps, Priority Queues, Shortest Path
This project is an exploration of Heap data structures and their use in shortest path computation.

These are implemented in C++, built using Bazel.

Pairing Heap has the best performance if the key values need to be decreased (e.g. for Dijkstra's Shortest Path algorithm). If you need only push and pop operations, just use a Binary Heap.

Dependencies:
* abseil-cpp library.

## Binary Heap
Reference: [https://en.wikipedia.org/wiki/Binary_heap].

This is a typical implementation, storing all elements in a vector. A hash map is used to map the element key (int value) to its index position in the vector.

## Binomial Heap
Reference: [https://en.wikipedia.org/wiki/Binomial_heap].

This is implemented using a list of root trees of BinomialHeapNodes, where each node points to its parent node, its right sibling node and its child node.

## Weak Heap
Reference: [https://en.wikipedia.org/wiki/Weak_heap].

This is implemented using a vector of elements, stored in "right-child left-sibling" format. Another vector of 0s or 1s indicate whether the children are swapped.

## Pairing Heap
Reference: [https://en.wikipedia.org/wiki/Pairing_heap].

This is implemented using a tree of PairingHeapNodes, where each node points to its left (previous sibling or parent) node, its right sibling node and its child node.

## 2-3 Heap
Reference: [https://en.wikipedia.org/wiki/2%E2%80%933_heap].

The implementation of this heap is quite challenging. Can probably be simplified and optimized.

## Fibonacci Heap
Reference: [https://en.wikipedia.org/wiki/Fibonacci_heap].

This implementation is based on the description in Wikipedia.

## Thin Heap
Reference: [https://www.cs.princeton.edu/courses/archive/spr04/cos423/handouts/thin%20heap.pdf]

Thin Heap is an optimized version of Fibonacci Heap.

## Graph
This is a relatively simple immutable Graph class. Use GraphBuilder to build a Graph object.

## Shortest Path
Shortest path implementations against a weighted directed graph.
Two implementations are provided:
* BfsShortestPath - a naive BFS traversal implementation.
* DijkstraShortestPath - a typical Dijkstra's algorithm.

## Feedback
Send comments and feedbacks to jinglim@gmail.com.
[https://www.linkedin.com/in/jing-yee-lim/]

Thanks!
