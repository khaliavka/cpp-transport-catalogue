#include "serialization.h"

#include <transport_catalogue.pb.h>

#include <fstream>
#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace serialization {

Serializer::Serializer(SerializationSettings s) : settings_(std::move(s)) {}

const serialize_proto::TransportCatalogue& Serializer::GetCatalogueProto()
    const {
  return const_cast<Serializer*>(this)->GetCatalogueProto();
}

serialize_proto::TransportCatalogue& Serializer::GetCatalogueProto() {
  return catalogue_proto_;
}

bool Serializer::Write() const {
  std::ofstream out_file(settings_.file, std::ios::binary);
  if (!out_file) {
    return false;
  }
  return catalogue_proto_.SerializeToOstream(&out_file);
}

bool Serializer::Read() {
  std::ifstream in_file(settings_.file, std::ios::binary);
  if (!in_file) {
    return false;
  }
  return catalogue_proto_.ParseFromIstream(&in_file);
}

bool MakeBase(std::istream& input) {
  using namespace catalogue;
  using namespace json_reader;
  using namespace map_renderer;
  JSONreader json_reader(json::Load(input));
  Serializer serializer(std::move(json_reader.GetSerializationSettings()));
  TransportCatalogue transport_catalogue;
  json_reader.ProcessBaseRequests(transport_catalogue);
  transport_catalogue.Save(serializer.GetCatalogueProto());
  map_renderer::MapRenderer map_renderer(
      std::move(json_reader.GetRenderSettings()));
  map_renderer.Save(serializer.GetCatalogueProto());
  return serializer.Write();
}

bool ProcessRequests(std::istream& input, std::ostream& output) {
  using namespace catalogue;
  using namespace json_reader;
  using namespace map_renderer;
  using namespace request_handler;
  using namespace transport_router;
  JSONreader json_reader(json::Load(input));
  Serializer serializer(std::move(json_reader.GetSerializationSettings()));
  if (!serializer.Read()) {
    return false;
  }
  TransportCatalogue transport_catalogue;
  transport_catalogue.Load(serializer.GetCatalogueProto());
  MapRenderer map_renderer;
  map_renderer.Load(serializer.GetCatalogueProto());
  RequestHandler request_handler;
  request_handler.ProcessStatRequestsLite(transport_catalogue, map_renderer,
                                          json_reader.GetStatRequests());
  request_handler.PrintRequests(output);
  return true;
}

}  // namespace serialization