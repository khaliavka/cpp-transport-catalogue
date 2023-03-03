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
  const json::Array& GetStatRequests() const;
  const json::Dict& GetRenderSettings() const;

 private:
  domain::StopData ProcessStop(const json::Dict& stop_map);
  domain::RouteData ProcessRoute(const json::Dict& r_map);
  json::Document document_;
  std::vector<domain::StopData> stops_;
  std::vector<domain::RouteData> routes_;
};

}  // namespace jreader