#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "geo.h"
#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace mrenderer {

using namespace std::literals;

class MapRenderer {
 public:
  void RenderMap(const catalogue::TransportCatalogue& cat, std::ostream& out);
  void SetRenderSettings(const json::Dict& s);

 private:
  using Palette = std::vector<svg::Color>;

  struct RenderSettings {
    double width;
    double height;

    double padding;

    double line_width;
    double stop_radius;

    uint32_t bus_label_font_size;
    svg::Point bus_label_offset;

    uint32_t stop_label_font_size;
    svg::Point stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;

    Palette color_palette;
  };

  void MakeRoutePolylines(const std::vector<domain::Route>& routes,
                          const geo::SphereProjector& projector);

  void MakeRouteLabels(const std::vector<domain::Route>& routes,
                       const geo::SphereProjector& projector);

  template <typename T>
  void MakeStopCircles(const T& unique, const geo::SphereProjector& projector);

  template <typename T>
  void MakeStopLabels(const T& unique, const geo::SphereProjector& projector);

  svg::Document document_;
  RenderSettings settings_;
};

template <typename T>
void MapRenderer::MakeStopCircles(const T& unique,
                                  const geo::SphereProjector& projector) {
  for (const auto& u : unique) {
    svg::Circle c;
    c.SetCenter(std::move(projector(u.coordinates)));
    c.SetRadius(settings_.stop_radius);
    c.SetFillColor(std::move("white"s));
    document_.Add(std::move(c));
  }
}
template <typename T>
void MapRenderer::MakeStopLabels(const T& unique,
                                 const geo::SphereProjector& projector) {
  for (const auto& u : unique) {
    svg::Text t;
    t.SetPosition(std::move(projector(u.coordinates)));
    t.SetData(std::move(std::string{u.name}));
    t.SetOffset(settings_.stop_label_offset);
    t.SetFontSize(settings_.stop_label_font_size);
    t.SetFontFamily(std::move("Verdana"s));

    svg::Text t_pad = t;
    t_pad.SetFillColor(settings_.underlayer_color);
    t_pad.SetStrokeColor(settings_.underlayer_color);
    t_pad.SetStrokeWidth(settings_.underlayer_width);
    t_pad.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    t_pad.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    t.SetFillColor(std::move(svg::Color{"black"s}));

    document_.Add(std::move(t_pad));
    document_.Add(std::move(t));
  }
}
}  // namespace mrenderer