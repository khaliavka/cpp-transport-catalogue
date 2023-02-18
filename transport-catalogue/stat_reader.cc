#include "stat_reader.h"

#include <iomanip>
#include <iostream>

#include "transport_catalogue.h"

namespace io {

void StatReader::PrintRouteInfo(const catalogue::RouteInfo& r) {
  using namespace std;
  output_ << setprecision(6) << "Bus "s << r.name << ": "s;
  if (r.stops == 0) {
    output_ << "not found"s;
  } else {
    output_ << r.stops << " stops on route, "s << r.unique << " unique stops, "s
            << static_cast<double>(r.length) << " route length, "s
            << r.curvature << " curvature"s;
  }
  output_ << endl;
}

void StatReader::PrintStopInfo(const catalogue::StopInfo& s) {
  using namespace std;
  output_ << "Stop "s << s.name << ": "s;
  if (!s.is_found) {
    output_ << "not found"s;
  } else if (s.routes.size() == 0) {
    output_ << "no buses"s;
  } else {
    output_ << "buses";
    for (auto& name : s.routes) {
      output_ << " "s << name;
    }
  }
  output_ << endl;
}

}