#include "json_reader.h"

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
using namespace catalogue;
using namespace map_renderer;
using namespace transport_router;

JSONreader::JSONreader(json::Document d) : document_(std::move(d)) {}

void JSONreader::ProcessBaseRequests(TransportCatalogue& c) const {
  const auto& base_reqs =
      document_.GetRoot().AsDict().at("base_requests"s).AsArray();
  for (const auto& req : base_reqs) {
    const auto& req_as_dict = req.AsDict();
    if (req_as_dict.at("type"s).AsString() == "Stop"s) {
      StopData stop = ProcessStop(req_as_dict);
      c.AddStop(stop.name, stop.coordinates);
      for (const auto& [other_name, distance] : stop.distances) {
        c.AddDraftStop(other_name, {});
        c.SetDistance(stop.name, other_name, distance);
      }
    }
    if (req_as_dict.at("type"s).AsString() == "Bus"s) {
      BusData bus = ProcessBus(req_as_dict);
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

RenderSettings JSONreader::GetRenderSettings() const {
  const auto& s = document_.GetRoot().AsDict().at("render_settings"s).AsDict();
  RenderSettings render_settings;
  render_settings.width = s.at("width"s).AsDouble();
  render_settings.height = s.at("height"s).AsDouble();
  render_settings.padding = s.at("padding"s).AsDouble();
  render_settings.line_width = s.at("line_width"s).AsDouble();
  render_settings.stop_radius = s.at("stop_radius"s).AsDouble();
  render_settings.bus_label_font_size =
      static_cast<uint32_t>(s.at("bus_label_font_size"s).AsInt());
  const auto& blo_arr = s.at("bus_label_offset"s).AsArray();
  render_settings.bus_label_offset =
      std::move(svg::Point{blo_arr[0].AsDouble(), blo_arr[1].AsDouble()});
  render_settings.stop_label_font_size =
      static_cast<uint32_t>(s.at("stop_label_font_size"s).AsInt());
  const auto& slo_arr = s.at("stop_label_offset"s).AsArray();
  render_settings.stop_label_offset =
      std::move(svg::Point{slo_arr[0].AsDouble(), slo_arr[1].AsDouble()});
  const auto& u_color = s.at("underlayer_color"s);
  if (u_color.IsArray()) {
    const auto& clr_arr = u_color.AsArray();
    if (clr_arr.size() == 4) {
      render_settings.underlayer_color = std::move(svg::Rgba{
          static_cast<uint8_t>(clr_arr[0].AsInt()),
          static_cast<uint8_t>(clr_arr[1].AsInt()),
          static_cast<uint8_t>(clr_arr[2].AsInt()), clr_arr[3].AsDouble()});
    }
    if (clr_arr.size() == 3) {
      render_settings.underlayer_color =
          std::move(svg::Rgb{static_cast<uint8_t>(clr_arr[0].AsInt()),
                             static_cast<uint8_t>(clr_arr[1].AsInt()),
                             static_cast<uint8_t>(clr_arr[2].AsInt())});
    }
  }
  if (u_color.IsString()) {
    render_settings.underlayer_color = u_color.AsString();
  }
  render_settings.underlayer_width = s.at("underlayer_width"s).AsDouble();
  const auto& palette = s.at("color_palette"s).AsArray();
  for (const auto& color : palette) {
    if (color.IsString()) {
      render_settings.color_palette.emplace_back(color.AsString());
    }
    if (color.IsArray()) {
      const auto& clr_arr = color.AsArray();
      if (clr_arr.size() == 4) {
        render_settings.color_palette.emplace_back(svg::Rgba{
            static_cast<uint8_t>(clr_arr[0].AsInt()),
            static_cast<uint8_t>(clr_arr[1].AsInt()),
            static_cast<uint8_t>(clr_arr[2].AsInt()), clr_arr[3].AsDouble()});
      }
      if (clr_arr.size() == 3) {
        render_settings.color_palette.emplace_back(
            svg::Rgb{static_cast<uint8_t>(clr_arr[0].AsInt()),
                     static_cast<uint8_t>(clr_arr[1].AsInt()),
                     static_cast<uint8_t>(clr_arr[2].AsInt())});
      }
    }
  }
  return render_settings;
}

RoutingSettings JSONreader::GetRoutingSettings() const {
  RoutingSettings routing_settings;
  const auto& s = document_.GetRoot().AsDict().at("routing_settings"s).AsDict();
  routing_settings.bus_wait_time = s.at("bus_wait_time"s).AsInt();
  routing_settings.bus_velocity = s.at("bus_velocity"s).AsDouble();
  return routing_settings;
}

serialization::SerSettings JSONreader::GetSerSettings() const {
  const auto& ser_settings =
      document_.GetRoot().AsDict().at("serialization_settings"s).AsDict();
  return {ser_settings.at("file").AsString()};
}

}  // namespace json_reader