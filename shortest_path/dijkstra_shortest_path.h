#ifndef SHORTEST_PATH_DIJKSTRA_SHORTEST_PATH_H_
#define SHORTEST_PATH_DIJKSTRA_SHORTEST_PATH_H_

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "graph/weighted_graph.h"
#include "shortest_path/shortest_path.h"

namespace {
// Print number of Heap operations.
const bool kDebugPrintStats = false;
} // namespace

// Distance from start vertex to this vertex.
// Used by DijkstraShortestPath to keep track of min distance to the vertex.
template <typename T> struct DistanceNode {
  DistanceNode() {}
  DistanceNode(VertexId vertex_id, T distance)
      : vertex_id(vertex_id), distance(distance) {}

  VertexId vertex_id;
  T distance;

  bool operator<(const DistanceNode<T> &other) const {
    return distance < other.distance;
  };
};

template <typename T>
std::ostream &operator<<(std::ostream &out, const DistanceNode<T> &node) {
  out << "[" << node.distance << "]";
  return out;
}

// An implementation of the Dijktra's Shortest Path algorithm.
template <typename T> class DijkstraShortestPath : public ShortestPath<T> {
public:
  DijkstraShortestPath(Factory<Heap<DistanceNode<T>>> heap_factory)
      : heap_factory_(heap_factory) {}

  // Factory to create an instance.
  static Factory<ShortestPath<T>>
  factory(Factory<Heap<DistanceNode<T>>> heap_factory) {
    return Factory<ShortestPath<T>>(
        "Dijkstra's Shortest Path (" + heap_factory.name() + ")",
        [heap_factory]() { return new DijkstraShortestPath<T>{heap_factory}; });
  };

  // Find the shortest paths for all nodes in the graph.
  virtual std::unordered_map<VertexId, Path<T>>
  Run(const WeightedGraph<T> &graph, VertexId start_vertex_index) override;

private:
  Factory<Heap<DistanceNode<T>>> heap_factory_;
};

template <typename T>
std::unordered_map<VertexId, Path<T>>
DijkstraShortestPath<T>::Run(const WeightedGraph<T> &weighted_graph,
                             VertexId start_vertex_id) {
  int num_adds = 0;
  int num_pops = 0;
  int num_reduce_keys = 0;

  // Maps a vertex to previous vertex in its shortest path.
  std::unordered_map<VertexId, VertexId> prev_vertex_map;

  // Set up a heap containing vertices that need to be visited. This is ordered
  // by distance.
  std::unique_ptr<Heap<DistanceNode<T>>> heap(heap_factory_());

  // Initial distance = 0.
  heap->Add(DistanceNode<T>(start_vertex_id, 0), start_vertex_id);
  num_adds++;

  const auto *graph = weighted_graph.graph.get();
  const auto *distances = weighted_graph.edge_weights.get();

  std::unordered_map<VertexId, Path<T>> results;
  while (!heap->empty()) {
    // Pop the node with shortest distance and add to result.
    DistanceNode<T> min_distance_node = heap->PopMinimum().first;
    num_pops++;

    // Skip if the shortest path is already found.
    if (!results
             .insert(std::make_pair(min_distance_node.vertex_id,
                                    Path<T>{min_distance_node.distance}))
             .second) {
      continue;
    }

    const Vertex &from_vertex = graph->GetVertex(min_distance_node.vertex_id);
    for (const Edge &edge : from_vertex.edges()) {
      VertexId to_id = edge.to_vertex_id();

      // If it's already in results, then there's already a shorter path.
      if (results.find(to_id) != results.end()) {
        continue;
      }

      // Create new node and add to heap.
      const auto distance = distances->Get(edge.id());
      T total_distance = min_distance_node.distance + distance;
      CHECK(total_distance >= 0);

      const DistanceNode<T> *to_node = heap->LookUp(to_id);
      if (to_node == nullptr) {
        heap->Add(DistanceNode<T>{to_id, total_distance}, to_id);
        num_adds++;
        prev_vertex_map[to_id] = min_distance_node.vertex_id;
      } else if (total_distance < to_node->distance) {
        // Update the DistanceNode with a shorter distance.
        heap->ReduceKey(DistanceNode<T>{to_id, total_distance}, to_id);
        num_reduce_keys++;
        prev_vertex_map[to_id] = min_distance_node.vertex_id;
      }
    }
  }

  // Construct the shortest path for each node.
  for (auto &result : results) {
    VertexId vertex_id = result.first;
    Path<T> &path = result.second;

    // Trace the path backwards by looking up the prev_vertex_map.
    while (vertex_id != start_vertex_id) {
      path.vertices.push_back(vertex_id);
      vertex_id = prev_vertex_map[vertex_id];
    }
    path.vertices.push_back(start_vertex_id);

    std::reverse(path.vertices.begin(), path.vertices.end());
  }

  if (kDebugPrintStats) {
    LOG(INFO) << "Heap operations: Adds: " << num_adds << " Pops: " << num_pops
              << " ReduceKeys: " << num_reduce_keys;
  }

  return std::move(results);
}

#endif /* SHORTEST_PATH_DIJKSTRA_SHORTEST_PATH_H_ */
