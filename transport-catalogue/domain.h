#pragma once

#include <set>
#include <string_view>
#include <vector>

#include "geo.h"

namespace domain {

struct StopData {
  std::string_view name;
  geo::Coordinates coordinates;
  std::vector<std::pair<std::string_view, size_t>> distances;
};

struct RouteData {
  std::string_view name;
  std::vector<std::string_view> stops;
};

struct StopInfo {
  std::string_view name;
  const std::set<std::string_view>& routes;
  bool is_found = false;
};

struct RouteInfo {
  std::string_view name;
  size_t count;
  size_t unique;
  size_t length;
  double curvature;
  bool is_found = false;
};

}  // namespace domain