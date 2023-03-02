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
      document_.GetRoot().AsMap().at("base_requests"s).AsArray();
  for (const auto& req : base_reqs) {
    const auto& r_map = req.AsMap();
    if (r_map.at("type"s).AsString() == "Stop"s) {
      StopData stop = ProcessStop(r_map);
      c.AddStop(stop.name, stop.coordinates);
      for (const auto& [name, dist] : stop.distances) {
        c.AddDraftStop(name, {});
        c.SetDistance(stop.name, name, dist);
      }
    }
    if (r_map.at("type"s).AsString() == "Bus"s) {
      RouteData route = ProcessRoute(r_map, r_map.at("is_roundtrip"s).AsBool());
      c.AddRoute(route.name, route.stops);
    }
  }
  return;
}

StopData JSONreader::ProcessStop(const json::Dict& stop_map) {
  StopData stop;
  stop.name = stop_map.at("name"s).AsString();
  stop.coordinates.lat = stop_map.at("latitude"s).AsDouble();
  stop.coordinates.lng = stop_map.at("longitude"s).AsDouble();
  for (const auto& [name, dist] : stop_map.at("road_distances"s).AsMap()) {
    stop.distances.emplace_back(
        std::pair<std::string_view, size_t>{name, dist.AsInt()});
  }
  return stop;
}

RouteData JSONreader::ProcessRoute(const json::Dict& route_map,
                                   bool is_roundtrip) {
  RouteData route;
  route.name = route_map.at("name"s).AsString();
  for (const auto& stop : route_map.at("stops"s).AsArray()) {
    route.stops.emplace_back(std::string_view{stop.AsString()});
  }
  if (!is_roundtrip) {
    for (size_t i = route.stops.size() - 1; i != 0; --i) {
      route.stops.push_back(route.stops[i - 1]);
    }
  }
  return route;
}

const json::Array& JSONreader::GetStatRequests() const {
  return document_.GetRoot().AsMap().at("stat_requests"s).AsArray();
}

const json::Dict& JSONreader::GetRenderSettings() const {
  return document_.GetRoot().AsMap().at("render_settings"s).AsMap();
}

}  // namespace jreader