#pragma once

#include <deque>
#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace catalogue {

struct StopInfo {
  std::string_view name;
  const std::set<std::string_view>& routes;
  bool is_found = false;
};

struct RouteInfo {
  std::string_view name;
  size_t stops;
  size_t unique;
  size_t length;
  double curvature;
};

class TransportCatalogue {
 public:
  void AddStop(const std::string_view name, Coordinates coordinates, bool is_consistent);
  void AddRoute(const std::string_view route_name,
                const std::vector<std::string_view>& stop_names);
  RouteInfo GetRouteInfo(std::string_view name);
  StopInfo GetStopInfo(std::string_view name) const;
  void SetDistance(std::string_view start, std::string_view end, size_t d);
  size_t GetDistance(std::string_view start, std::string_view end) const;

 private:
  struct Stop {
    std::string name;
    Coordinates coordinates;
    std::set<std::string_view> routes;
    bool is_consistent = false;
  };

  struct Route {
    std::string name;
    std::vector<Stop*> stops;
    std::unordered_set<Stop*> unique;
  };

  std::deque<Stop> stops_;
  std::deque<Route> routes_;
  std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
  std::unordered_map<std::string_view, Route*> routename_to_route_;
  std::unordered_map<size_t, size_t> distances_;
  std::hash<std::string_view> hasher_;
  void AddStopInternal(const std::string_view name, Coordinates coordinates, bool is_consistent);
};

}