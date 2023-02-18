// #include "tests.h"

#include "transport_catalogue.h"
#include "input_reader.h"

int main() {
  // TestTC();
  // TestIR();
  
  catalogue::TransportCatalogue cat;
  io::InputReader reader(std::cin, std::cout);
  reader.ReadInput();
  reader.ProcessQueries(cat);

  return 0;
}