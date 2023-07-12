#include "graph/graph.h"

#include "absl/log/check.h"

namespace {

// Generates sequential int ids starting from 0.
class IdCounter {
public:
  IdCounter() : id_(0) {}

  int next() { return id_++; }
  int count() const { return id_; }

private:
  int id_;
};

// Holds per-vertex data while building a Graph.
struct VertexData {
  VertexData(VertexId id) : id(id) {}
  VertexData(const VertexData &other) = delete;
  VertexData(VertexData &&other)
      : id(other.id), edges(std::move(other.edges)) {}

  VertexId id;
  std::vector<Edge> edges;
};

class GraphBuilderImpl : public GraphBuilder {
public:
  GraphBuilderImpl(const std::string &name) : name_(name) {}

  virtual VertexId AddVertex() override {
    VertexId vertex_id = static_cast<VertexId>(vertices_.size());
    vertices_.emplace_back(vertex_id);
    return vertex_id;
  }

  virtual EdgeId AddEdge(VertexId from_id, VertexId to_id) override {
    DCHECK(from_id < vertices_.size() && to_id < vertices_.size());

    EdgeId edge_id = edge_id_counter_.next();
    vertices_[from_id].edges.emplace_back(edge_id, to_id);
    return edge_id;
  }

  virtual std::unique_ptr<Graph> Build() override {
    auto vertices = std::make_unique<std::vector<Vertex>>();

    for (const auto &vertex_data : vertices_) {
      vertices->emplace_back(
          vertex_data.id,
          std::make_unique<std::vector<Edge>>(std::move(vertex_data.edges)));
    }

    std::unique_ptr<Graph> graph = std::make_unique<Graph>(
        name_, std::move(*vertices.release()), edge_id_counter_.count());
    return graph;
  }

private:
  std::string name_;
  IdCounter edge_id_counter_;
  std::vector<VertexData> vertices_;
};
} // namespace

void Graph::Validate() {
  for (const auto &vertex : vertices_) {
    CHECK(vertex.id() < num_vertices());

    for (const auto &edge : vertex.edges()) {
      CHECK(edge.id() < num_edges_);
    }
  }
}

std::unique_ptr<GraphBuilder> GraphBuilder::Builder(const std::string &name) {
  return std::make_unique<GraphBuilderImpl>(name);
}
