#pragma once
#include <vector>

#include "domain.h"
#include "json.h"
#include "transport_catalogue.h"

namespace json_reader {

class JSONreader {
 public:
  JSONreader(json::Document d);
  void ProcessBaseRequests(catalogue::TransportCatalogue& c);
  const json::Node& GetStatRequests() const;
  const json::Node& GetRenderSettings() const;
  const json::Node& GetRoutingSettings() const;

 private:
  domain::StopData ProcessStop(const json::Dict& stop_map) const;
  domain::BusData ProcessBus(const json::Dict& bus_as_dict) const;
  json::Document document_;
};

}  // namespace json_reader