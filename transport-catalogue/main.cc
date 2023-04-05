#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include "geo.h"
#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"
// #include "input_reader.h"

namespace test {

// json::Document LoadJSON(const std::string& s) {
//   std::istringstream strm(s);
//   return json::Load(strm);
// }

// void TestTC() {
//   using namespace std::literals;
//   using namespace catalogue;
//   TransportCatalogue cat;
//   cat.AddStop("Tolstopaltsevo"s, {55.611087, 37.208290});
//   cat.AddStop("Marushkino"s, {55.595884, 37.209755});
//   cat.AddStop("Rasskazovka"s, {55.632761, 37.333324});
//   cat.AddStop("Universam"s, {55.587655, 37.645687});
//   cat.AddStop("Universam2"s, {77.77, 99.99});
//   cat.AddStop("Rasskazovka"s, {777.888, 888.777});

//   cat.AddRoute("256"s, {"Biryulyovo Zapadnoye"s, "Biryusinka", "Universam"s},
//   false);

//   cat.AddStop("Biryulyovo Zapadnoye"s, {55.574371, 37.651700});
//   cat.AddStop("Biryusinka"s, {55.581065, 37.648390});
//   cat.AddStop("Biryulyovo Tovarnaya"s, {55.592028, 37.653656});
//   cat.AddStop("Biryulyovo Passazhirskaya"s, {55.580999, 37.659164});
//   cat.AddRoute("255"s, {"Biryulyovo Zapadnoye"s, "Biryusinka", "Universam"s,
//                         "Biryulyovo Tovarnaya"s, "Biryulyovo Zapadnoye"s},
//                         true);
//   cat.AddRoute("254"s, {"Biryulyovo Zapadnoye"s, "Biryusinka", "Universam2"s,
//                         "Biryulyovo Tovarnaya"s, "Biryulyovo Zapadnoye"s},
//                         true);
//   [[maybe_unused]] const RouteInfo r_256 = cat.GetRouteInfo("256"s);
//   [[maybe_unused]] const StopInfo s_i = cat.GetStopInfo("Universam"s);
//   return;
// }

// void TestIR() {
//   catalogue::TransportCatalogue cat;
//   io::InputReader reader(std::cin, std::cout);
//   reader.ReadInput();
//   reader.ProcessQueries(cat);
//   return;
// }

// void TestJR() {
//   using namespace std;
//   using namespace jreader;
//   using namespace catalogue;
//   using namespace reqhandler;
//   TransportCatalogue c;
//   JSONreader jr(json::Load(cin));
//   jr.ProcessBaseRequests(c);
//   RequestHandler rh;
//   rh.ProcessStatRequests(c, jr.GetStatRequests());
//   rh.PrintRequests(cout);
//   return;
// }

// void TestMapRenderer() {
//   using namespace json_reader;
//   using namespace map_renderer;
//   using namespace request_handler;
//   using namespace catalogue;

//   JSONreader jr(json::Load(std::cin));
//   TransportCatalogue cat;
//   jr.ProcessBaseRequests(cat);
//   MapRenderer mr{jr.GetRenderSettings()};
//   mr.RenderMap(cat, std::cout);
// }

}  // namespace test

int main() {
  using namespace json_reader;
  using namespace map_renderer;
  using namespace request_handler;
  using namespace catalogue;
  using namespace transport_router;

  JSONreader json_reader(json::Load(std::cin));
  TransportCatalogue transport_catalogue;
  json_reader.ProcessBaseRequests(transport_catalogue);
  TransportRouter transport_router{json_reader.GetRoutingSettings(), transport_catalogue};
  MapRenderer map_renderer{json_reader.GetRenderSettings()};
  RequestHandler request_handler;
  request_handler.ProcessStatRequests(transport_catalogue, transport_router,
                                      map_renderer,
                                      json_reader.GetStatRequests());
  request_handler.PrintRequests(std::cout);
  return 0;
}