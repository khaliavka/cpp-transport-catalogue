// #include "tests.h"

#include "transport_catalogue.h"
// #include "input_reader.h"
#include "json_reader.h"
#include "request_handler.h"

int main() {
  // TestTC();
  // TestIR();
  // TestJR();
  // catalogue::TransportCatalogue cat;
  // io::InputReader reader(std::cin, std::cout);
  // reader.ReadInput();
  // reader.ProcessQueries(cat);
  using namespace std;
  using namespace jreader;
  using namespace catalogue;
  using namespace reqhandler;
  TransportCatalogue c;
  JSONreader jr(json::Load(cin));
  jr.ProcessBaseRequests(c);
  RequestHandler rh;
  rh.ProcessStatRequests(c, jr.GetStatRequests());
  rh.PrintRequests(cout);
  return 0;
}