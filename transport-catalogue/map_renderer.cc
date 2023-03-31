#include "map_renderer.h"

#include <cassert>
#include <iostream>
#include <set>
#include <string>

#include "geo.h"
#include "json.h"
#include "svg.h"

namespace map_renderer {

using namespace std::literals;

void MapRenderer::RenderMap(const catalogue::TransportCatalogue& cat,
                            std::ostream& out) {
  auto bus_names = cat.GetBusNames();
  std::sort(bus_names.begin(), bus_names.end());
  std::vector<geo::Coordinates> sp_init;
  std::vector<domain::Bus> buses;
  struct Cmp {
    bool operator()(const domain::Stop& lhs, const domain::Stop& rhs) const {
      return lhs.name < rhs.name;
    }
  };
  std::set<domain::Stop, Cmp> unique_stops;
  for (const auto& b : bus_names) {
    domain::Bus bus;
    bus.name = b;
    for (const auto& s : cat.GetStopsForBus(b)) {
      auto geo_coords = cat.GetCoordinates(s);
      sp_init.push_back(geo_coords);
      bus.stops.emplace_back(domain::Stop{s, geo_coords});
      unique_stops.emplace(domain::Stop{s, geo_coords});
    }
    bus.is_roundtrip = cat.IsRoundTrip(b);
    if (!bus.stops.empty()) {
      buses.push_back(std::move(bus));
    }
  }
  geo::SphereProjector projector(sp_init.cbegin(), sp_init.cend(),
                                 settings_.width, settings_.height,
                                 settings_.padding);

  MakeBusPolylines(buses, projector);
  MakeBusLabels(buses, projector);
  MakeStopCircles(unique_stops, projector);
  MakeStopLabels(unique_stops, projector);

  document_.Render(out);
  return;
}

void MapRenderer::MakeBusPolylines(const std::vector<domain::Bus>& buses,
                                     const geo::SphereProjector& projector) {
  const auto pal_sz = settings_.color_palette.size();
  for (size_t i = 0; i < buses.size(); ++i) {
    svg::Polyline p;
    for (const auto& stop : buses[i].stops) {
      p.AddPoint(std::move(projector(stop.coordinates)));
    }
    p.SetFillColor(std::move(svg::NoneColor));
    p.SetStrokeColor(settings_.color_palette[i % pal_sz]);
    p.SetStrokeWidth(settings_.line_width);
    p.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    p.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    document_.Add(std::move(p));
  }
}

void MapRenderer::MakeBusLabels(const std::vector<domain::Bus>& buses,
                                  const geo::SphereProjector& projector) {
  const auto pal_sz = settings_.color_palette.size();
  for (size_t i = 0; i < buses.size(); ++i) {
    svg::Text t;
    t.SetPosition(std::move(projector(buses[i].stops.front().coordinates)));
    t.SetData(std::move(std::string{buses[i].name}));
    t.SetOffset(settings_.bus_label_offset);
    t.SetFontSize(settings_.bus_label_font_size);
    t.SetFontFamily(std::move("Verdana"s));
    t.SetFontWeight(std::move("bold"s));

    svg::Text t_pad = t;
    t_pad.SetFillColor(settings_.underlayer_color);
    t_pad.SetStrokeColor(settings_.underlayer_color);
    t_pad.SetStrokeWidth(settings_.underlayer_width);
    t_pad.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    t_pad.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    t.SetFillColor(settings_.color_palette[i % pal_sz]);

    if (buses[i].is_roundtrip) {
      document_.Add(std::move(t_pad));
      document_.Add(std::move(t));
    } else {
      document_.Add(t_pad);
      document_.Add(t);

      const auto& s = buses[i].stops;
      if (s.front().name != s[s.size() / 2].name) {
        auto second_end_pos = projector(s[s.size() / 2].coordinates);
        t_pad.SetPosition(second_end_pos);
        t.SetPosition(std::move(second_end_pos));

        document_.Add(std::move(t_pad));
        document_.Add(std::move(t));
      }
    }
  }
}

void MapRenderer::SetRenderSettings(const json::Node& settings) {
  auto& s = settings.AsDict();
  settings_.width = s.at("width"s).AsDouble();
  settings_.height = s.at("height"s).AsDouble();
  settings_.padding = s.at("padding"s).AsDouble();
  settings_.line_width = s.at("line_width"s).AsDouble();
  settings_.stop_radius = s.at("stop_radius"s).AsDouble();
  settings_.bus_label_font_size =
      static_cast<uint32_t>(s.at("bus_label_font_size"s).AsInt());
  const auto& blo_arr = s.at("bus_label_offset"s).AsArray();
  settings_.bus_label_offset =
      std::move(svg::Point{blo_arr[0].AsDouble(), blo_arr[1].AsDouble()});
  settings_.stop_label_font_size =
      static_cast<uint32_t>(s.at("stop_label_font_size"s).AsInt());
  const auto& slo_arr = s.at("stop_label_offset"s).AsArray();
  settings_.stop_label_offset =
      std::move(svg::Point{slo_arr[0].AsDouble(), slo_arr[1].AsDouble()});
  const auto& u_color = s.at("underlayer_color"s);
  if (u_color.IsArray()) {
    const auto& clr_arr = u_color.AsArray();
    if (clr_arr.size() == 4) {
      settings_.underlayer_color = std::move(svg::Rgba{
          static_cast<uint8_t>(clr_arr[0].AsInt()),
          static_cast<uint8_t>(clr_arr[1].AsInt()),
          static_cast<uint8_t>(clr_arr[2].AsInt()), clr_arr[3].AsDouble()});
    }
    if (clr_arr.size() == 3) {
      settings_.underlayer_color =
          std::move(svg::Rgb{static_cast<uint8_t>(clr_arr[0].AsInt()),
                             static_cast<uint8_t>(clr_arr[1].AsInt()),
                             static_cast<uint8_t>(clr_arr[2].AsInt())});
    }
  }
  if (u_color.IsString()) {
    settings_.underlayer_color = u_color.AsString();
  }
  settings_.underlayer_width = s.at("underlayer_width"s).AsDouble();
  const auto& palette = s.at("color_palette"s).AsArray();
  for (const auto& color : palette) {
    if (color.IsString()) {
      settings_.color_palette.emplace_back(color.AsString());
    }
    if (color.IsArray()) {
      const auto& clr_arr = color.AsArray();
      if (clr_arr.size() == 4) {
        settings_.color_palette.emplace_back(svg::Rgba{
            static_cast<uint8_t>(clr_arr[0].AsInt()),
            static_cast<uint8_t>(clr_arr[1].AsInt()),
            static_cast<uint8_t>(clr_arr[2].AsInt()), clr_arr[3].AsDouble()});
      }
      if (clr_arr.size() == 3) {
        settings_.color_palette.emplace_back(
            svg::Rgb{static_cast<uint8_t>(clr_arr[0].AsInt()),
                     static_cast<uint8_t>(clr_arr[1].AsInt()),
                     static_cast<uint8_t>(clr_arr[2].AsInt())});
      }
    }
  }
}

}  // namespace map_renderer