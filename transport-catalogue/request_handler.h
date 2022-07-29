#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <unordered_set>
#include <vector>

#include  "transport_catalogue.h"
#include  "map_renderer.h"
#include  "svg.h"

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

        explicit RequestHandler(transport_catalogue::TransportCatalogue &db, map_renderer::MapRenderer &renderer);

        struct BusStatistic {
            int stop_count = 0;
            int route_length = 0;
            int unique_stop_count = 0;
            double curvature = 0.0;
        };

        // Находит и возвращает информацию о маршруте (запрос Bus)
        std::optional<RequestHandler::BusStatistic> FindBusStat(const transport_catalogue::Bus &bus) const;
        std::optional<BusStatistic> GetBusStat(const transport_catalogue::Bus &bus) const;

        struct StopStat {
            std::set<std::string> stop_to_buses;
        };

        // Получаю маршруты автобусов, проходящие через остановки.
        std::optional<StopStat> GetBusesStop(const transport_catalogue::Stop &stop) const;

        struct GetCoordinateStops {
            std::vector<geo::Coordinates> coordinate;
            std::vector<std::string> name;
            std::map<std::string, geo::Coordinates> name_coord;

            std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> polyline;
            std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> name_route_inform;
        };

        RequestHandler::GetCoordinateStops GetStopsWithRoute(const std::vector<std::pair<std::string, bool>>& stops);
        std::string RenderMap(svg::Document& doc, const RequestHandler::GetCoordinateStops& get_inform, const map_renderer::MapRenderer::RenderSettings &renderSettings) const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        transport_catalogue::TransportCatalogue &db_;
        map_renderer::MapRenderer &renderer_;
    };

}// namespace request_handler