#pragma once

#include <deque>
#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <transport_catalogue.pb.h>


#include "domain.h"
#include "geo.h"

namespace catalogue {

struct StopInfo {
  std::string_view name;
  const std::set<std::string_view>& buses;
  bool is_found = false;
};

struct BusInfo {
  std::string_view name;
  int stop_count;
  int unique_stop_count;
  int route_length;
  double curvature;
  bool is_found = false;
};

class TransportCatalogue {
 public:
  void AddStop(std::string_view name, geo::Coordinates coordinates);
  void AddDraftStop(std::string_view name, geo::Coordinates coordinates);
  void AddBus(std::string_view bus_name,
              const std::vector<std::string_view>& stop_names,
              bool is_roundtrip);
  void SetDistance(std::string_view from, std::string_view to, int d);
  size_t GetDistance(std::string_view from, std::string_view to) const;
  BusInfo GetBusInfo(std::string_view name) const;
  StopInfo GetStopInfo(std::string_view name) const;
  std::vector<std::string_view> GetReachableStopNames() const;
  std::vector<std::string_view> GetBusNames() const;
  std::vector<std::string_view> GetStopsForBus(std::string_view name) const;
  geo::Coordinates GetCoordinates(std::string_view name) const;
  bool IsRoundTrip(std::string_view name) const;
  void SaveTo(serialize_proto::TransportCatalogue& catalogue_proto) const;
  void LoadFrom(const serialize_proto::TransportCatalogue& catalogue_proto);

 private:
  void AddStopInternal(std::string_view name, geo::Coordinates coordinates,
                       bool is_consistent);

  struct Stop {
    int id;
    std::string name;
    geo::Coordinates coordinates;
    std::set<std::string_view> buses;
    bool is_consistent = false;
  };

  struct Bus {
    std::string name;
    std::vector<Stop*> stops;
    std::unordered_set<Stop*> unique_stops;
    bool is_roundtrip;
  };

  std::deque<Stop> stops_;
  std::deque<Bus> buses_;
  std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
  std::unordered_map<std::string_view, Bus*> busname_to_bus_;
  std::unordered_map<size_t, int> distances_;
  std::hash<std::string_view> hasher_;
  int current_id_ = 0;
};

}  // namespace catalogue