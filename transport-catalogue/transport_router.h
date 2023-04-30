#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "graph.h"
#include "json.h"
#include "router.h"
#include "serialization.h"
#include "transport_catalogue.h"

namespace transport_router {

using TrCat = catalogue::TransportCatalogue;
using Graph = graph::DirectedWeightedGraph<double>;

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

using RouteElement = std::variant<Wait, Bus>;

struct RouteInfo {
  double weight;
  std::optional<std::vector<RouteElement>> elements;
};

class TransportRouter {
 public:
  TransportRouter(RoutingSettings rs, const TrCat& tc);
  std::optional<RouteInfo> BuildRoute(std::string_view from,
                                      std::string_view to) const;
  int GetBusWaitTime() const;

  friend class serialization::Saver;
  friend class serialization::Loader;

 private:
  using VertexMap = std::unordered_map<std::string_view, graph::VertexId>;
  using EdgeMap = std::unordered_map<graph::EdgeId, RouteElement>;
  using EdgeStruct = graph::Edge<double>;

  TransportRouter(Graph&& g); 
  Graph BuildVertexMapEdgeMapAndGraph(const TrCat& transport_catalogue);

  RoutingSettings routing_settings_;
  VertexMap stop_name_to_vertex_id_;
  EdgeMap edge_id_to_route_element_;
  Graph graph_;
  graph::Router<double> router_;
};

}  // namespace transport_router
