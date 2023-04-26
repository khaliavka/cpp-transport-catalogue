#include "transport_catalogue.h"

#include <string>
#include <string_view>
#include <transport_catalogue.pb.h>

#include "domain.h"
#include "geo.h"

namespace catalogue {

void TransportCatalogue::AddStopInternal(std::string_view name,
                                         geo::Coordinates coordinates,
                                         bool is_consistent) {
  if (stopname_to_stop_.count(name) == 0) {
    stops_.emplace_back(
        Stop{current_id_, std::string{name}, coordinates, {}, is_consistent});
    ++current_id_;
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

void TransportCatalogue::AddBus(std::string_view bus_name,
                                const std::vector<std::string_view>& stop_names,
                                bool is_roundtrip) {
  buses_.emplace_back(Bus{std::string{bus_name}, {}, {}, is_roundtrip});

  for (const auto& name : stop_names) {
    if (stopname_to_stop_.count(name) == 0) {
      stops_.emplace_back(Stop{current_id_, std::string{name}, {}, {}, false});
      ++current_id_;
      stopname_to_stop_[stops_.back().name] = &stops_.back();
    }

    Stop* stop = stopname_to_stop_.at(name);
    buses_.back().stops.push_back(stop);
    buses_.back().unique_stops.insert(stop);
    stop->buses.insert(buses_.back().name);
  }
  busname_to_bus_[buses_.back().name] = &buses_.back();
}

void TransportCatalogue::SetDistance(std::string_view from, std::string_view to,
                                     int d) {
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
    road_distance += GetDistance(bus->stops[i - 1]->name, bus->stops[i]->name);
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

std::vector<std::string_view> TransportCatalogue::GetReachableStopNames()
    const {
  std::vector<std::string_view> result;
  for (const auto& stop : stops_) {
    if (!stop.buses.empty()) {
      result.push_back(stop.name);
    }
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

void TransportCatalogue::SaveTo(
    serialize_proto::TransportCatalogue& catalogue_proto) const {
  for (const auto& stop : stops_) {
    auto serial_stop = catalogue_proto.add_stop();
    serial_stop->set_id(stop.id);
    serial_stop->set_name(stop.name);
    serial_stop->mutable_coordinates()->set_lat(stop.coordinates.lat);
    serial_stop->mutable_coordinates()->set_lng(stop.coordinates.lng);
    serial_stop->set_is_consistent(stop.is_consistent);
  }
  for (const auto& bus : buses_) {
    auto serial_bus = catalogue_proto.add_bus();
    serial_bus->set_name(bus.name);
    serial_bus->set_is_roundtrip(bus.is_roundtrip);
    for (auto stop : bus.stops) {
      serial_bus->add_stop_id(stop->id);
    }
  }
  for (const auto& [key, val] : distances_) {
    auto serial_dist = catalogue_proto.add_distance();
    serial_dist->set_key(key);
    serial_dist->set_val(val);
  }
}

void TransportCatalogue::LoadFrom(
    const serialize_proto::TransportCatalogue& catalogue_proto) {
  for (const auto& s : catalogue_proto.stop()) {
    auto& stop = stops_.emplace_back(Stop{});
    stop.id = s.id();
    stop.name = s.name();
    stop.coordinates.lat = s.coordinates().lat();
    stop.coordinates.lng = s.coordinates().lng();
    stop.is_consistent = s.is_consistent();
    stopname_to_stop_[stop.name] = &stop;
  }
  for (const auto& b : catalogue_proto.bus()) {
    auto& bus = buses_.emplace_back(Bus{});
    bus.name = b.name();
    for (const auto& id : b.stop_id()) {
      bus.stops.push_back(&stops_[id]);
      bus.unique_stops.insert(&stops_[id]);
      stops_[id].buses.insert(bus.name);
    }
    bus.is_roundtrip = b.is_roundtrip();
    busname_to_bus_[bus.name] = &bus;
  }
  for (const auto& d : catalogue_proto.distance()) {
    distances_[d.key()] = d.val();
  }
  current_id_ = stops_.size();
}

}  // namespace catalogue