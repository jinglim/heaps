#ifndef GRAPH_WEIGHTED_GRAPH_H_
#define GRAPH_WEIGHTED_GRAPH_H_

#include "graph/graph.h"
#include "graph/properties.h"

// A Graph that has weights of type T on the edges.
template <typename T> class WeightedGraph {
public:
  WeightedGraph(std::unique_ptr<Graph> graph,
                std::unique_ptr<Properties<T>> edge_weights)
      : graph(std::move(graph)), edge_weights(std::move(edge_weights)) {}

  // Print the graph, for debugging.
  void PrintGraph(std::ostream &out) const;

  std::unique_ptr<Graph> graph;

  // Weights on the edges.
  std::unique_ptr<Properties<T>> edge_weights;
};

// Print the weighted graph.
template <typename T> void WeightedGraph<T>::PrintGraph(std::ostream &out) const {
  out << "Graph(" << graph->name() << ")" << std::endl;
  for (const auto &vertex : graph->vertices()) {
    out << "Vertex " << vertex.id() << std::endl;

    for (const auto &edge : vertex.edges()) {
      out << " " << vertex.id() << " -> " << edge.to_vertex_id() << " ("
          << edge_weights->Get(edge.id()) << ")" << std::endl;
    }
  }
}

#endif /* GRAPH_WEIGHTED_GRAPH_H_ */
