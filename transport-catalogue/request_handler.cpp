#include "request_handler.h"

#include <sstream>
#include <algorithm>

using namespace transport_catalogue;
using namespace request_handler;
using namespace std::literals;

namespace request_handler {

    RequestHandler::RequestHandler(TransportCatalogue &db, map_renderer::MapRenderer &renderer) : db_(db), renderer_(renderer) {}

    std::optional<RequestHandler::BusStatistic> RequestHandler::FindBusStat(const transport_catalogue::Bus &bus) const{
        BusStatistic busStat{};

        if (db_.FindBus(bus.name) == nullptr) {
            return std::nullopt;
        }

        int actual_route = 0;
        double coordinates_route = 0.0;
        std::string last_stop_name;

        auto bus_num = db_.FindBus(bus.name);
        for (const auto &stop_name: bus_num->stop_names) {
            if (!last_stop_name.empty()) {
                coordinates_route += ComputeDistance(db_.FindStop(last_stop_name)->coordinates,
                                                     db_.FindStop(stop_name)->coordinates);
                actual_route += db_.GetCalculateDistance(db_.FindStop(last_stop_name), db_.FindStop(stop_name));
            }
            last_stop_name = stop_name;
        }

        busStat.stop_count = bus_num->stop_names.size();
        busStat.unique_stop_count = db_.FindUniqueStopCount(bus.name);
        busStat.route_length = actual_route;
        busStat.curvature = (static_cast<double>(actual_route) / coordinates_route);

        return busStat;
    }

    std::optional<RequestHandler::BusStatistic> RequestHandler::GetBusStat(const transport_catalogue::Bus &bus) const {
        return FindBusStat(bus);
    }

    std::optional<RequestHandler::StopStat> RequestHandler::GetBusesStop(const transport_catalogue::Stop &stop) const {
        StopStat stopStat{};

        auto get_stop = db_.FindStop(stop.name);
        if (get_stop == nullptr) {
            return std::nullopt;
        }

        auto stop_to_bus = db_.FindBusByStop(stop.name);
        if (stop_to_bus.empty()) {
            return stopStat;
        }

        stopStat.stop_to_buses = stop_to_bus;
        return stopStat;
    }

    RequestHandler::GetCoordinateStops
    RequestHandler::GetStopsWithRoute(const std::vector<std::pair<std::string, bool>> &buses) {
        RequestHandler::GetCoordinateStops getCoordinateStops;

        std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> polyline;
        std::map<std::string, std::vector<std::pair<std::string, geo::Coordinates>>> name_route_inform;
        std::vector<std::string> sort_bus_name;
        sort_bus_name.reserve(buses.size());

        for (const auto &stop: buses) {
            const Bus *bus_num = db_.FindBus(stop.first);
            if (bus_num == nullptr) {
                continue;
            }

            std::string first_stop_name = bus_num->stop_names.front();
            geo::Coordinates get_coord = db_.GetCoordinatesByStop(first_stop_name);
            name_route_inform[stop.first].push_back({first_stop_name, get_coord});

            if (!stop.second) {
                if (bus_num->stop_names.back() != bus_num->stop_names[bus_num->stop_names.size() / 2]) {
                    auto last_stop_name = bus_num->stop_names[bus_num->stop_names.size() / 2];
                    get_coord = db_.GetCoordinatesByStop(last_stop_name);
                    name_route_inform[stop.first].push_back({last_stop_name, get_coord});
                }
            }

            for (const auto &stop_name: bus_num->stop_names) {
                sort_bus_name.push_back(stop_name);
                get_coord = db_.GetCoordinatesByStop(stop_name);
                polyline[stop.first].push_back({stop_name, get_coord});
            }
        }

        std::map<std::string, geo::Coordinates> res;
        std::vector<geo::Coordinates> coord;
        coord.reserve(buses.size());

        for (const std::string &stop_name: sort_bus_name) {
            geo::Coordinates get_coord = db_.GetCoordinatesByStop(stop_name);
            res.insert({stop_name, get_coord});
            coord.push_back(get_coord);
        }

        getCoordinateStops.polyline = polyline;
        getCoordinateStops.name_route_inform = name_route_inform;
        getCoordinateStops.name_coord = res;
        getCoordinateStops.coordinate = coord;

        return getCoordinateStops;
    }

