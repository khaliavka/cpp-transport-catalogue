#include "serialization.h"

#include <transport_catalogue.pb.h>

#include <fstream>
#include <iostream>

#include "graph.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace serialization {

using JSONrr = json_reader::JSONreader;
using ReqHand = request_handler::RequestHandler;
using Wait = transport_router::Wait;
using Bus = transport_router::Bus;

Saver::Saver(SerSettings s, const TrCat& tc, const TrRouter& tr)
    : cat_(tc), tr_router_(tr), settings_(std::move(s)) {}

void Saver::SaveTrCat() {
  for (const auto& stop : cat_.stops_) {
    auto serial_stop = base_proto_.add_stop();
    serial_stop->set_id(stop.id);
    serial_stop->set_name(stop.name);
    serial_stop->mutable_coordinates()->set_lat(stop.coordinates.lat);
    serial_stop->mutable_coordinates()->set_lng(stop.coordinates.lng);
    serial_stop->set_is_consistent(stop.is_consistent);
  }
  for (const auto& bus : cat_.buses_) {
    auto serial_bus = base_proto_.add_bus();
    serial_bus->set_id(bus.id);
    serial_bus->set_name(bus.name);
    serial_bus->set_is_roundtrip(bus.is_roundtrip);
    for (auto stop : bus.stops) {
      serial_bus->add_stop_id(stop->id);
    }
  }
  for (const auto& [key, val] : cat_.distances_) {
    auto serial_dist = base_proto_.add_distance();
    serial_dist->set_key(key);
    serial_dist->set_val(val);
  }
}

void Saver::SaveMapRend(const MapRend& mr) {
  auto rs_proto = base_proto_.mutable_render_settings();
  const auto& setts = mr.settings_;
  rs_proto->set_width(setts.width);
  rs_proto->set_height(setts.height);
  rs_proto->set_padding(setts.padding);
  rs_proto->set_line_width(setts.line_width);
  rs_proto->set_stop_radius(setts.stop_radius);
  rs_proto->set_bus_label_font_size(setts.bus_label_font_size);
  rs_proto->mutable_bus_label_offset()->set_x(setts.bus_label_offset.x);
  rs_proto->mutable_bus_label_offset()->set_y(setts.bus_label_offset.y);
  rs_proto->set_stop_label_font_size(setts.stop_label_font_size);
  rs_proto->mutable_stop_label_offset()->set_x(setts.stop_label_offset.x);
  rs_proto->mutable_stop_label_offset()->set_y(setts.stop_label_offset.y);
  auto muc = rs_proto->mutable_underlayer_color();
  SaveColor(muc, setts.underlayer_color);
  rs_proto->set_underlayer_width(setts.underlayer_width);
  for (const auto& color : setts.color_palette) {
    SaveColor(rs_proto->add_color_palette(), color);
  }
}

void Saver::SaveVertexMap(TrRoutProto* router_proto) {
  for (const auto& [key, val] : tr_router_.stop_name_to_vertex_id_) {
    auto ver_map = router_proto->add_stop_name_to_vertex_id();
    ver_map->set_key(cat_.stopname_to_stop_.at(key)->id);
    ver_map->set_val(val);
  }
}

void Saver::SaveEdgeMap(TrRoutProto* router_proto) {
  const auto& edge_map = tr_router_.edge_id_to_route_element_;
  for (const auto& [key, val] : edge_map) {
    auto edge_map = router_proto->add_edge_id_to_route_element();
    edge_map->set_key(key);
    if (std::holds_alternative<Wait>(val)) {
      const auto& wait_val = std::get<Wait>(val);
      edge_map->set_stop_id(cat_.stopname_to_stop_.at(wait_val.stop_name)->id);
    }
    if (std::holds_alternative<Bus>(val)) {
      const auto& bus_val = std::get<Bus>(val);
      auto bus = edge_map->mutable_bus();
      bus->set_bus_id(cat_.busname_to_bus_.at(bus_val.bus_name)->id);
      bus->set_span_count(bus_val.span_count);
      bus->set_time(bus_val.time);
    }
  }
}

void Saver::SaveGraph(TrRoutProto* router_proto) {
  auto graph_proto = router_proto->mutable_graph();
  for (const auto& [from, to, weight] : tr_router_.graph_.edges_) {
    auto edge = graph_proto->add_edges();
    edge->set_from(from);
    edge->set_to(to);
    edge->set_weight(weight);
  }
  for (const auto& inc_list : tr_router_.graph_.incidence_lists_) {
    auto inc_list_pr = graph_proto->add_incidence_lists();
    for (const auto& edge_id : inc_list) {
      inc_list_pr->add_edge_id(edge_id);
    }
  }
}

