#pragma once

#include "domain.h"
#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace request_handler {

class RequestHandler {
 public:
  void ProcessStatRequests(
      const catalogue::TransportCatalogue& cat,
      const transport_router::TransportRouter& transport_router,
      map_renderer::MapRenderer& mr, const json::Node& stat_reqs);
  void ProcessStatRequestsLite(const catalogue::TransportCatalogue& cat,
                               map_renderer::MapRenderer& mr,
                               const json::Node& stat_requests);
  void PrintRequests(std::ostream& out) const;

 private:
  json::Node ProcessStopRequest(const catalogue::TransportCatalogue& cat,
                                const json::Node& request) const;
  json::Node ProcessBusRequest(const catalogue::TransportCatalogue& cat,
                               const json::Node& request) const;
  json::Node ProcessRouteRequest(
      const transport_router::TransportRouter& transport_router,
      const json::Node& request) const;
  json::Node ProcessMapRequest(const catalogue::TransportCatalogue& cat,
                               map_renderer::MapRenderer& mr,
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