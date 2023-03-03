#include "input_reader.h"

#include <iostream>
#include <queue>
#include <string>
#include <string_view>
#include <utility>

#include "geo.h"
#include "transport_catalogue.h"

namespace io {

std::string InputReader::ReadLine() {
  std::string s;
  std::getline(std::cin, s);
  return s;
}

void InputReader::ReadInput() {
  std::string s;
  for (; getline(input_, s);) {
    size_t count = std::stoull(s);
    for (size_t i = 0; i < count; ++i) {
      buffer_.push(move(ReadLine()));
    }
  }
}

std::pair<std::string_view, std::string_view> InputReader::Split(
    std::string_view line, std::string_view by) {
  size_t pos = line.find(by);
  std::string_view left = line.substr(0, pos);

  if (pos < line.size() && pos + by.size() < line.size()) {
    return {left, line.substr(pos + by.size())};
  } else {
    return {left, std::string_view()};
  }
}

std::string_view InputReader::Crop(std::string_view str,
                                   std::string_view symbols) {
  if (str.empty()) {
    return str;
  }
  str = str.substr(str.find_first_not_of(symbols));
  return str.substr(0, str.find_last_not_of(symbols) + 1);
}

std::vector<std::string_view> InputReader::Chop(std::string_view str,
                                                std::string_view by) {
  using namespace std::literals;
  std::vector<std::string_view> slices;
  auto p = Split(str, by);
  for (; !p.second.empty();) {
    p.first = Crop(p.first, " "sv);
    slices.push_back(p.first);
    p = Split(p.second, by);
  }
  p.first = Crop(p.first, " "sv);
  slices.push_back(p.first);
  return slices;
}

InputReader::Query InputReader::ParseQuery(std::string_view raw_query) {
  using namespace std::literals;
  auto [type, data] = Split(raw_query, " "sv);
  type = Crop(type, " ");
  Query query;
  if (type == "Stop"sv) {
    query.type = (data.find(":"sv) != std::string_view::npos)
                     ? QueryType::ADD_STOP
                     : QueryType::GET_STOP_INFO;
  }
  if (type == "Bus"sv) {
    query.type = (data.find(":"sv) != std::string_view::npos)
                     ? QueryType::ADD_ROUTE
                     : QueryType::GET_ROUTE_INFO;
  }
  query.data = data;
  return query;
}

domain::StopData InputReader::ParseAddStop(std::string_view str) {
  using namespace std;
  auto [name, data] = Split(str, ":"sv);
  name = Crop(name, " "sv);
  vector<pair<string_view, size_t>> distances;
  vector<string_view> slices = Chop(data, ","sv);
  for (size_t i = 2; i < slices.size(); ++i) {
    auto [dist, stop] = Split(slices[i], "m to "sv);
    stop = Crop(stop, " "sv);
    distances.emplace_back(pair{stop, stoull(string{dist})});
  }
  return {name, geo::Coordinates{stod(string{slices[0]}), stod(string{slices[1]})},
          move(distances)};
}

domain::RouteData InputReader::ParseAddRoute(std::string_view str) {
  using namespace std::literals;
  domain::RouteData result;
  auto [name, stops] = Split(str, ":"sv);
  name = Crop(name, " "sv);
  result.name = name;
  result.is_roundtrip = true;
  auto& rs = result.stops;
  rs = Chop(stops, ">"sv);
  if (rs.size() == 1ull) {
    rs = Chop(stops, "-"sv);
    for (size_t i = rs.size() - 1; i != 0; --i) {
      rs.push_back(rs[i - 1]);
    }
    result.is_roundtrip = false;
  }
  return result;
}

std::string_view InputReader::ParseGetRouteInfo(std::string_view str) {
  using namespace std::literals;
  return Crop(str, " "sv);
}

std::string_view InputReader::ParseGetStopInfo(std::string_view str) {
  using namespace std::literals;
  return Crop(str, " "sv);
}

void InputReader::ProcessQueries(catalogue::TransportCatalogue& c) {
  for (; !buffer_.empty();) {
    auto query = ParseQuery(buffer_.front());
    domain::StopData stop;
    domain::RouteData route;
    switch (query.type) {
      case QueryType::ADD_STOP:
        stop = ParseAddStop(query.data);
        c.AddStop(stop.name, stop.coordinates);
        for (const auto& [name, dist] : stop.distances) {
          c.AddDraftStop(name, {});
          c.SetDistance(stop.name, name, dist);
        }
        break;
      case QueryType::ADD_ROUTE:
        route = ParseAddRoute(query.data);
        c.AddRoute(route.name, route.stops, route.is_roundtrip);
        break;
      case QueryType::GET_ROUTE_INFO:
        stat_reader_.PrintRouteInfo(
            c.GetRouteInfo(ParseGetRouteInfo(query.data)));
        break;
      case QueryType::GET_STOP_INFO:
        stat_reader_.PrintStopInfo(c.GetStopInfo(ParseGetStopInfo(query.data)));
        break;
    }
    buffer_.pop();
  }
}

}  // namespace io
