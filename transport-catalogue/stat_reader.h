#pragma once

#include <iostream>

#include "domain.h"
#include "transport_catalogue.h"

namespace io {

class StatReader {
 public:
  StatReader(std::ostream& out) : output_(out) {}
  void PrintRouteInfo(const domain::RouteInfo& r);
  void PrintStopInfo(const domain::StopInfo& s);

 private:
  std::ostream& output_;
};

}