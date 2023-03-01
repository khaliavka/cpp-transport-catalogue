#include "transport_catalogue.h"

#include "domain.h"
#include "geo.h"

namespace catalogue {

void TransportCatalogue::AddStopInternal(const std::string_view name,
                                         geo::Coordinates coordinates,
                                         bool is_consistent) {
  if (stopname_to_stop_.count(name) == 0) {
    stops_.emplace_back(
        Stop{std::string{name}, coordinates, {}, is_consistent});
    stopname_to_stop_[stops_.back().name] = &stops_.back();
    return;
  }
  Stop* stop = stopname_to_stop_.at(name);
  if (stop->is_consistent) {
    return;
  }
  stop->coordinates = coordinates;
  stop->is_consistent = is_consistent;
}

void TransportCatalogue::AddStop(const std::string_view name,
                                 geo::Coordinates coordinates) {
  AddStopInternal(name, coordinates, true);
}

void TransportCatalogue::AddDraftStop(const std::string_view name,
                                 geo::Coordinates coordinates) {
  AddStopInternal(name, coordinates, false);
}

void TransportCatalogue::AddRoute(
    const std::string_view route_name,
    const std::vector<std::string_view>& stop_names) {
  routes_.emplace_back(Route{std::string{route_name}, {}, {}});

  for (const auto& name : stop_names) {
    if (stopname_to_stop_.count(name) == 0) {
      stops_.emplace_back(Stop{std::string{name}, {}, {}, false});
      stopname_to_stop_[stops_.back().name] = &stops_.back();
    }

    Stop* stop = stopname_to_stop_.at(name);
    routes_.back().stops.push_back(stop);
    routes_.back().unique.insert(stop);
    stop->routes.insert(routes_.back().name);
  }
  routename_to_route_[routes_.back().name] = &routes_.back();
  return;
}

domain::RouteInfo TransportCatalogue::GetRouteInfo(std::string_view name) const {
  if (routename_to_route_.count(name) == 0) {
    return {name, {}, {}, {}, {}, false};
  }
  const Route* route = routename_to_route_.at(name);
  // distance calculation
  double g_distance = 0.;
  size_t r_distance = 0.;
  for (size_t i = 1; i < route->stops.size(); ++i) {
    g_distance += geo::ComputeDistance(route->stops[i - 1]->coordinates,
                                       route->stops[i]->coordinates);
    r_distance += GetDistance(route->stops[i - 1]->name, route->stops[i]->name);
  }

  return {route->name, route->stops.size(), route->unique.size(), r_distance,
          r_distance / g_distance, true};
}

domain::StopInfo TransportCatalogue::GetStopInfo(std::string_view name) const {
  if (stopname_to_stop_.count(name) == 0) {
    return {name, {}, false};
  }
  const Stop* stop = stopname_to_stop_.at(name);
  return {stop->name, stop->routes, true};
}

void TransportCatalogue::SetDistance(std::string_view start,
                                     std::string_view end, size_t d) {
  distances_[hasher_(start) + 2083 * hasher_(end)] = d;
}

// Existence required
size_t TransportCatalogue::GetDistance(std::string_view start,
                                       std::string_view end) const {
  size_t hash = hasher_(start) + 2083 * hasher_(end);
  if (distances_.count(hash)) {
    return distances_.at(hash);
  }
  return distances_.at(hasher_(end) + 2083 * hasher_(start));
}

}  // namespace catalogue