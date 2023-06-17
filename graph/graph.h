#ifndef GRAPH_GRAPH_H_
#define GRAPH_GRAPH_H_

#include <iostream>
#include <vector>

class Edge;

// Each vertex is identified by its VertexId.
typedef int VertexId;

// Each Edge is identified by its EdgeId.
typedef int EdgeId;

// A vertex in a Graph.
class Vertex {
public:
  Vertex(VertexId id, std::unique_ptr<std::vector<Edge>> edges)
      : id_(id), edges_(std::move(edges)) {}
  Vertex(Vertex &&other) : id_(other.id_), edges_(std::move(other.edges_)) {}

  // Unique id that identifies the vertex. 
  // Ranges from 0 to (num nodes in graph - 1). 
  VertexId id() const { return id_; };

  // List of all directed edges from this vertex.
  const std::vector<Edge> &edges() const { return *edges_; }

private:
  int id_;
  std::unique_ptr<std::vector<Edge>> edges_;
};

// A directed edge in a Graph.
// Each instance is relative to a starting vertex.
class Edge {
public:
  Edge(EdgeId id, VertexId to_vertex_id)
      : id_(id), to_vertex_id_(to_vertex_id) {}

  // An id that is unique in the graph.
  // Ranges from 0 to (num edges in graph - 1).
  EdgeId id() const { return id_; }

  // Returns the destination vertex id.
  VertexId to_vertex_id() const { return to_vertex_id_; }

private:
  EdgeId id_;
  VertexId to_vertex_id_;
};

// An immutable Graph consisting of nodes and directed edges.
// Use GraphBuilder to build the graph.
class Graph {
public:
  Graph(const std::string &name,
        std::vector<Vertex>&& vertices, int num_edges)
      : vertices_(std::move(vertices)), num_edges_(num_edges), name_(name) {}
  Graph(Graph &&other)
      : vertices_(std::move(other.vertices_)),
        num_edges_(other.num_edges_), name_(std::move(other.name_)) {}

  // Name for printing, labeling the graph.
  const std::string &name() const { return name_; }

  // Returns all vertices.
  const std::vector<Vertex> &vertices() const { return vertices_; }

  // Total number of vertices.
  int num_vertices() const { return static_cast<int>(vertices_.size()); }

  // Total number of edges.
  int num_edges() const { return num_edges_; }

  // Get a specific vertex.
  const Vertex &GetVertex(VertexId vertex_id) const {
    return vertices_[vertex_id];
  }

  // Check the invariants.
  void Validate();

private:
  std::vector<Vertex> vertices_;
  int num_edges_;
  std::string name_;
};

// Builds a Graph.
class GraphBuilder {
public:
  static std::unique_ptr<GraphBuilder> Builder(const std::string &name);

  virtual ~GraphBuilder() {}

  // Adds a Vertex, returns the node id.
  virtual VertexId AddVertex() = 0;

  // Adds an Edge.
  virtual EdgeId AddEdge(VertexId from_id, VertexId to_id) = 0;

  // Builds a Graph.
  // After this, no more vertices should be added.
  virtual std::unique_ptr<Graph> Build() = 0;
};

#endif /* GRAPH_GRAPH_H_ */
