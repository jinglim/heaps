#ifndef SHORTEST_PATH_BFS_SHORTEST_PATH_H_
#define SHORTEST_PATH_BFS_SHORTEST_PATH_H_

#include <queue>

#include "absl/log/log.h"

#include "graph/weighted_graph.h"
#include "shortest_path/shortest_path.h"

// A simple (not efficient) shortest path implementation that uses
// depth-first traversal through the graph.
template <typename T> class BfsShortestPath : public ShortestPath<T> {
public:
  // Factory to create an instance.
  static Factory<ShortestPath<T>> factory() {
    return Factory<ShortestPath<T>>{"BFS Shortest Path",
                                    []() { return new BfsShortestPath<T>{}; }};
  };

  virtual std::unordered_map<VertexId, Path<T>>
  Run(const WeightedGraph<T> &graph, VertexId start_vertex_index) override;

private:
  void Bfs_(const WeightedGraph<T> &weighted_graph, VertexId start_vertex_id,
            std::unordered_map<VertexId, Path<T>> *results);
};

// Recursive BFS to find shortest path.
template <typename T>
std::unordered_map<VertexId, Path<T>>
BfsShortestPath<T>::Run(const WeightedGraph<T> &weighted_graph,
                        VertexId start_vertex_index) {
  std::unordered_map<VertexId, Path<T>> results;
  Bfs_(weighted_graph, start_vertex_index, &results);
  return results;
}

template <typename T>
void BfsShortestPath<T>::Bfs_(const WeightedGraph<T> &weighted_graph,
                              VertexId start_vertex_id,
                              std::unordered_map<VertexId, Path<T>> *results) {

  Path<T> start_path(0);
  start_path.vertices.push_back(start_vertex_id);
  results->emplace(start_vertex_id, start_path);

  std::queue<VertexId> queue;
  queue.push(start_vertex_id);

  while (!queue.empty()) {
    VertexId vertex_id = queue.front();
    queue.pop();
    const auto &vertex = weighted_graph.graph->GetVertex(vertex_id);
    T current_distance = results->at(vertex_id).distance;

    for (const auto &edge : vertex.edges()) {
      auto total_distance =
          current_distance + weighted_graph.edge_weights->Get(edge.id());

      // Check against current shortest distance to this vertex.
      auto it = results->find(edge.to_vertex_id());
      if (it != results->end() && total_distance >= it->second.distance) {
        // Previously found a shorter path.
        continue;
      }

      // Insert path to results.
      Path<T> path{total_distance};
      path.vertices = results->at(vertex_id).vertices;
      path.vertices.push_back(edge.to_vertex_id());

      if (it != results->end()) {
        // Update with shorter path.
        it->second = std::move(path);
      } else {
        results->emplace(edge.to_vertex_id(), std::move(path));
      }

      queue.push(edge.to_vertex_id());
    }
  }
}

#endif /* SHORTEST_PATH_BFS_SHORTEST_PATH_H_ */
