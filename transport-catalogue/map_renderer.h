#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "geo.h"
#include "json.h"
#include "serialization.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace map_renderer {

using namespace std::literals;

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

class MapRenderer {
 public:
  using TrCat = catalogue::TransportCatalogue;

  MapRenderer(RenderSettings rs);
  void RenderMap(const TrCat& cat, std::ostream& out);
  
  friend class serialization::Saver;
  friend class serialization::Loader;
  
 private:
  MapRenderer() = default;
  void MakeBusPolylines(const std::vector<domain::Bus>& buses,
                        const geo::SphereProjector& projector);

  void MakeBusLabels(const std::vector<domain::Bus>& buses,
                     const geo::SphereProjector& projector);

  template <typename T>
  void MakeStopCircles(const T& unique_stops,
                       const geo::SphereProjector& projector);

  template <typename T>
  void MakeStopLabels(const T& unique_stops,
                      const geo::SphereProjector& projector);

  svg::Document document_;
  RenderSettings settings_;
};

template <typename T>
void MapRenderer::MakeStopCircles(const T& unique_stops,
                                  const geo::SphereProjector& projector) {
  for (const auto& u : unique_stops) {
    svg::Circle c;
    c.SetCenter(std::move(projector(u.coordinates)));
    c.SetRadius(settings_.stop_radius);
    c.SetFillColor(std::move("white"s));
    document_.Add(std::move(c));
  }
}

template <typename T>
void MapRenderer::MakeStopLabels(const T& unique_stops,
                                 const geo::SphereProjector& projector) {
  for (const auto& u : unique_stops) {
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

}  // namespace map_renderer