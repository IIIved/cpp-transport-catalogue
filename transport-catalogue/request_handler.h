#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <unordered_set>
#include <vector>
#include <memory>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "svg.h"
#include "graph.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
namespace request_handler {

    class RequestHandler {
    public:

        explicit RequestHandler(transport_catalogue::TransportCatalogue &db, renderer::MapRenderer &renderer);

        struct BusStat {
            int stop_count;
            int unique_stop_count;
            int route_length;
            double curvature;
        };

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<BusStat> GetBusStat(const transport_catalogue::Bus &bus) const;

        struct StopStat {
            std::set<std::string> stop_to_buses;
        };

        // Возвращает маршруты, проходящие через
        std::optional<StopStat> GetBusesStop(const transport_catalogue::Stop &stop) const;

        struct GetCoordinateStops {
            std::vector<geo::Coordinates> coordinate;
            std::vector<std::string> name;
            std::map<std::string, geo::Coordinates> name_coord;

            std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> polyline;
            std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> name_route_inform;
        };

        RequestHandler::GetCoordinateStops GetStopsWithRoute(const std::vector<std::pair<std::string, bool>>& stops);
        std::string RenderMap(svg::Document& doc, const RequestHandler::GetCoordinateStops& get_inform, const renderer::MapRenderer::RenderSettings &renderSettings) const;

        template <typename Weight>
        std::optional<transport_router::TransportRouter::ResponseFindRoute> FindRoute(const transport_router::TransportRouter& transportRouter,
                                                                                      const graph::Router<Weight> router,
                                                                                      std::string_view stop_from,
                                                                                      std::string_view stop_to) const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        transport_catalogue::TransportCatalogue &db_;
        renderer::MapRenderer &renderer_;
    };

    template<typename Weight>
    std::optional<transport_router::TransportRouter::ResponseFindRoute>
    RequestHandler::FindRoute(const transport_router::TransportRouter& transportRouter,
                               graph::Router<Weight> router,
                              std::string_view stop_from,
                              std::string_view stop_to) const {
        return transportRouter.FindRoute(stop_from, stop_to);
    }

}