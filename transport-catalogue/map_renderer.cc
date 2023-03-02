#include "map_renderer.h"

#include <cassert>
#include <iostream>
#include <string>

#include "geo.h"
#include "json.h"
#include "svg.h"

namespace mrenderer {

using namespace std::literals;

void MapRenderer::RenderMap(const catalogue::TransportCatalogue& cat,
                            std::ostream& out) {
  auto routes = cat.GetRoutes();
  std::sort(routes.begin(), routes.end());
  std::vector<geo::Coordinates> sp_init;
  std::vector<std::vector<geo::Coordinates>> all_pack;
  for (const auto r : routes) {
    std::vector<geo::Coordinates> route_pack;
    for (const auto s : cat.GetStopsForRoute(r)) {
      auto geo_coords = cat.GetCoordinates(s);
      sp_init.push_back(geo_coords);
      route_pack.push_back(geo_coords);
    }
    if (!route_pack.empty()) {
      all_pack.push_back(std::move(route_pack));
    }
  }
  geo::SphereProjector projector(sp_init.cbegin(), sp_init.cend(),
                                 settings_.width, settings_.height,
                                 settings_.padding);

  std::vector<svg::Polyline> p_lines;
  const auto pal_sz = settings_.color_palette.size();
  for (size_t i = 0; i < all_pack.size(); ++i) {
    svg::Polyline p;
    for (const auto& coords : all_pack[i]) {
      p.AddPoint(std::move(projector(coords)));
    }
    p.SetFillColor(svg::NoneColor);
    p.SetStrokeColor(settings_.color_palette[i % pal_sz]);
    p.SetStrokeWidth(settings_.line_width);
    p.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    p.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    document_.Add(p);
  }
  document_.Render(out);
  return;
}

void MapRenderer::SetRenderSettings(const json::Dict& s) {
  settings_.width = s.at("width"s).AsDouble();
  settings_.height = s.at("height"s).AsDouble();
  settings_.padding = s.at("padding"s).AsDouble();
  settings_.line_width = s.at("line_width"s).AsDouble();
  settings_.stop_radius = s.at("stop_radius"s).AsDouble();
  settings_.bus_label_font_size = s.at("bus_label_font_size"s).AsInt();

  const auto& blo_arr = s.at("bus_label_offset"s).AsArray();
  settings_.bus_label_offset = {blo_arr[0].AsDouble(), blo_arr[1].AsDouble()};

  settings_.stop_label_font_size = s.at("stop_label_font_size"s).AsInt();

  const auto& slo_arr = s.at("stop_label_offset"s).AsArray();
  settings_.stop_label_offset = {slo_arr[0].AsDouble(), slo_arr[1].AsDouble()};

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
  return;
}

}  // namespace mrenderer