    std::string RequestHandler::RenderMap(svg::Document &doc, const RequestHandler::GetCoordinateStops &get_inform,
                                          const map_renderer::MapRenderer::RenderSettings &renderSettings) const {
        auto first = get_inform.coordinate.begin();
        auto end = get_inform.coordinate.end();
        map_renderer::SphereProjector sphereProjector(first, end, renderSettings.width, renderSettings.height,
                                                      renderSettings.padding);
        size_t count_color = 0;

        for (const auto &render_iterator: get_inform.polyline) {
            svg::Polyline polyline;
            for (const auto &stop_geo: render_iterator.second) {
                svg::Point points = sphereProjector.operator()(stop_geo.second);
                polyline.AddPoint(points);
            }
            polyline.SetStrokeColor(renderSettings.color_palette[count_color % renderSettings.color_palette.size()]);
            polyline.SetFillColor("none"s);
            polyline.SetStrokeWidth(renderSettings.line_width);
            polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            doc.Add(polyline);
            ++count_color;
        }

        auto route_inform = get_inform.name_route_inform;
        count_color = 0;

        for (const auto &render_iterator: route_inform) {
            for (const auto &stop_geo: render_iterator.second) {

                svg::Point points = sphereProjector.operator()(stop_geo.second);
                svg::Text text_substrate;


                text_substrate.SetPosition(points);
                svg::Point point{renderSettings.bus_label_offset.first, renderSettings.bus_label_offset.second};
                text_substrate.SetOffset(point);
                text_substrate.SetFontSize(renderSettings.bus_label_font_size);
                text_substrate.SetFontFamily("Verdana"s);
                text_substrate.SetFontWeight("bold"s);
                text_substrate.SetData(render_iterator.first);

                text_substrate.SetFillColor(renderSettings.underlayer_color);
                text_substrate.SetStrokeColor(renderSettings.underlayer_color);
                text_substrate.SetStrokeWidth(renderSettings.underlayer_width);
                text_substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                text_substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                doc.Add(text_substrate);

                svg::Text text;
                text.SetPosition(points);
                text.SetOffset(point);
                text.SetFontSize(renderSettings.bus_label_font_size);
                text.SetFontFamily("Verdana"s);
                text.SetFontWeight("bold"s);
                text.SetData(render_iterator.first);
                text.SetFillColor(renderSettings.color_palette[count_color % renderSettings.color_palette.size()]);
                doc.Add(text);
            }
            ++count_color;
        }

        for (const auto &g_i: get_inform.name_coord) {
            svg::Point points = sphereProjector.operator()(g_i.second);
            svg::Circle circle;
            circle.SetCenter(points);
            circle.SetRadius(renderSettings.stop_radius);
            circle.SetFillColor("white"s);
            doc.Add(circle);
        }

        for (const auto &g_i: get_inform.name_coord) {
            svg::Point points = sphereProjector.operator()(g_i.second);
            svg::Text text_substrate_stop;
            text_substrate_stop.SetPosition(points);
            svg::Point point_stop{renderSettings.stop_label_offset.first, renderSettings.stop_label_offset.second};
            text_substrate_stop.SetOffset(point_stop);
            text_substrate_stop.SetFontSize(renderSettings.stop_label_font_size);
            text_substrate_stop.SetFontFamily("Verdana"s);
            text_substrate_stop.SetData(g_i.first);
            text_substrate_stop.SetFillColor(renderSettings.underlayer_color);
            text_substrate_stop.SetStrokeColor(renderSettings.underlayer_color);
            text_substrate_stop.SetStrokeWidth(renderSettings.underlayer_width);
            text_substrate_stop.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            text_substrate_stop.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            doc.Add(text_substrate_stop);

            svg::Text text_stop;
            text_stop.SetPosition(points);
            text_stop.SetOffset(point_stop);
            text_stop.SetFontSize(renderSettings.stop_label_font_size);
            text_stop.SetFontFamily("Verdana"s);
            text_stop.SetData(g_i.first);
            text_stop.SetFillColor("black"s);
            doc.Add(text_stop);
        }

        std::ostringstream out;
        renderer_.Render(doc, out);

        return out.str();
    }
}// namespace request_handler