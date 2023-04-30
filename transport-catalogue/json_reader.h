#pragma once
#include <vector>

#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace json_reader {

  using RoutSett = transport_router::RoutingSettings;
  using RendSett = map_renderer::RenderSettings;
  using SerSett = serialization::SerSettings;

class JSONreader {
 public:
  using TrCat = catalogue::TransportCatalogue;

  JSONreader(json::Document d);
  void ProcessBaseRequests(TrCat& c) const;
  const json::Node& GetStatRequests() const;
  RendSett GetRenderSettings() const;
  RoutSett GetRoutingSettings() const;
  SerSett GetSerSettings() const;

 private:
  domain::StopData ProcessStop(const json::Dict& stop_map) const;
  domain::BusData ProcessBus(const json::Dict& bus_as_dict) const;
  svg::Color GetColor(const json::Node& color) const;

  json::Document document_;
};

}  // namespace json_reader