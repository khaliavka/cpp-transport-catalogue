#pragma once

#include <deque>
#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "domain.h"
#include "geo.h"

namespace catalogue {

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

class TransportCatalogue {
 public:
  void AddStop(const std::string_view name, geo::Coordinates coordinates);
  void AddDraftStop(const std::string_view name, geo::Coordinates coordinates);
  void AddRoute(const std::string_view route_name,
                const std::vector<std::string_view>& stop_names,
                bool is_roundtrip);
  void SetDistance(std::string_view start, std::string_view end, size_t d);
  size_t GetDistance(std::string_view start, std::string_view end) const;
  RouteInfo GetRouteInfo(std::string_view name) const;
  StopInfo GetStopInfo(std::string_view name) const;
  std::vector<std::string_view> GetRouteNames() const;
  std::vector<std::string_view> GetStopsForRoute(std::string_view name) const;
  geo::Coordinates GetCoordinates(std::string_view name) const;
  bool IsRoundTrip(std::string_view name) const;

 private:
  struct Stop {
    std::string name;
    geo::Coordinates coordinates;
    std::set<std::string_view> routes;
    bool is_consistent = false;
  };

  struct Route {
    std::string name;
    std::vector<Stop*> stops;
    std::unordered_set<Stop*> unique;
    bool is_roundtrip;
  };

  void AddStopInternal(const std::string_view name,
                       geo::Coordinates coordinates, bool is_consistent);

  std::deque<Stop> stops_;
  std::deque<Route> routes_;
  std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
  std::unordered_map<std::string_view, Route*> routename_to_route_;
  std::unordered_map<size_t, size_t> distances_;
  std::hash<std::string_view> hasher_;
};

}  // namespace catalogue