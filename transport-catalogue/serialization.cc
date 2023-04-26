#include "serialization.h"

#include <fstream>
#include <iostream>

#include "json_reader.h"

namespace serialization {

void MakeBase(std::istream& input) {
  using namespace catalogue;
  using namespace json_reader;
  JSONreader json_reader(json::Load(input));
  auto serial_settings = json_reader.GetSerializationSettings();
  std::ofstream out_file(std::string{serial_settings.file}, std::ios::binary);
  if (out_file) {
    TransportCatalogue transport_catalogue;
    json_reader.ProcessBaseRequests(transport_catalogue);
    transport_catalogue.SaveTo(out_file);
  }
  out_file.close();
}

void ProcessRequests(std::istream& input, std::ostream& output) {
  using namespace catalogue;
  using namespace json_reader;
  JSONreader json_reader(json::Load(input));
  auto serial_settings = json_reader.GetSerializationSettings();
  std::ifstream in_file(std::string{serial_settings.file}, std::ios::binary);
  if (in_file) {
    TransportCatalogue transport_catalogue;
    transport_catalogue.LoadFrom(in_file);
  }
}

}  // namespace serialization