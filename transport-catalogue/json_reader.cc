#include "json_reader.h"

#include <string>
#include <string_view>
#include <utility>

#include "domain.h"
#include "json.h"
#include "transport_catalogue.h"

namespace json_reader {

using namespace std::literals;
using namespace domain;
using namespace catalogue;

JSONreader::JSONreader(json::Document d) : document_(std::move(d)) {}

void JSONreader::ProcessBaseRequests(TransportCatalogue& c) {
  const auto& base_reqs =
      document_.GetRoot().AsDict().at("base_requests"s).AsArray();
  for (const auto& req : base_reqs) {
    const auto& req_as_dict = req.AsDict();
    if (req_as_dict.at("type"s).AsString() == "Stop"s) {
      StopData stop = ProcessStop(req_as_dict);
      c.AddStop(stop.name, stop.coordinates);
      for (const auto& [other_name, distance] : stop.distances) {
        c.AddDraftStop(other_name, {});
        c.SetDistance(stop.name, other_name, distance);
      }
    }
    if (req_as_dict.at("type"s).AsString() == "Bus"s) {
      BusData bus = ProcessBus(req_as_dict);
      c.AddBus(bus.name, bus.stops, bus.is_roundtrip);
    }
  }
  return;
}

StopData JSONreader::ProcessStop(const json::Dict& stop_as_dict) const {
  StopData stop;
  stop.name = stop_as_dict.at("name"s).AsString();
  stop.coordinates.lat = stop_as_dict.at("latitude"s).AsDouble();
  stop.coordinates.lng = stop_as_dict.at("longitude"s).AsDouble();
  for (const auto& [name, dist] : stop_as_dict.at("road_distances"s).AsDict()) {
    stop.distances.emplace_back(
        std::pair<std::string_view, size_t>{name, dist.AsInt()});
  }
  return stop;
}

BusData JSONreader::ProcessBus(const json::Dict& bus_as_dict) const {
  BusData bus;
  bus.name = bus_as_dict.at("name"s).AsString();
  for (const auto& stop : bus_as_dict.at("stops"s).AsArray()) {
    bus.stops.emplace_back(std::string_view{stop.AsString()});
  }
  bool is_roundtrip = bus_as_dict.at("is_roundtrip"s).AsBool();
  if (!is_roundtrip) {
    for (size_t i = bus.stops.size() - 1; i != 0; --i) {
      bus.stops.push_back(bus.stops[i - 1]);
    }
  }
  bus.is_roundtrip = is_roundtrip;
  return bus;
}

const json::Node& JSONreader::GetStatRequests() const {
  return document_.GetRoot().AsDict().at("stat_requests"s);
}

const json::Node& JSONreader::GetRenderSettings() const {
  return document_.GetRoot().AsDict().at("render_settings"s);
}

const json::Node& JSONreader::GetRoutingSettings() const {
  return document_.GetRoot().AsDict().at("routing_settings"s);
}
}  // namespace json_reader