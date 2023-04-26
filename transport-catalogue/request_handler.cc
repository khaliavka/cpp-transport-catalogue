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
#include "transport_router.h"

namespace request_handler {

using namespace json_reader;
using namespace catalogue;
using namespace domain;
using namespace json;

json::Node RequestHandler::ErrorMessage(int id) const {
  using namespace std;
  return Builder{}
      .StartDict()
      .Key(move("request_id"s))
      .Value(id)
      .Key(move("error_message"s))
      .Value(move("not found"s))
      .EndDict()
      .Build();
}

json::Node RequestHandler::ProcessStopRequest(const TransportCatalogue& cat,
                                              const Node& request) const {
  using namespace std;
  Builder stop{};
  int id = request.AsDict().at("id"s).AsInt();
  const auto stop_info =
      cat.GetStopInfo(request.AsDict().at("name"s).AsString());
  if (stop_info.is_found) {
    stop.StartDict()
        .Key(move("request_id"s))
        .Value(id)
        .Key(move("buses"s))
        .StartArray();
    for (const auto& bus : stop_info.buses) {
      stop.Value(move(string{bus}));
    }
    stop.EndArray().EndDict();
  } else {
    stop.Value(ErrorMessage(id).AsDict());
  }
  return stop.Build();
}

Node RequestHandler::ProcessBusRequest(const TransportCatalogue& cat,
                                       const Node& request) const {
  using namespace std;
  Builder bus{};
  int id = request.AsDict().at("id"s).AsInt();
  const auto bus_info = cat.GetBusInfo(request.AsDict().at("name"s).AsString());
  if (bus_info.is_found) {
    bus.StartDict()
        .Key(move("request_id"s))
        .Value(id)
        .Key(move("stop_count"s))
        .Value(bus_info.stop_count)
        .Key(move("unique_stop_count"s))
        .Value(bus_info.unique_stop_count)
        .Key(move("route_length"s))
        .Value(bus_info.route_length)
        .Key(move("curvature"s))
        .Value(bus_info.curvature)
        .EndDict();
  } else {
    bus.Value(ErrorMessage(id).AsDict());
  }
  return bus.Build();
}

Node RequestHandler::ProcessRouteRequest(
    const transport_router::TransportRouter& transport_router,
    const Node& request) const {
  using namespace std;
  Builder route{};
  int id = request.AsDict().at("id"s).AsInt();
  const auto& from = request.AsDict().at("from"s).AsString();
  const auto& to = request.AsDict().at("to"s).AsString();
  auto route_info = transport_router.BuildRoute(from, to);
  if (route_info) {
    route.StartDict()
        .Key("request_id"s)
        .Value(id)
        .Key("total_time"s)
        .Value(route_info->weight)
        .Key("items"s)
        .StartArray();
    int bus_wait_time = transport_router.GetBusWaitTime();
    if (route_info->elements) {
      for (const auto& element : route_info->elements.value()) {
        if (std::holds_alternative<transport_router::Wait>(element)) {
          const auto& wait_element = get<transport_router::Wait>(element);
          route.StartDict()
              .Key("type"s)
              .Value("Wait"s)
              .Key("stop_name"s)
              .Value(move(string{wait_element.stop_name}))
              .Key("time"s)
              .Value(bus_wait_time)
              .EndDict();
        } else {
          const auto& bus_element = std::get<transport_router::Bus>(element);
          route.StartDict()
              .Key("type"s)
              .Value("Bus"s)
              .Key("bus"s)
              .Value(move(string{bus_element.bus_name}))
              .Key("span_count"s)
              .Value(bus_element.span_count)
              .Key("time"s)
              .Value(bus_element.time)
              .EndDict();
        }
      }
    }
    route.EndArray().EndDict();
  } else {
    route.Value(ErrorMessage(id).AsDict());
  }
  return route.Build();
}

Node RequestHandler::ProcessMapRequest(const TransportCatalogue& cat,
                                       map_renderer::MapRenderer& mr,
                                       const Node& request) const {
  using namespace std;
  ostringstream os;
  mr.RenderMap(cat, os);
  int id = request.AsDict().at("id"s).AsInt();
  return Builder{}
      .StartDict()
      .Key(move("request_id"s))
      .Value(id)
      .Key(move("map"s))
      .Value(move(os.str()))
      .EndDict()
      .Build();
}

void RequestHandler::ProcessStatRequests(
    const TransportCatalogue& cat,
    const transport_router::TransportRouter& transport_router,
    map_renderer::MapRenderer& mr, const Node& stat_requests) {
  using namespace std;
  Builder body{};
  auto body_array = body.StartArray();
  for (const auto& request : stat_requests.AsArray()) {
    const auto& type = request.AsDict().at("type"s).AsString();
    if (type == "Stop"s) {
      body_array.Value(ProcessStopRequest(cat, request).AsDict());
    }
    if (type == "Bus"s) {
      body_array.Value(ProcessBusRequest(cat, request).AsDict());
    }
    if (type == "Route"s) {
      body_array.Value(ProcessRouteRequest(transport_router, request).AsDict());
    }
    if (type == "Map"s) {
      body_array.Value(ProcessMapRequest(cat, mr, request).AsDict());
    }
  }
  out_ = move(body_array.EndArray().Build());
}

void RequestHandler::ProcessStatRequestsLite(const TransportCatalogue& cat,
                                             const Node& stat_requests) {
  using namespace std;
  Builder body{};
  auto body_array = body.StartArray();
  for (const auto& request : stat_requests.AsArray()) {
    const auto& type = request.AsDict().at("type"s).AsString();
    if (type == "Stop"s) {
      body_array.Value(ProcessStopRequest(cat, request).AsDict());
    }
    if (type == "Bus"s) {
      body_array.Value(ProcessBusRequest(cat, request).AsDict());
    }
  }
  out_ = move(body_array.EndArray().Build());
}

void RequestHandler::PrintRequests(std::ostream& out) const {
  Print(Document{out_}, out);
}

}  // namespace request_handler