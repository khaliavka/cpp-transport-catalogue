#include "json_reader.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace json_reader {

using namespace std::literals;
using namespace domain;

JSONreader::JSONreader(json::Document d) : document_(std::move(d)) {}

void JSONreader::ProcessBaseRequests(TrCat& c) const {
  const auto& reqs =
      document_.GetRoot().AsDict().at("base_requests"s).AsArray();
  for (const auto& req : reqs) {
    const auto& req_dict = req.AsDict();
    const auto& type = req_dict.at("type"s).AsString();
    if (type == "Stop"s) {
      StopData stop = ProcessStop(req_dict);
      c.AddStop(stop.name, stop.coordinates);
      for (const auto& [other_name, distance] : stop.distances) {
        c.AddDraftStop(other_name, {});
        c.SetDistance(stop.name, other_name, distance);
      }
    }
    if (type == "Bus"s) {
      BusData bus = ProcessBus(req_dict);
      c.AddBus(bus.name, bus.stops, bus.is_roundtrip);
    }
  }
}

StopData JSONreader::ProcessStop(const json::Dict& stop_as_dict) const {
  StopData stop;
  stop.name = stop_as_dict.at("name"s).AsString();
  stop.coordinates.lat = stop_as_dict.at("latitude"s).AsDouble();
  stop.coordinates.lng = stop_as_dict.at("longitude"s).AsDouble();
  for (const auto& [name, dist] : stop_as_dict.at("road_distances"s).AsDict()) {
    stop.distances.emplace_back(
        std::pair<std::string_view, int>{name, dist.AsInt()});
  }
  return stop;
}

BusData JSONreader::ProcessBus(const json::Dict& bus_as_dict) const {
  BusData bus;
  bus.name = bus_as_dict.at("name"s).AsString();
  for (const auto& stop : bus_as_dict.at("stops"s).AsArray()) {
    bus.stops.emplace_back(std::string_view{stop.AsString()});
  }
  bool is_roundtrip = bus_as_dict.at("is_roundtrip"s).AsBool();
  if (!is_roundtrip) {
    for (size_t i = bus.stops.size() - 1; i != 0; --i) {
      bus.stops.push_back(bus.stops[i - 1]);
    }
  }
  bus.is_roundtrip = is_roundtrip;
  return bus;
}

const json::Node& JSONreader::GetStatRequests() const {
  return document_.GetRoot().AsDict().at("stat_requests"s);
}

svg::Color JSONreader::GetColor(const json::Node& color) const {
  if (color.IsArray()) {
    const auto& clr = color.AsArray();
    if (clr.size() == 4) {
      return {svg::Rgba{static_cast<uint8_t>(clr[0].AsInt()),
                        static_cast<uint8_t>(clr[1].AsInt()),
                        static_cast<uint8_t>(clr[2].AsInt()),
                        clr[3].AsDouble()}};
    }
    if (clr.size() == 3) {
      return {svg::Rgb{static_cast<uint8_t>(clr[0].AsInt()),
                       static_cast<uint8_t>(clr[1].AsInt()),
                       static_cast<uint8_t>(clr[2].AsInt())}};
    }
  }
  if (color.IsString()) {
    return {color.AsString()};
  }
  throw std::invalid_argument("Invalid argument in GetColor() method");
}

RendSett JSONreader::GetRenderSettings() const {
  RendSett render_settings;
  const auto& s = document_.GetRoot().AsDict().at("render_settings"s).AsDict();
  render_settings.width = s.at("width"s).AsDouble();
  render_settings.height = s.at("height"s).AsDouble();
  render_settings.padding = s.at("padding"s).AsDouble();
  render_settings.line_width = s.at("line_width"s).AsDouble();
  render_settings.stop_radius = s.at("stop_radius"s).AsDouble();
  render_settings.bus_label_font_size =
      static_cast<uint32_t>(s.at("bus_label_font_size"s).AsInt());
  const auto& blo = s.at("bus_label_offset"s).AsArray();
  render_settings.bus_label_offset =
      std::move(svg::Point{blo[0].AsDouble(), blo[1].AsDouble()});
  render_settings.stop_label_font_size =
      static_cast<uint32_t>(s.at("stop_label_font_size"s).AsInt());
  const auto& slo = s.at("stop_label_offset"s).AsArray();
  render_settings.stop_label_offset =
      std::move(svg::Point{slo[0].AsDouble(), slo[1].AsDouble()});
  const auto& u_color = s.at("underlayer_color"s);
  render_settings.underlayer_color = std::move(GetColor(u_color));
  render_settings.underlayer_width = s.at("underlayer_width"s).AsDouble();
  const auto& palette = s.at("color_palette"s).AsArray();
  for (const auto& color : palette) {
    render_settings.color_palette.push_back(GetColor(color));
  }
  return render_settings;
}

RoutSett JSONreader::GetRoutingSettings() const {
  RoutSett routing_settings;
  const auto& s = document_.GetRoot().AsDict().at("routing_settings"s).AsDict();
  routing_settings.bus_wait_time = s.at("bus_wait_time"s).AsInt();
  routing_settings.bus_velocity = s.at("bus_velocity"s).AsDouble();
  return routing_settings;
}

SerSett JSONreader::GetSerSettings() const {
  const auto& ser_settings =
      document_.GetRoot().AsDict().at("serialization_settings"s).AsDict();
  return {ser_settings.at("file").AsString()};
}

}  // namespace json_reader