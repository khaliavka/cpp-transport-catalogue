#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "graph.h"
#include "json.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {

struct RoutingSettings {
  int bus_wait_time;
  double bus_velocity;
};

struct Wait {
  std::string_view stop_name;
};

struct Bus {
  std::string_view bus_name;
  int span_count;
  double time;
};

class TransportRouter {
 public:
  using RouteElement = std::variant<Wait, Bus>;
  TransportRouter(RoutingSettings rs, const catalogue::TransportCatalogue& tc);
  std::optional<std::vector<RouteElement>> BuildRoute(
      std::string_view from, std::string_view to) const;

 private:
  using Graph = graph::DirectedWeightedGraph<double>;
  using EdgeStruct = graph::Edge<double>;
  using VertexMap = std::unordered_map<std::string_view, graph::VertexId>;
  using EdgeMap = std::unordered_map<graph::EdgeId, RouteElement>;
  Graph BuildVertexMapEdgeMapAndGraph(
      const catalogue::TransportCatalogue& transport_catalogue);
  RoutingSettings routing_settings_;
  VertexMap stop_name_to_vertex_id_;
  EdgeMap edge_id_to_route_element_;
  Graph graph_;
  graph::Router<double> router_;
};

}  // namespace transport_router
