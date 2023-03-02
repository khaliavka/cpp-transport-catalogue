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

  struct Offset {
    double dx;
    double dy;
  };

  struct RenderSettings {
    double width;
    double height;

    double padding;

    double line_width;
    double stop_radius;

    int bus_label_font_size;
    Offset bus_label_offset;

    int stop_label_font_size;
    Offset stop_label_offset;

    svg::Color underlayer_color;
    double underlayer_width;

    Palette color_palette;
  };

  // RenderSettings settings_{600,     400,
  //                          50,      14,
  //                          5,      20,
  //                          {7, 15}, 20,
  //                          {7, -3}, {255, 255, 255, 0.85},
  //                          3,       {"green"s, svg::Rgb{255, 160, 0}, "red"s}};
  svg::Document document_;
  RenderSettings settings_;
};

}  // namespace mrenderer