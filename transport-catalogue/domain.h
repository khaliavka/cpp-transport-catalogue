#pragma once

#include <set>
#include <string_view>
#include <vector>

#include "geo.h"

namespace domain {

struct Stop {
  std::string_view name;
  geo::Coordinates coordinates;
};

struct Bus {
  std::string_view name;
  std::vector<Stop> stops;
  bool is_roundtrip = false;
};

struct StopData {
  std::string_view name;
  geo::Coordinates coordinates;
  std::vector<std::pair<std::string_view, int>> distances;
};

struct BusData {
  std::string_view name;
  std::vector<std::string_view> stops;
  bool is_roundtrip = false;
};

}  // namespace domain