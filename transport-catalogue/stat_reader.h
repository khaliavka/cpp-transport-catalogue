#pragma once

#include <iostream>

#include "domain.h"
#include "transport_catalogue.h"

namespace io {

class StatReader {
 public:
  StatReader(std::ostream& out) : output_(out) {}
  void PrintBusInfo(const catalogue::BusInfo& r);
  void PrintStopInfo(const catalogue::StopInfo& s);

 private:
  std::ostream& output_;
};

}