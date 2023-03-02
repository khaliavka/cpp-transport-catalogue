// #include "tests.h"

#include "transport_catalogue.h"
// #include "input_reader.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

int main() {
  using namespace std;
  using namespace jreader;
  using namespace mrenderer;
  using namespace catalogue;

  JSONreader jr(json::Load(cin));
  TransportCatalogue c;
  jr.ProcessBaseRequests(c);

  MapRenderer mr;
  mr.SetRenderSettings(jr.GetRenderSettings());
  mr.RenderMap(c, cout);
  return 0;
}