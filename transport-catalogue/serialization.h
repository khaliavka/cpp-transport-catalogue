#pragma once

#include <string_view>

namespace serialization {

struct SerializationSettings {
  std::string_view file;
};

void MakeBase(std::istream& input);
void ProcessRequests(std::istream& input, std::ostream& output);

}  // namespace serialization