#include "json_reader.h"

#include <string>
#include <string_view>
#include <utility>

#include "domain.h"
#include "json.h"
#include "transport_catalogue.h"

namespace jreader {

using namespace std::literals;
using namespace domain;
using namespace catalogue;

JSONreader::JSONreader(json::Document d) : document_(std::move(d)) {}

void JSONreader::ProcessBaseRequests(TransportCatalogue& c) {
  const auto& base_reqs =
      document_.GetRoot().AsDict().at("base_requests"s).AsArray();
  for (const auto& req : base_reqs) {
    const auto& r_map = req.AsDict();
    if (r_map.at("type"s).AsString() == "Stop"s) {
      StopData stop = ProcessStop(r_map);
      c.AddStop(stop.name, stop.coordinates);
      for (const auto& [name, dist] : stop.distances) {
        c.AddDraftStop(name, {});
        c.SetDistance(stop.name, name, dist);
      }
    }
    if (r_map.at("type"s).AsString() == "Bus"s) {
      RouteData route = ProcessRoute(r_map);
      c.AddRoute(route.name, route.stops, route.is_roundtrip);
    }
  }
  return;
}

StopData JSONreader::ProcessStop(const json::Dict& stop_map) const {
  StopData stop;
  stop.name = stop_map.at("name"s).AsString();
  stop.coordinates.lat = stop_map.at("latitude"s).AsDouble();
  stop.coordinates.lng = stop_map.at("longitude"s).AsDouble();
  for (const auto& [name, dist] : stop_map.at("road_distances"s).AsDict()) {
    stop.distances.emplace_back(
        std::pair<std::string_view, size_t>{name, dist.AsInt()});
  }
  return stop;
}

RouteData JSONreader::ProcessRoute(const json::Dict& r_map) const {
  RouteData route;
  route.name = r_map.at("name"s).AsString();
  for (const auto& stop : r_map.at("stops"s).AsArray()) {
    route.stops.emplace_back(std::string_view{stop.AsString()});
  }
  bool is_roundtrip = r_map.at("is_roundtrip"s).AsBool();
  if (!is_roundtrip) {
    for (size_t i = route.stops.size() - 1; i != 0; --i) {
      route.stops.push_back(route.stops[i - 1]);
    }
  }
  route.is_roundtrip = is_roundtrip;
  return route;
}

const json::Node& JSONreader::GetStatRequests() const {
  return document_.GetRoot().AsDict().at("stat_requests"s);
}

const json::Node& JSONreader::GetRenderSettings() const {
  return document_.GetRoot().AsDict().at("render_settings"s);
}

}  // namespace jreader