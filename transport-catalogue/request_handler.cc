#include "request_handler.h"

#include <sstream>
#include <string>
#include <utility>

#include "domain.h"
#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace rhandler {

using namespace std::literals;
using namespace jreader;
using namespace catalogue;
using namespace domain;
using namespace json;

json::Dict RequestHandler::ErrorMessage(int id) const {
  json::Dict err_map;
  err_map["request_id"s] = id;
  err_map["error_message"s] = std::move("not found"s);
  return err_map;
}

void RequestHandler::ProcessStatRequests(const TransportCatalogue& c,
                                         mrenderer::MapRenderer& mr,
                                         const json::Array& stat_reqs) {
  for (const auto& req : stat_reqs) {
    const auto& r_map = req.AsMap();
    int id = r_map.at("id"s).AsInt();
    if (r_map.at("type"s).AsString() == "Stop"s) {
      const auto stop = c.GetStopInfo(r_map.at("name"s).AsString());
      if (stop.is_found) {
        json::Dict ans_map;
        ans_map["request_id"s] = id;
        json::Array ans_arr;
        for (const auto& route : stop.routes) {
          ans_arr.emplace_back(Node{std::string{route}});
        }
        ans_map["buses"s] = std::move(ans_arr);
        out_.push_back(std::move(ans_map));
      } else {
        out_.push_back(std::move(ErrorMessage(id)));
      }
    }
    if (r_map.at("type"s).AsString() == "Bus"s) {
      const auto route = c.GetRouteInfo(r_map.at("name"s).AsString());
      if (route.is_found) {
        json::Dict ans_map;
        ans_map["request_id"s] = r_map.at("id"s).AsInt();
        ans_map["stop_count"s] = static_cast<int>(route.count);
        ans_map["unique_stop_count"s] = static_cast<int>(route.unique);
        ans_map["route_length"s] = static_cast<int>(route.length);
        ans_map["curvature"s] = route.curvature;
        out_.push_back(std::move(ans_map));
      } else {
        out_.push_back(std::move(ErrorMessage(id)));
      }
    }
    if (r_map.at("type"s).AsString() == "Map"s) {
      json::Dict ans_map;
      ans_map["request_id"s] = r_map.at("id"s).AsInt();
      std::ostringstream os;
      mr.RenderMap(c, os);
      ans_map["map"s] = std::move(os.str());
      out_.push_back(std::move(ans_map));
    }
  }
}
void RequestHandler::PrintRequests(std::ostream& out) {
  Print(Document{out_}, out);
}

}  // namespace rhandler