#pragma once

#include "json.h"

namespace transport_router {

class TransportRouter {
 public:
  void SetRoutingSettings(const json::Node& routing_settings);
 private:
  struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
  };
  RoutingSettings routing_settings_;
};

}  // namespace transport_router
