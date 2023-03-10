#pragma once
#include <vector>

#include "domain.h"
#include "json.h"
#include "transport_catalogue.h"

namespace jreader {

class JSONreader {
 public:
  JSONreader(json::Document d);
  void ProcessBaseRequests(catalogue::TransportCatalogue& c);
  const json::Node& GetStatRequests() const;
  const json::Node& GetRenderSettings() const;

 private:
  domain::StopData ProcessStop(const json::Dict& stop_map) const;
  domain::RouteData ProcessRoute(const json::Dict& r_map) const;
  
  json::Document document_;
};

}  // namespace jreader