#include <chrono>
#include <map>
#include <unordered_set>

#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/flags.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"

#include "graph/weighted_graph.h"
#include "heaps/binary_heap.h"
#include "heaps/binomial_heap.h"
#include "heaps/pairing_heap.h"
#include "heaps/heap.h"
#include "heaps/weak_heap.h"
#include "shortest_path/bfs_shortest_path.h"
#include "shortest_path/dijkstra_shortest_path.h"

namespace {

bool kEnableDebugLogging = false;

// Tests Shortest Path implementations.
class ShortestPathTester {
public:
  ShortestPathTester(const std::vector<Factory<ShortestPath<int>>> &factories)
      : factories_(factories) {}

  // Test against the given weighted graph.
  void Run(const WeightedGraph<int> &weighted_graph, VertexId start_vertex_id) {
    if (kEnableDebugLogging) {
      weighted_graph.PrintGraph(std::cerr);
    }

    std::vector<std::unordered_map<VertexId, Path<int>>> all_results;

    for (const auto &factory : factories_) {
      LOG(INFO) << "Using " << factory.name();

      std::unique_ptr<ShortestPath<int>> shortest_path{factory()};
      auto results = shortest_path->Run(weighted_graph, start_vertex_id);
      all_results.emplace_back(std::move(results));
    }

    // Compare the results from different implementations.
    std::map<VertexId, Path<int>> sorted_results(all_results.front().begin(),
                                                 all_results.front().end());
    for (const auto &pair : sorted_results) {
      auto vertex_id = pair.first;
      const auto &path = pair.second;

      if (kEnableDebugLogging) {
        LOG(INFO) << vertex_id << ": " << path;
      }

      for (int i = 1; i < all_results.size(); i++) {
        auto it = all_results[i].find(vertex_id);
        if (it == all_results[i].end()) {
          LOG(WARNING) << factories_[i].name() << ": vertex " << vertex_id
                       << " not found";
          continue;
        }
        const auto &path2 = it->second;
        if (path.distance != path2.distance ||
            path.vertices != path2.vertices) {
          LOG(WARNING) << factories_[i].name() << ": " << path << " vs "
                       << factories_[0].name() << ": " << path2;
          continue;
        }
      }
    }
  }

  void TestSimpleGraph() {
    LOG(INFO) << "Testing simple graph";
    WeightedGraph<int> weighted_graph = BuildSimpleGraph_();
    Run(weighted_graph, 0);
  }

  void TestRandomGraph() {
    LOG(INFO) << "Testing random graph";
    WeightedGraph<int> weighted_graph = BuildRandomGraph_();
    Run(weighted_graph, 0);
  }

private:
  static WeightedGraph<int> BuildSimpleGraph_() {
    std::unique_ptr<GraphBuilder> builder = GraphBuilder::Builder("simple");
    std::unique_ptr<Properties<int>> distances =
        std::make_unique<Properties<int>>(0);

    // x -> y (5)
    auto x = builder->AddVertex();
    auto y = builder->AddVertex();
    auto x_y = builder->AddEdge(x, y);
    distances->Set(x_y, 5);

    // x -> z (3)
    auto z = builder->AddVertex();
    auto x_z = builder->AddEdge(x, z);
    distances->Set(x_z, 3);

    // y -> a (10)
    auto a = builder->AddVertex();
    auto y_a = builder->AddEdge(y, a);
    distances->Set(y_a, 10);

    // z -> a (20)
    auto z_a = builder->AddEdge(z, a);
    distances->Set(z_a, 20);

    std::unique_ptr<Graph> graph = builder->Build();
    graph->Validate();

    return WeightedGraph<int>(std::move(graph), std::move(distances));
  }

  // Builds a Random Graph.
  static WeightedGraph<int> BuildRandomGraph_() {
    const int num_vertices = 1000;
    const int num_edges_per_vertex = 20;
    std::unique_ptr<GraphBuilder> builder = GraphBuilder::Builder("random");
    std::unique_ptr<Properties<int>> distances =
        std::make_unique<Properties<int>>(0);

    VertexId vertices[num_vertices];
    for (int i = 0; i < num_vertices; i++) {
      vertices[i] = builder->AddVertex();
    }

    for (int i = 0; i < num_vertices; i++) {
      for (int j = 0; j < num_edges_per_vertex; ++j) {
        auto edge =
            builder->AddEdge(vertices[i], vertices[rand() % num_vertices]);
        distances->Set(edge, rand() % 100000);
      }
    }

    std::unique_ptr<Graph> graph = builder->Build();
    graph->Validate();

    return WeightedGraph<int>(std::move(graph), std::move(distances));
  }

  const std::vector<Factory<ShortestPath<int>>> &factories_;
};

void RunShortestPathTests() {
  Factory<ShortestPath<int>> f1 = DijkstraShortestPath<int>::factory(
      BinaryHeap<DistanceNode<int>>::factory());

  std::vector<Factory<ShortestPath<int>>> factories{
      BfsShortestPath<int>::factory(),
      DijkstraShortestPath<int>::factory(
          BinaryHeap<DistanceNode<int>>::factory()),
      DijkstraShortestPath<int>::factory(
          BinomialHeap<DistanceNode<int>>::factory()),
      DijkstraShortestPath<int>::factory(
          WeakHeap<DistanceNode<int>>::factory()),
            DijkstraShortestPath<int>::factory(
          PairingHeap<DistanceNode<int>>::factory()),  
  };
  ShortestPathTester tester{factories};
  tester.TestSimpleGraph();
  tester.TestRandomGraph();
}

} // namespace

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  absl::InitializeLog();

  RunShortestPathTests();
  std::cout << "Done." << std::endl;
  return 0;
}
