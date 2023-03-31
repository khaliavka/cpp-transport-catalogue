#include <iomanip>
#include <iostream>

#include "domain.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

namespace io {

void StatReader::PrintBusInfo(const catalogue::BusInfo& b) {
  using namespace std;
  output_ << setprecision(6) << "Bus "s << b.name << ": "s;
  if (b.stop_count == 0) {
    output_ << "not found"s;
  } else {
    output_ << b.stop_count << " stops on route, "s << b.unique_stop_count << " unique stops, "s
            << static_cast<double>(b.route_length) << " route length, "s
            << b.curvature << " curvature"s;
  }
  output_ << endl;
}

void StatReader::PrintStopInfo(const catalogue::StopInfo& s) {
  using namespace std;
  output_ << "Stop "s << s.name << ": "s;
  if (!s.is_found) {
    output_ << "not found"s;
  } else if (s.buses.size() == 0) {
    output_ << "no buses"s;
  } else {
    output_ << "buses";
    for (auto& name : s.buses) {
      output_ << " "s << name;
    }
  }
  output_ << endl;
}

}