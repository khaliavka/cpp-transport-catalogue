#include "transport_catalogue.h"

#include "domain.h"
#include "geo.h"

namespace catalogue {

void TransportCatalogue::AddStopInternal(std::string_view name,
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

void TransportCatalogue::AddStop(std::string_view name,
                                 geo::Coordinates coordinates) {
  AddStopInternal(name, coordinates, true);
}

void TransportCatalogue::AddDraftStop(std::string_view name,
                                      geo::Coordinates coordinates) {
  AddStopInternal(name, coordinates, false);
}

void TransportCatalogue::AddBus(
    std::string_view bus_name,
    const std::vector<std::string_view>& stop_names, bool is_roundtrip) {
  buses_.emplace_back(Bus{std::string{bus_name}, {}, {}, is_roundtrip});

  for (const auto& name : stop_names) {
    if (stopname_to_stop_.count(name) == 0) {
      stops_.emplace_back(Stop{std::string{name}, {}, {}, false});
      stopname_to_stop_[stops_.back().name] = &stops_.back();
    }

    Stop* stop = stopname_to_stop_.at(name);
    buses_.back().stops.push_back(stop);
    buses_.back().unique_stops.insert(stop);
    stop->buses.insert(buses_.back().name);
  }
  busname_to_bus_[buses_.back().name] = &buses_.back();
}

void TransportCatalogue::SetDistance(std::string_view from,
                                     std::string_view to, size_t d) {
  distances_[hasher_(from) + 2083 * hasher_(to)] = d;
}

// Existence required
size_t TransportCatalogue::GetDistance(std::string_view from,
                                       std::string_view to) const {
  size_t hash = hasher_(from) + 2083 * hasher_(to);
  if (distances_.count(hash) == 1) {
    return distances_.at(hash);
  }
  return distances_.at(hasher_(to) + 2083 * hasher_(from));
}

BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const {
  if (busname_to_bus_.count(name) == 0) {
    return {name, {}, {}, {}, {}, false};
  }
  const Bus* bus = busname_to_bus_.at(name);
  // distance calculation
  double geo_distance = 0.;
  size_t road_distance = 0.;
  for (size_t i = 1; i < bus->stops.size(); ++i) {
    geo_distance += geo::ComputeDistance(bus->stops[i - 1]->coordinates,
                                         bus->stops[i]->coordinates);
    road_distance +=
        GetDistance(bus->stops[i - 1]->name, bus->stops[i]->name);
  }

  return {bus->name,
          static_cast<int>(bus->stops.size()),
          static_cast<int>(bus->unique_stops.size()),
          static_cast<int>(road_distance),
          road_distance / geo_distance,
          true};
}

StopInfo TransportCatalogue::GetStopInfo(std::string_view name) const {
  if (stopname_to_stop_.count(name) == 0) {
    return {name, {}, false};
  }
  const Stop* stop = stopname_to_stop_.at(name);
  return {stop->name, stop->buses, true};
}

std::vector<std::string_view> TransportCatalogue::GetReachableStopNames() const {
  std::vector<std::string_view> result;
  for (const auto& stop : stops_) {
    if (stop.buses.empty()) {
      continue;
    }
    result.push_back(stop.name);
  }
  return result;
}

std::vector<std::string_view> TransportCatalogue::GetBusNames() const {
  std::vector<std::string_view> result;
  for (const auto& [name, _] : busname_to_bus_) {
    result.push_back(name);
  }
  return result;
}

std::vector<std::string_view> TransportCatalogue::GetStopsForBus(
    std::string_view name) const {
  if (busname_to_bus_.count(name) == 0) {
    return {};
  }
  std::vector<std::string_view> result;
  const auto& stops = busname_to_bus_.at(name)->stops;
  for (const auto& stop : stops) {
    result.push_back(stop->name);
  }
  return result;
}

geo::Coordinates TransportCatalogue::GetCoordinates(
    std::string_view name) const {
  if (stopname_to_stop_.count(name) == 0) {
    return {};
  }
  return stopname_to_stop_.at(name)->coordinates;
}

bool TransportCatalogue::IsRoundTrip(std::string_view name) const {
  return busname_to_bus_.at(name)->is_roundtrip;
}

}  // namespace catalogue