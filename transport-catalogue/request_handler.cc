#include "request_handler.h"

#include <sstream>
#include <string>
#include <utility>

#include "domain.h"
#include "json.h"
#include "json_builder.h"
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
  using namespace std;
  return json::Builder{}
      .StartDict()
      .Key(move("request_id"s))
      .Value(id)
      .Key(move("error_message"s))
      .Value(move("not found"s))
      .EndDict()
      .Build()
      .AsDict();
}

void RequestHandler::ProcessStatRequests(const TransportCatalogue& c,
                                         mrenderer::MapRenderer& mr,
                                         const json::Node& stat_reqs) {
  using namespace std;
  json::Builder body{};
  auto body_arr = body.StartArray();

  for (const auto& req : stat_reqs.AsArray()) {
    const auto& r_dict = req.AsDict();
    int id = r_dict.at("id"s).AsInt();
    if (r_dict.at("type"s).AsString() == "Stop"s) {
      const auto stop = c.GetStopInfo(r_dict.at("name"s).AsString());
      if (stop.is_found) {
        body_arr.StartDict()
            .Key(move("request_id"s))
            .Value(id)
            .Key(move("buses"s))
            .StartArray();
        for (const auto& r : stop.routes) {
          body_arr.Value(move(string{r}));
        }
        body_arr.EndArray().EndDict();
      } else {
        body_arr.Value(move(ErrorMessage(id)));
      }
    }
    if (r_dict.at("type"s).AsString() == "Bus"s) {
      const auto route = c.GetRouteInfo(r_dict.at("name"s).AsString());
      if (route.is_found) {
        body_arr.StartDict()
            .Key(move("request_id"s))
            .Value(id)
            .Key(move("stop_count"s))
            .Value(route.count)
            .Key(move("unique_stop_count"s))
            .Value(route.unique)
            .Key(move("route_length"s))
            .Value(route.length)
            .Key(move("curvature"s))
            .Value(route.curvature)
            .EndDict();
      } else {
        body_arr.Value(move(ErrorMessage(id)));
      }
    }
    if (r_dict.at("type"s).AsString() == "Map"s) {
      std::ostringstream os;
      mr.RenderMap(c, os);
      body_arr.StartDict()
          .Key(move("request_id"s))
          .Value(id)
          .Key(move("map"s))
          .Value(move(os.str()))
          .EndDict();
    }
  }
  out_ = body_arr.EndArray().Build();
}
void RequestHandler::PrintRequests(std::ostream& out) const {
  Print(Document{out_}, out);
}

}  // namespace rhandler