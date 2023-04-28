#pragma once
#include <vector>

#include "domain.h"
#include "json.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

namespace json_reader {

class JSONreader {
 public:
  JSONreader(json::Document d);
  void ProcessBaseRequests(catalogue::TransportCatalogue& c) const;
  const json::Node& GetStatRequests() const;
  map_renderer::RenderSettings GetRenderSettings() const;
  transport_router::RoutingSettings GetRoutingSettings() const;
  serialization::SerSettings GetSerSettings() const;
  domain::StopData ProcessStop(const json::Dict& stop_map) const;
  domain::BusData ProcessBus(const json::Dict& bus_as_dict) const;
 private:
  json::Document document_;
};

}  // namespace json_reader