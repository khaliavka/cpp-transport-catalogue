#include "request_handler.h"

#include <string>
#include <utility>

#include "domain.h"
#include "json.h"
#include "json_reader.h"
#include "transport_catalogue.h"

namespace reqhandler {

using namespace std::literals;
using namespace jreader;
using namespace catalogue;
using namespace domain;
using namespace json;

void RequestHandler::PushError(int id) {
  json::Dict err_map;
  err_map["request_id"s] = id;
  err_map["error_message"s] = std::move("not found"s);
  out_.push_back(std::move(err_map));
}

void RequestHandler::ProcessStatRequests(const TransportCatalogue& c,
                                         const json::Array& stat_reqs) {
  for (const auto& req : stat_reqs) {
    const auto& r_map = req.AsMap();
    if (r_map.at("type"s).AsString() == "Stop"s) {
      const auto stop = c.GetStopInfo(r_map.at("name"s).AsString());
      if (stop.is_found) {
        json::Dict ans_map;
        ans_map["request_id"s] = r_map.at("id"s).AsInt();
        json::Array ans_arr;
        for (const auto& route : stop.routes) {
          ans_arr.emplace_back(Node{std::string{route}});
        }
        ans_map["buses"s] = std::move(ans_arr);
        out_.push_back(std::move(ans_map));
      } else {
        PushError(r_map.at("id"s).AsInt());
      }
    }
    if (r_map.at("type"s).AsString() == "Bus"s) {
      const auto route = c.GetRouteInfo(r_map.at("name"s).AsString());
      if (route.is_found) {
        json::Dict ans_map;
        ans_map["curvature"s] = route.curvature;
        ans_map["request_id"s] = r_map.at("id"s).AsInt();
        ans_map["route_length"s] = static_cast<int>(route.length);
        ans_map["stop_count"s] = static_cast<int>(route.count);
        ans_map["unique_stop_count"s] = static_cast<int>(route.unique);
        out_.push_back(std::move(ans_map));
      } else {
        PushError(r_map.at("id"s).AsInt());
      }
    }
  }
}
void RequestHandler::PrintRequests(std::ostream& out) {
  Print(Document{out_}, out);
}

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего
 * логику, которую не хотелось бы помещать ни в transport_catalogue, ни в json
 * reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

}  // namespace reqhandler