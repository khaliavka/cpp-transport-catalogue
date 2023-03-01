#pragma once

#include <iostream>
#include <queue>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

namespace io {

enum class QueryType { ADD_STOP, ADD_ROUTE, GET_ROUTE_INFO, GET_STOP_INFO };

class InputReader {
 public:
  InputReader(std::istream& in, std::ostream& out)
      : input_(in), stat_reader_(out){};
  void ReadInput();
  void ProcessQueries(catalogue::TransportCatalogue& c);

 private:
  struct Query {
    QueryType type;
    std::string_view data;
  };

  std::queue<std::string> buffer_;
  std::istream& input_;
  StatReader stat_reader_;

  std::string ReadLine();
  std::pair<std::string_view, std::string_view> Split(std::string_view line,
                                                      std::string_view by);
  std::string_view Crop(std::string_view str, std::string_view symbols);
  std::vector<std::string_view> Chop(std::string_view str, std::string_view by);
  Query ParseQuery(std::string_view raw_query);
  domain::StopData ParseAddStop(std::string_view str);
  domain::RouteData ParseAddRoute(std::string_view str);
  std::string_view ParseGetRouteInfo(std::string_view str);
  std::string_view ParseGetStopInfo(std::string_view str);
};

}  // namespace io