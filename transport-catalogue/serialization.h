#pragma once

#include <string_view>
#include <transport_catalogue.pb.h>

namespace serialization {

struct SerializationSettings {
  std::string file;
};

class Serializer {
 public:
  Serializer(SerializationSettings s);
  const serialize_proto::TransportCatalogue& GetCatalogueProto() const;
  serialize_proto::TransportCatalogue& GetCatalogueProto();
  bool Write() const;
  bool Read();
  
 private:
  serialize_proto::TransportCatalogue catalogue_proto_;
  SerializationSettings settings_;
};

bool MakeBase(std::istream& input);
bool ProcessRequests(std::istream& input, std::ostream& output);

}  // namespace serialization