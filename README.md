# Heaps
This project is an exploration of Heap data structures and their use in shortest path computation.

These are implemented in C++, built using Bazel.

Dependencies:
* abseil-cpp library.

## Binary Heap
Reference: [https://en.wikipedia.org/wiki/Binary_heap].

This is a typical implementation, storing all elements in a vector. A hash map is used to map the element key (int value) to its index position in the vector.

## Binomial Heap
Reference: [https://en.wikipedia.org/wiki/Binomial_heap].

This is implemented using a list of root trees of BinomialNodes, where each node points to its parent node, its right sibling node and its child node.

## Graph
This is a relatively simple non-immutable Graph class. Use GraphBuilder to build a Graph object.

## Shortest Path
Shortest path implementations against a weighted directed graph.
Two implementations are provided:
* BfsShortestPath - a naive BFS traversal implementation.
* DijkstraShortestPath - a typical Dijkstra's algorithm.

## Next
* Weak Heap
* 2-3 Heap
* Performance tests

## Feedback
Send comments and feedbacks to jinglim@gmail.com.
Thanks!
