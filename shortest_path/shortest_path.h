#ifndef SHORTEST_PATH_SHORTEST_PATH_H_
#define SHORTEST_PATH_SHORTEST_PATH_H_

#include <unordered_map>

// Path from a starting index to an ending index, with a total distance.
template <typename T> struct Path {
  Path(T distance) : distance(distance) {}
  Path(const Path &other) = default;
  Path(Path &&other)
      : vertices(std::move(other.vertices)), distance(other.distance) {}

  Path &operator=(Path &&other) {
    vertices = std::move(other.vertices);
    distance = other.distance;
    return *this;
  }

  // List of vertices starting from start to end vertex.
  std::vector<VertexId> vertices;

  // Total distance.
  T distance;
};

template <typename T>
std::ostream &operator<<(std::ostream &out, const Path<T> &path) {
  out << "Path (";
  for (int i = 0; i < path.vertices.size(); i++) {
    if (i > 0) {
      out << "->";
    }
    out << path.vertices[i];
  }
  out << ", distance: " << path.distance << ")";
  return out;
}

// Computes shortest path for a given graph.
template <typename T> class ShortestPath {
public:
  virtual ~ShortestPath() {}

  // Given a graph, and distances on the edges, compute shortest path from the
  // `start_vertex_index` to all the vertices.
  virtual std::unordered_map<VertexId, Path<T>>
  Run(const WeightedGraph<T> &graph, VertexId start_vertex_index) = 0;
};

#endif /* SHORTEST_PATH_SHORTEST_PATH_H_ */
