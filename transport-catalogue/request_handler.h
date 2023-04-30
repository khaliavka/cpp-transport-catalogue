#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace request_handler {

class RequestHandler {

  using TrCat = catalogue::TransportCatalogue;
  using MapRend = map_renderer::MapRenderer;
  using TrRouter = transport_router::TransportRouter;
  using Wait = transport_router::Wait;
  using Bus = transport_router::Bus;

 public:
  void ProcessStatRequests(const TrCat& cat, const TrRouter& tr_router,
                           MapRend& mr, const json::Node& stat_reqs);

  void ProcessStatRequestsLite(const TrCat& cat, MapRend& mr,
                               const json::Node& stat_requests);

  void PrintRequests(std::ostream& out) const;

 private:
  json::Node ProcessStopRequest(const TrCat& cat,
                                const json::Node& request) const;

  json::Node ProcessBusRequest(const TrCat& cat,
                               const json::Node& request) const;

  json::Node ProcessRouteRequest(const TrRouter& tr_router,
                                 const json::Node& request) const;

  json::Node ProcessMapRequest(const TrCat& cat, MapRend& mr,
                               const json::Node& request) const;

  json::Node ErrorMessage(int id) const;

  json::Node out_;
};

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего
 * логику, которую не хотелось бы помещать ни в transport_catalogue, ни в json
 * reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика
 * запросов. Вы можете реализовать обработку запросов способом, который удобнее
 * вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON
// reader-а с другими подсистемами приложения. См. паттерн проектирования Фасад:
// https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
/*
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer&
renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view&
stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и
"Визуализатор Карты" const TransportCatalogue& db_; const renderer::MapRenderer&
renderer_;
};
*/

}  // namespace request_handler