void Saver::SaveRouter(TrRoutProto* router_proto) {
  auto router_lib_pr = router_proto->mutable_router();
  for (const auto& vec_rid : tr_router_.router_.routes_internal_data_) {
    auto rep_rep_rid = router_lib_pr->add_repreprid();
    for (const auto& rid : vec_rid) {
      auto rep_rid = rep_rep_rid->add_reprid();
      if (rid) {
        rep_rid->set_weight(rid->weight);
        if (rid->prev_edge) {
          rep_rid->set_prev_edge(*rid->prev_edge);
        }
      }
    }
  }
}

void Saver::SaveTrRouter() {
  auto router_proto = base_proto_.mutable_router();
  auto router_sett = router_proto->mutable_settings();
  router_sett->set_bus_wait_time(tr_router_.routing_settings_.bus_wait_time);
  router_sett->set_bus_velocity(tr_router_.routing_settings_.bus_velocity);
  SaveVertexMap(router_proto);
  SaveEdgeMap(router_proto);
  SaveGraph(router_proto);
  SaveRouter(router_proto);
}

bool Saver::Write() const {
  std::ofstream out_file(settings_.file, std::ios::binary);
  if (!out_file) {
    return false;
  }
  return base_proto_.SerializeToOstream(&out_file);
}

void Saver::SaveColor(protobuf::Color* color_proto,
                      const svg::Color& color) const {
  if (std::holds_alternative<std::monostate>(color)) {
    color_proto->set_variant(protobuf::ColorVariant::COLOR_UNSPECIFIED);
  }
  if (std::holds_alternative<std::string>(color)) {
    color_proto->set_variant(protobuf::ColorVariant::COLOR_AS_STRING);
    color_proto->set_as_string(std::get<std::string>(color));
  }
  if (std::holds_alternative<svg::Rgb>(color)) {
    color_proto->set_variant(protobuf::ColorVariant::COLOR_AS_RGB);
    color_proto->mutable_as_rgb_rgba()->set_r(std::get<svg::Rgb>(color).red);
    color_proto->mutable_as_rgb_rgba()->set_g(std::get<svg::Rgb>(color).green);
    color_proto->mutable_as_rgb_rgba()->set_b(std::get<svg::Rgb>(color).blue);
  }
  if (std::holds_alternative<svg::Rgba>(color)) {
    color_proto->set_variant(protobuf::ColorVariant::COLOR_AS_RGBA);
    color_proto->mutable_as_rgb_rgba()->set_r(std::get<svg::Rgba>(color).red);
    color_proto->mutable_as_rgb_rgba()->set_g(std::get<svg::Rgba>(color).green);
    color_proto->mutable_as_rgb_rgba()->set_b(std::get<svg::Rgba>(color).blue);
    color_proto->mutable_as_rgb_rgba()->set_a(
        std::get<svg::Rgba>(color).opacity);
  }
}

// -- Loader --

Loader::Loader(SerSettings s) : settings_(std::move(s)) {}

TrCat Loader::LoadTrCat() const {
  TrCat cat;
  for (const auto& s : base_proto_.stop()) {
    auto& stop = cat.stops_.emplace_back(TrCat::Stop{});
    stop.id = s.id();
    stop.name = s.name();
    stop.coordinates.lat = s.coordinates().lat();
    stop.coordinates.lng = s.coordinates().lng();
    stop.is_consistent = s.is_consistent();
    cat.stopname_to_stop_[stop.name] = &stop;
  }
  for (const auto& b : base_proto_.bus()) {
    auto& bus = cat.buses_.emplace_back(TrCat::Bus{});
    bus.id = b.id();
    bus.name = b.name();
    for (const auto& id : b.stop_id()) {
      bus.stops.push_back(&cat.stops_[id]);
      bus.unique_stops.insert(&cat.stops_[id]);
      cat.stops_[id].buses.insert(bus.name);
    }
    bus.is_roundtrip = b.is_roundtrip();
    cat.busname_to_bus_[bus.name] = &bus;
  }
  for (const auto& d : base_proto_.distance()) {
    cat.distances_[d.key()] = d.val();
  }
  return cat;
}

MapRend Loader::LoadMapRend() const {
  MapRend mr;
  const auto& rs_proto = base_proto_.render_settings();
  auto& setts = mr.settings_;
  setts.width = rs_proto.width();
  setts.height = rs_proto.height();
  setts.padding = rs_proto.padding();
  setts.line_width = rs_proto.line_width();
  setts.stop_radius = rs_proto.stop_radius();
  setts.bus_label_font_size = rs_proto.bus_label_font_size();
  setts.bus_label_offset.x = rs_proto.bus_label_offset().x();
  setts.bus_label_offset.y = rs_proto.bus_label_offset().y();
  setts.stop_label_font_size = rs_proto.stop_label_font_size();
  setts.stop_label_offset.x = rs_proto.stop_label_offset().x();
  setts.stop_label_offset.y = rs_proto.stop_label_offset().y();
  setts.underlayer_color = LoadColor(rs_proto.underlayer_color());
  setts.underlayer_width = rs_proto.underlayer_width();
  for (const auto& color : rs_proto.color_palette()) {
    setts.color_palette.push_back(LoadColor(color));
  }
  return mr;
}

