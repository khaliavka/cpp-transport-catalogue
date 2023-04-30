#include "transport_router.h"

#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "graph.h"
#include "transport_catalogue.h"

namespace transport_router {

TransportRouter::TransportRouter(RoutingSettings rs, const TrCat& tc)
    : routing_settings_(rs),
      graph_{std::move(BuildVertexMapEdgeMapAndGraph(tc))},
      router_{graph_} {}

TransportRouter::TransportRouter(Graph&& g)
    : graph_{std::move(g)}, router_(graph_, false) {}

Graph TransportRouter::BuildVertexMapEdgeMapAndGraph(
    const TrCat& transport_catalogue) {
  auto reachable_stop_names = transport_catalogue.GetReachableStopNames();
  size_t stop_count = reachable_stop_names.size();
  Graph graph(2 * stop_count);
  for (size_t i = 0; i < stop_count; ++i) {
    stop_name_to_vertex_id_.insert({reachable_stop_names[i], 2 * i});
    graph::EdgeId edge_id = graph.AddEdge(
        EdgeStruct{2 * i, 2 * i + 1, 1.0 * routing_settings_.bus_wait_time});
    edge_id_to_route_element_.insert({edge_id, Wait{reachable_stop_names[i]}});
  }
  for (const auto& bus : transport_catalogue.GetBusNames()) {
    auto stop_names_for_bus = transport_catalogue.GetStopsForBus(bus);
    size_t stop_names_for_bus_count = stop_names_for_bus.size();
    for (size_t i = 0; i < stop_names_for_bus_count; ++i) {
      int span_count = 0;
      double time = 0.;
      for (size_t j = i + 1; j < stop_names_for_bus_count; ++j) {
        ++span_count;
        time += transport_catalogue.GetDistance(stop_names_for_bus[j - 1],
                                                stop_names_for_bus[j]) *
                .06 / routing_settings_.bus_velocity;
        graph::VertexId from =
            stop_name_to_vertex_id_.at(stop_names_for_bus[i]) + 1;
        graph::VertexId to = stop_name_to_vertex_id_.at(stop_names_for_bus[j]);
        graph::EdgeId edge_id = graph.AddEdge(EdgeStruct{from, to, time});
        edge_id_to_route_element_.insert({edge_id, Bus{bus, span_count, time}});
      }
    }
  }
  return graph;
}

std::optional<RouteInfo> TransportRouter::BuildRoute(
    std::string_view from, std::string_view to) const {
  if (from == to) {
    return RouteInfo{0., std::nullopt};
  }
  if (stop_name_to_vertex_id_.count(from) == 0 ||
      stop_name_to_vertex_id_.count(to) == 0) {
    return std::nullopt;
  }
  auto route_info = router_.BuildRoute(stop_name_to_vertex_id_.at(from),
                                       stop_name_to_vertex_id_.at(to));
  if (!route_info) {
    return std::nullopt;
  }
  std::vector<RouteElement> items;
  for (const auto& edge_id : route_info.value().edges) {
    const auto& element = edge_id_to_route_element_.at(edge_id);
    items.push_back(element);
  }
  return RouteInfo{route_info->weight, std::move(items)};
}

int TransportRouter::GetBusWaitTime() const {
  return routing_settings_.bus_wait_time;
}

}  // namespace transport_router