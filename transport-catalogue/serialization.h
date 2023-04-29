#pragma once

#include <transport_catalogue.pb.h>

#include <iostream>

#include "svg.h"

namespace catalogue {
class TransportCatalogue;
}

namespace map_renderer {
class MapRenderer;
}

namespace transport_router {
class TransportRouter;
}

namespace graph {
  template <typename Weight>
  class DirectedWeightedGraph;
}

namespace serialization {

using TrCat = catalogue::TransportCatalogue;
using MapRend = map_renderer::MapRenderer;
using Graph = graph::DirectedWeightedGraph<double>;
using TrRouter = transport_router::TransportRouter;
using TrCatProto = protobuf::TransportCatalogue;
using TrRoutProto = protobuf::TransportRouter;

struct SerSettings {
  std::string file;
};

class Saver {
 public:
  Saver(SerSettings s, const TrCat& tc, const TrRouter& tr);
  void SaveTrCat();
  void SaveTrRouter();
  void SaveMapRend(const MapRend& mr);
  bool Write() const;

 private:
  void SaveVertexMap(TrRoutProto* router_proto);
  void SaveEdgeMap(TrRoutProto* router_proto);
  void SaveGraph(TrRoutProto* router_proto);
  void SaveRouter(TrRoutProto* router_proto);
  void SaveColor(protobuf::Color* color_proto, const svg::Color& color) const;

  const TrCat& cat_;
  const TrRouter& tr_router_;
  TrCatProto base_proto_;
  SerSettings settings_;
};

class Loader {
 public:
  Loader(SerSettings s);
  TrCat LoadTrCat() const;
  TrRouter LoadTrRouter(const TrCat& tr_cat) const;
  MapRend LoadMapRend() const;
  bool Read();

 private:
  Graph LoadGraph(const TrRoutProto& router_proto) const;
  void LoadLibRouter(TrRouter& tr_router) const;
  void LoadRoutingSettings(TrRouter& tr_router) const;
  void LoadVertexMap(const TrCat& tr_cat, TrRouter& tr_router) const;
  void LoadEdgeMap(const TrCat& tr_cat, TrRouter& tr_router) const;
  svg::Color LoadColor(const protobuf::Color& color_proto) const;

  TrCatProto base_proto_;
  SerSettings settings_;
};

bool MakeBase(std::istream& input);
bool ProcessRequests(std::istream& input, std::ostream& output);

}  // namespace serialization