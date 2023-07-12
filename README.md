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

This is implemented using a list of root trees of BinomialHeapNodes, where each node points to its parent node, its right sibling node and its child node.

## Weak Heap
Reference: [https://en.wikipedia.org/wiki/Weak_heap].

This is implemented using a vector of elements, stored in "right-child left-sibling" format. Another vector of 0s or 1s indicate whether the children are swapped.

## Pairing Heap
Reference: [https://en.wikipedia.org/wiki/Pairing_heap].

This is implemented using a tree of PairingHeapNodes, where each node points to its left (previous sibling or parent) node, its right sibling node and its child node.

## Graph
This is a relatively simple immutable Graph class. Use GraphBuilder to build a Graph object.

## Shortest Path
Shortest path implementations against a weighted directed graph.
Two implementations are provided:
* BfsShortestPath - a naive BFS traversal implementation.
* DijkstraShortestPath - a typical Dijkstra's algorithm.

## Next
* 2-3 Heap
* Performance tests

## Feedback
Send comments and feedbacks to jinglim@gmail.com.
[https://www.linkedin.com/in/jing-yee-lim/]

Thanks!
