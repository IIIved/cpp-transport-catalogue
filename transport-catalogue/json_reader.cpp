#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного
 * справочника данными из JSON, а также код обработки запросов
 * к базе и формирование массива ответов в формате JSON
 */

#include <sstream>
#include <vector>


using namespace transport_catalogue;
using namespace map_renderer;
using namespace std::literals;

namespace json {

    JsonReader::JsonReader(TransportCatalogue &transportCatalogue, map_renderer::MapRenderer &mapRenderer)
            : transportCatalogue_(transportCatalogue), mapRenderer_(mapRenderer),
              requestHandler_(transportCatalogue, mapRenderer) {}

    Document JsonReader::LoadJSON(const std::string &s) {
        std::istringstream strm(s);
        return json::Load(strm);
    }

    Document JsonReader::Load(std::istream &input) {
        return json::Load(input);
    }

    std::string JsonReader::Print(const json::Node &node) {
        std::ostringstream out;
        json::Print(json::Document{node}, out);
        return out.str();
    }

    void JsonReader::ParsingBasicQueries(const json::Node &root_) {
        using type_distance_stop = std::map<std::string, std::vector<std::pair<std::string, int>>>;
        using type_bus_routes = std::map<std::string, std::vector<std::string>>;

        type_distance_stop distance_stop;
        type_bus_routes bus_routes;

        std::map<std::string, Node> data_root_ = root_.AsMap();
        json::Node base_req = data_root_["base_requests"s];
        for (const auto &b_r_: base_req.AsArray()) {
            auto key = b_r_.AsMap();
            auto type = key["type"s];

            /////////////////STOP
            {
                if (type == "Stop"s) {
                    std::string name = key["name"s].AsString();
                    double latitude = key["latitude"s].AsDouble();
                    double longitude = key["longitude"s].AsDouble();

                    geo::Coordinates coordinates{latitude, longitude};
                    transportCatalogue_.AddStop(name, coordinates);

                    std::map<std::string, Node> road_distances = key["road_distances"s].AsMap();
                    for (const auto &r_d_: road_distances) {
                        distance_stop[name].push_back({r_d_.first, r_d_.second.AsInt()});
                    }
                }
            }
            /////////////////BUS
            {
                if (type == "Bus"s) {
                    std::string name = key["name"s].AsString();
                    std::vector stops = key["stops"s].AsArray();

                    //Если маршрут не является кольцевым - добавляем остановки
                    bool is_roundtrip = key["is_roundtrip"s].AsBool();
                    if (!is_roundtrip) {
                        int size;
                        size = static_cast<int>(stops.size()) - 2;
                        while (size >= 0) {
                            stops.push_back(stops[size]);
                            size--;
                        }
                    }

                    for (const Node &stop: stops) {
                        bus_routes[name].push_back(stop.AsString());
                    }

                    buses_route.push_back({name, is_roundtrip});
                }
            }
        }
        //AddBusRoute(добавляем автобусный маршрут)
        for (const auto&[bus_number, stop_names]: bus_routes) {
            Bus bus{bus_number, stop_names};
            transportCatalogue_.AddBusRoute(bus);
        }
        //AddDistance(добавляем расстояние)
        for (const auto&[stop_name, distances_to_stop]: distance_stop) {
            for (const auto&[name, distance]: distances_to_stop) {
                transportCatalogue_.AddDistance(stop_name, name, distance);
            }
        }
    }
    void JsonReader::GetStatRequestsTypeBus(const json::Node &root_,
                                            const map_renderer::MapRenderer::RenderSettings &renderSettings,
                                            Array &array_request, std::map<std::string, json::Node> key,
                                            int request_id) {

        std::map<std::string, json::Node> data_root_ = root_.AsMap();
        auto stat_requests = data_root_["stat_requests"s];
        std::string name = key["name"s].AsString();
        Bus bus;
        bus.name = name;
        auto getBusStat = requestHandler_.GetBusStat(bus);

        if (getBusStat != std::nullopt) {
            json::Node dict_node_bus{json::Dict{{"request_id"s,        request_id},
                                                {"curvature"s,         getBusStat->curvature},
                                                {"route_length"s,      getBusStat->route_length},
                                                {"stop_count"s,        getBusStat->stop_count},
                                                {"unique_stop_count"s, getBusStat->unique_stop_count}}};
            array_request.push_back(dict_node_bus);
        }

    }

    void JsonReader::GetStatRequestsTypeStop(const json::Node &root_,
                                             const map_renderer::MapRenderer::RenderSettings &renderSettings,
                                             Array &array_request, std::map<std::string, json::Node> key,
                                             int request_id) {
        std::string name = key["name"s].AsString();
        Stop stop;
        stop.name = name;
        auto getStopStat = requestHandler_.GetBusesStop(stop);

        if (getStopStat != std::nullopt) {
            std::set<std::string> stops = getStopStat->stop_to_buses;
            json::Array stop_array;
            for (const auto &s: stops) {
                stop_array.push_back(s);
            }
            json::Node dict_node_stop{json::Dict{{"request_id"s, request_id},
                                                 {"buses"s,      stop_array}}};
            array_request.push_back(dict_node_stop);
        }

    }