Graph Loader::LoadGraph(const TrRoutProto& router_proto) const {
  auto graph_proto = router_proto.graph();
  Graph graph;
  for (const auto& edge : graph_proto.edges()) {
    graph.edges_.push_back(
        graph::Edge<double>{edge.from(), edge.to(), edge.weight()});
  }
  for (const auto& incidence_list : graph_proto.incidence_lists()) {
    graph.incidence_lists_.push_back({});
    for (const auto& edge_id : incidence_list.edge_id()) {
      graph.incidence_lists_.back().push_back(edge_id);
    }
  }
  return graph;
}

void Loader::LoadLibRouter(TrRouter& tr_router) const {
  using RID = graph::Router<double>::RouteInternalData;
  const auto& router_proto = base_proto_.router();
  auto& rid = tr_router.router_.routes_internal_data_;
  for (const auto& rep_rid_pr : router_proto.router().repreprid()) {
    rid.push_back({});
    for (const auto& rid_pr : rep_rid_pr.reprid()) {
      rid.back().push_back(std::nullopt);
      if (rid_pr.has_weight()) {
        rid.back().back().emplace(RID{rid_pr.weight(), std::nullopt});
        if (rid_pr.has_prev_edge()) {
          rid.back().back()->prev_edge.emplace(rid_pr.prev_edge());
        }
      }
    }
  }
}

void Loader::LoadRoutingSettings(TrRouter& tr_router) const {
  const auto& router_proto = base_proto_.router();
  tr_router.routing_settings_.bus_wait_time =
      router_proto.settings().bus_wait_time();
  tr_router.routing_settings_.bus_velocity =
      router_proto.settings().bus_velocity();
}

void Loader::LoadVertexMap(const TrCat& tr_cat, TrRouter& tr_router) const {
  const auto& stop_map_pr = base_proto_.router().stop_name_to_vertex_id();
  const auto& stops = tr_cat.stops_;
  auto& stop_map = tr_router.stop_name_to_vertex_id_;
  for (const auto& elem : stop_map_pr) {
    stop_map.emplace(stops[elem.key()].name, elem.val());
  }
}

void Loader::LoadEdgeMap(const TrCat& tr_cat, TrRouter& tr_router) const {
  using rec = protobuf::EdgeMap::RouteElementCase;
  const auto& edge_map_pr = base_proto_.router().edge_id_to_route_element();
  auto& edge_map = tr_router.edge_id_to_route_element_;
  for (const auto& elem : edge_map_pr) {
    switch (elem.route_element_case()) {
      case rec::kStopId:
        edge_map.emplace(elem.key(), Wait{tr_cat.stops_[elem.stop_id()].name});
        break;
      case rec::kBus:
        edge_map.emplace(elem.key(),
                         Bus{tr_cat.buses_[elem.bus().bus_id()].name,
                             elem.bus().span_count(), elem.bus().time()});
        break;
      case rec::ROUTE_ELEMENT_NOT_SET:
        throw;
    }
  }
}

TrRouter Loader::LoadTrRouter(const TrCat& tr_cat) const {
  const auto& router_proto = base_proto_.router();
  TrRouter tr_router(LoadGraph(router_proto));
  LoadVertexMap(tr_cat, tr_router);
  LoadEdgeMap(tr_cat, tr_router);
  LoadLibRouter(tr_router);
  LoadRoutingSettings(tr_router);
  return tr_router;
}

bool Loader::Read() {
  std::ifstream in_file(settings_.file, std::ios::binary);
  if (!in_file) {
    return false;
  }
  return base_proto_.ParseFromIstream(&in_file);
}

svg::Color Loader::LoadColor(const protobuf::Color& color_proto) const {
  using CV = protobuf::ColorVariant;
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

// -- functions --

bool MakeBase(std::istream& input) {
  TrCat tr_cat;
  JSONrr j_reader(json::Load(input));
  j_reader.ProcessBaseRequests(tr_cat);
  MapRend map_rend(std::move(j_reader.GetRenderSettings()));
  TrRouter tr_router{j_reader.GetRoutingSettings(), tr_cat};
  Saver saver(std::move(j_reader.GetSerSettings()), tr_cat, tr_router);
  saver.SaveTrCat();
  saver.SaveTrRouter();
  saver.SaveMapRend(map_rend);
  return saver.Write();
}

bool ProcessRequests(std::istream& input, std::ostream& output) {
  JSONrr reader(json::Load(input));
  Loader loader(std::move(reader.GetSerSettings()));
  if (!loader.Read()) {
    return false;
  }
  auto cat = loader.LoadTrCat();
  auto router = loader.LoadTrRouter(cat);
  auto renderer = loader.LoadMapRend();
  ReqHand req_hand;
  req_hand.ProcessStatRequests(cat, router, renderer, reader.GetStatRequests());
  req_hand.PrintRequests(output);
  return true;
}

}  // namespace serialization