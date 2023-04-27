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

void MapRenderer::SaveColor(serialize_proto::Color* color_proto,
                            const svg::Color& color) const {
  if (std::holds_alternative<std::monostate>(color)) {
    color_proto->set_variant(serialize_proto::ColorVariant::COLOR_UNSPECIFIED);
  }
  if (std::holds_alternative<std::string>(color)) {
    color_proto->set_variant(serialize_proto::ColorVariant::COLOR_AS_STRING);
    color_proto->set_as_string(std::get<std::string>(color));
  }
  if (std::holds_alternative<svg::Rgb>(color)) {
    color_proto->set_variant(serialize_proto::ColorVariant::COLOR_AS_RGB);
    color_proto->mutable_as_rgb_rgba()->set_r(std::get<svg::Rgb>(color).red);
    color_proto->mutable_as_rgb_rgba()->set_g(std::get<svg::Rgb>(color).green);
    color_proto->mutable_as_rgb_rgba()->set_b(std::get<svg::Rgb>(color).blue);
  }
  if (std::holds_alternative<svg::Rgba>(color)) {
    color_proto->set_variant(serialize_proto::ColorVariant::COLOR_AS_RGBA);
    color_proto->mutable_as_rgb_rgba()->set_r(std::get<svg::Rgba>(color).red);
    color_proto->mutable_as_rgb_rgba()->set_g(std::get<svg::Rgba>(color).green);
    color_proto->mutable_as_rgb_rgba()->set_b(std::get<svg::Rgba>(color).blue);
    color_proto->mutable_as_rgb_rgba()->set_a(
        std::get<svg::Rgba>(color).opacity);
  }
}

svg::Color MapRenderer::LoadColor(
    const serialize_proto::Color& color_proto) const {
  using CV = serialize_proto::ColorVariant;
  const auto& color = color_proto.as_rgb_rgba();
  switch (color_proto.variant()) {
    case CV::COLOR_UNSPECIFIED:
      return {};
    case CV::COLOR_AS_STRING:
      return {color_proto.as_string()};
    case CV::COLOR_AS_RGB:
      return svg::Rgb{static_cast<uint8_t>(color.r()),
                      static_cast<uint8_t>(color.g()),
                      static_cast<uint8_t>(color.b())};
    case CV::COLOR_AS_RGBA:
      return svg::Rgba{
          static_cast<uint8_t>(color.r()), static_cast<uint8_t>(color.g()),
          static_cast<uint8_t>(color.b()), static_cast<double>(color.a())};
    default:
      return {};
  }
}

void MapRenderer::Save(
    serialize_proto::TransportCatalogue& catalogue_proto) const {
  auto rs_proto = catalogue_proto.mutable_render_settings();
  rs_proto->set_width(settings_.width);
  rs_proto->set_height(settings_.height);
  rs_proto->set_padding(settings_.padding);
  rs_proto->set_line_width(settings_.line_width);
  rs_proto->set_stop_radius(settings_.stop_radius);
  rs_proto->set_bus_label_font_size(settings_.bus_label_font_size);
  rs_proto->mutable_bus_label_offset()->set_x(settings_.bus_label_offset.x);
  rs_proto->mutable_bus_label_offset()->set_y(settings_.bus_label_offset.y);
  rs_proto->set_stop_label_font_size(settings_.stop_label_font_size);
  rs_proto->mutable_stop_label_offset()->set_x(settings_.stop_label_offset.x);
  rs_proto->mutable_stop_label_offset()->set_y(settings_.stop_label_offset.y);
  auto muc = rs_proto->mutable_underlayer_color();
  SaveColor(muc, settings_.underlayer_color);
  rs_proto->set_underlayer_width(settings_.underlayer_width);
  for (const auto& color : settings_.color_palette) {
    SaveColor(rs_proto->add_color_palette(), color);
  }
}

void MapRenderer::Load(
    const serialize_proto::TransportCatalogue& catalogue_proto) {
  auto rs_proto = catalogue_proto.render_settings();
  settings_.width = rs_proto.width();
  settings_.height = rs_proto.height();
  settings_.padding = rs_proto.padding();
  settings_.line_width = rs_proto.line_width();
  settings_.stop_radius = rs_proto.stop_radius();
  settings_.bus_label_font_size = rs_proto.bus_label_font_size();
  settings_.bus_label_offset.x = rs_proto.bus_label_offset().x();
  settings_.bus_label_offset.y = rs_proto.bus_label_offset().y();
  settings_.stop_label_font_size = rs_proto.stop_label_font_size();
  settings_.stop_label_offset.x = rs_proto.stop_label_offset().x();
  settings_.stop_label_offset.y = rs_proto.stop_label_offset().y();
  settings_.underlayer_color = LoadColor(rs_proto.underlayer_color());
  settings_.underlayer_width = rs_proto.underlayer_width();
  for (const auto& color : rs_proto.color_palette()) {
    settings_.color_palette.push_back(LoadColor(color));
  }
}

}  // namespace map_renderer