    void JsonReader::GetStatRequestsTypeMap(const json::Node &root_,
                                            const map_renderer::MapRenderer::RenderSettings &renderSettings,
                                            Array &array_request, std::map<std::string, json::Node> key,
                                            int request_id) {
        svg::Document doc;
        request_handler::RequestHandler::GetCoordinateStops route_inf = requestHandler_.GetStopsWithRoute(
                buses_route);
        std::string render_str = requestHandler_.RenderMap(doc, route_inf, renderSettings);
        json::Node dict_map{json::Dict{{"request_id"s, request_id},
                                       {"map"s,        render_str}}};
        array_request.push_back(dict_map);
    }

    json::Array
    JsonReader::GetStatRequests(const json::Node &root_, const MapRenderer::RenderSettings &renderSettings) {
        Array array_request;
        std::map<std::string, json::Node> data_root_ = root_.AsMap();
        auto stat_requests = data_root_["stat_requests"s];

        for (const json::Node &stat_req_: stat_requests.AsArray()) {
            std::map<std::string, json::Node> key = stat_req_.AsMap();
            int request_id = key["id"s].AsInt();
            json::Node type = key["type"s];

            if (type == "Bus"s) {
                GetStatRequestsTypeBus(root_, renderSettings, array_request, key, request_id);
            } else {
                json::Node dict_node_bus{json::Dict{{"request_id"s,    request_id},
                                                    {"error_message"s, "not found"s}}};
                array_request.push_back(dict_node_bus);
            }

            if (type == "Stop"s) {
                GetStatRequestsTypeStop(root_, renderSettings, array_request, key, request_id);
            } else {
                json::Node dict_node_stop{json::Dict{{"request_id"s,    request_id},
                                                     {"error_message"s, "not found"s}}};
                array_request.push_back(dict_node_stop);
            }

            if (type == "Map"s) {
                GetStatRequestsTypeMap(root_, renderSettings, array_request, key, request_id);
            }

            return array_request;

        }

    }

    std::optional<MapRenderer::RenderSettings> JsonReader::ParseRenderSettingsColor(const json::Node &root_,std::map<std::string, json::Node> key, svg::Color underlayer_color){
        std::vector<json::Node> u_c = key["underlayer_color"s].AsArray();
        if (u_c.size() == 3) {
            svg::Rgb rgb;
            rgb.red = u_c[0].AsInt();
            rgb.green = u_c[1].AsInt();
            rgb.blue = u_c[2].AsInt();
            underlayer_color = rgb;
        } else if (u_c.size() == 4) {
            svg::Rgba rgba;
            rgba.red = u_c[0].AsInt();
            rgba.green = u_c[1].AsInt();
            rgba.blue = u_c[2].AsInt();
            rgba.opacity = u_c[3].AsDouble();
            underlayer_color = rgba;
        }
    }


    std::optional<MapRenderer::RenderSettings> JsonReader::ParseRenderSettings(const json::Node &root_) {
        MapRenderer::RenderSettings renderSettings;

        std::map<std::string, json::Node> data_root_ = root_.AsMap();
        auto render_settings = data_root_["render_settings"s];
        if (render_settings == nullptr) {
            return std::nullopt;
        }
        std::map<std::string, json::Node> key = render_settings.AsMap();

        renderSettings.width = key["width"s].AsDouble();
        renderSettings.height = key["height"s].AsDouble();
        renderSettings.padding = key["padding"s].AsDouble();
        renderSettings.stop_radius = key["stop_radius"s].AsDouble();
        renderSettings.line_width = key["line_width"s].AsDouble();

        renderSettings.bus_label_font_size = key["bus_label_font_size"s].AsInt();
        renderSettings.bus_label_offset.first = key["bus_label_offset"s].AsArray()[0].AsDouble();
        renderSettings.bus_label_offset.second = key["bus_label_offset"s].AsArray()[1].AsDouble();

        renderSettings.stop_label_font_size = key["stop_label_font_size"s].AsInt();
        renderSettings.stop_label_offset.first = key["stop_label_offset"s].AsArray()[0].AsDouble();
        renderSettings.stop_label_offset.second = key["stop_label_offset"s].AsArray()[1].AsDouble();

        svg::Color underlayer_color;
        if (key["underlayer_color"s].IsArray()) {
            ParseRenderSettingsColor(root_,key, underlayer_color);

        } else {
            underlayer_color = key["underlayer_color"s].AsString();
        }
        renderSettings.underlayer_color = underlayer_color;
        renderSettings.underlayer_width = key["underlayer_width"s].AsDouble();

        std::vector<svg::Color> color_plate;
        for (const json::Node &c_p: key["color_palette"s].AsArray()) {
            if (c_p.IsArray()) {
                if (c_p.AsArray().size() == 3) {
                    svg::Rgb rgb;
                    rgb.red = c_p.AsArray()[0].AsInt();
                    rgb.green = c_p.AsArray()[1].AsInt();
                    rgb.blue = c_p.AsArray()[2].AsInt();
                    color_plate.push_back(rgb);
                } else if (c_p.AsArray().size() == 4) {
                    svg::Rgba rgba;
                    rgba.red = c_p.AsArray()[0].AsInt();
                    rgba.green = c_p.AsArray()[1].AsInt();
                    rgba.blue = c_p.AsArray()[2].AsInt();
                    rgba.opacity = c_p.AsArray()[3].AsDouble();
                    color_plate.push_back(rgba);
                }
            } else {
                color_plate.push_back(c_p.AsString());
            }
        }

        renderSettings.color_palette = color_plate;
        return renderSettings;
    }
}//namespace json