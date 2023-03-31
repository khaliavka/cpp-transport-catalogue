#include "transport_router.h"
#include <string>
namespace transport_router {

  using namespace std::literals;

  void TransportRouter::SetRoutingSettings(const json::Node& routing_settings) {
    const auto& s = routing_settings.AsDict();
    routing_settings_.bus_wait_time = s.at("bus_wait_time"s).AsInt();
    routing_settings_.bus_velocity = s.at("bus_velocity"s).AsDouble();
  }
}