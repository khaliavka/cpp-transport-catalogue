#include <cassert>
#include <iostream>
#include <string>

#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

namespace tests {

void TestTC() {
  using namespace std::literals;
  using namespace catalogue;
  TransportCatalogue cat;
  cat.AddStop("Tolstopaltsevo"s, {55.611087, 37.208290}, true);
  cat.AddStop("Marushkino"s, {55.595884, 37.209755}, true);
  cat.AddStop("Rasskazovka"s, {55.632761, 37.333324}, true);
  cat.AddStop("Universam"s, {55.587655, 37.645687}, true);
  cat.AddStop("Rasskazovka"s, {777.888, 888.777}, true);

  cat.AddRoute("256"s, {"Biryulyovo Zapadnoye"s, "Biryusinka", "Universam"s,
                        "Biryulyovo Tovarnaya"s, "Biryulyovo Passazhirskaya"s,
                        "Biryulyovo Zapadnoye"s});

  cat.AddStop("Biryulyovo Zapadnoye"s, {55.574371, 37.651700}, true);
  cat.AddStop("Biryusinka"s, {55.581065, 37.648390}, true);
  cat.AddStop("Biryulyovo Tovarnaya"s, {55.592028, 37.653656}, true);
  cat.AddStop("Biryulyovo Passazhirskaya"s, {55.580999, 37.659164}, true);
  cat.AddRoute("255"s, {"Biryulyovo Zapadnoye"s, "Biryusinka", "Universam"s,
                        "Biryulyovo Tovarnaya"s, "Biryulyovo Zapadnoye"s});
  cat.AddRoute("254"s, {"Biryulyovo Zapadnoye"s, "Biryusinka", "Universam"s,
                        "Biryulyovo Tovarnaya"s, "Biryulyovo Passazhirskaya"s});
  // const RouteInfo r_256 = cat.GetRouteInfo("256"s);
  // const StopInfo s_i = cat.GetStopInfo("Universam"s);
  return;
}

void TestIR() {
  catalogue::TransportCatalogue cat;
  io::InputReader reader(std::cin, std::cout);
  reader.ReadInput();
  reader.ProcessQueries(cat);
  return;
}

}