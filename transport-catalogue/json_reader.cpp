#include "json_reader.h"

#include <sstream>
#include <vector>

using namespace json;
using namespace transport_catalogue;
using namespace renderer;
using namespace transport_router;
using namespace std::literals;
namespace json{
    JsonReader::JsonReader(TransportCatalogue &transportCatalogue, renderer::MapRenderer &mapRenderer)
            : transportCatalogue_(transportCatalogue),
              mapRenderer_(mapRenderer),
              requestHandler_(transportCatalogue, mapRenderer) {}

    json::Document JsonReader::LoadJSON(const std::string& s) {
        std::istringstream strm(s);
        return json::Load(strm);
    }

    json::Document JsonReader::Load(std::istream& input) {
        return json::Load(input);
    }

    std::string JsonReader::Print(const json::Node& node) {
        std::ostringstream out;
        json::Print(json::Document{node}, out);
        return out.str();
    }

    void JsonReader::PostRequestsInCatalogue(const json::Node& root_) {
        using type_distance_stop = std::map<std::string, std::vector<std::pair<std::string, int>>>;
        using type_bus_routes = std::map<std::string, std::vector<std::string>>;

        type_distance_stop distance_stop;
        type_bus_routes bus_routes;

        auto data_root_ = root_.AsDict();
        auto base_requests = data_root_["base_requests"s];
        for (const auto& b_r_ : base_requests.AsArray()) {
            auto key = b_r_.AsDict();
            auto type = key["type"s];
            if (type == "Stop"s) {

                std::string name = key["name"s].AsString();
                double latitude = key["latitude"s].AsDouble();
                double longitude = key["longitude"s].AsDouble();

                geo::Coordinates coordinates{latitude, longitude};
                transportCatalogue_.AddStop(name, coordinates);

                auto road_distances = key["road_distances"s].AsDict();
                for (const auto& r_d_ : road_distances) {
                    distance_stop[name].push_back({r_d_.first, r_d_.second.AsInt()});
                }
            }

            if (type == "Bus"s) {
                std::string name = key["name"s].AsString();
                std::vector stops = key["stops"s].AsArray();

                bool is_roundtrip = key["is_roundtrip"s].AsBool();   // если не кольцевой, то добавляем остановки
                if (!is_roundtrip) {
                    int size = static_cast<int>(stops.size()) - 2;
                    while (size >= 0) {
                        stops.push_back(stops[size]);
                        size--;
                    }
                }

                for (const auto& stop : stops) {
                    bus_routes[name].push_back(stop.AsString());
                }

                buses_route.push_back({name, is_roundtrip}); // TO-DO ???
            }
        }

        for (const auto& [bus_number, stop_names] : bus_routes) {
            Bus bus{bus_number, stop_names};
            transportCatalogue_.AddBusRoute(bus);
        }

        for (const auto& [stop_name, distances_to_stop] : distance_stop) {
            for (const auto&[name, distance] : distances_to_stop) {
                transportCatalogue_.AddDistance(stop_name, name, distance);
            }
        }
    }

    void JsonReader::GetStatRequestsBus(std::map<std::string,json::Node> key, int request_id,json::Array array_request ) {
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

    void JsonReader::GetStatRequestsStop(std::map<std::string,json::Node> key, int request_id,json::Array array_request ){
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

    void JsonReader::GetStatRequestsMap(const MapRenderer::RenderSettings &renderSettings, int request_id,json::Array array_request ) {
        svg::Document doc;
        auto route_inform = requestHandler_.GetStopsWithRoute(buses_route);
        std::string render_str = requestHandler_.RenderMap(doc, route_inform, renderSettings);
        json::Node dict_map{json::Dict{{"request_id"s, request_id},
                                       {"map"s,        render_str}}};
        array_request.push_back(dict_map);
    }

    json::Array JsonReader::GetStatRequests(const json::Node& root_, const MapRenderer::RenderSettings &renderSettings,
                                            const TransportRouter::RoutingSettings& routingSettings) {

        graph::DirectedWeightedGraph<double> directedWeightedGraph_(transportCatalogue_.GetAllStops().size());
        TransportRouter transportRouter(routingSettings, transportCatalogue_);

        transportRouter.CreateGraph(directedWeightedGraph_);
        graph::Router router(directedWeightedGraph_);

        json::Array array_request;
        std::map<std::string,Node> data_root_ = root_.AsDict();
        Node stat_requests = data_root_["stat_requests"s];

        for (const Node &s_r_: stat_requests.AsArray()) {
            std::map<std::string,Node> key = s_r_.AsDict();
            int request_id = key["id"s].AsInt();
            Node type = key["type"s];

            if (type == "Bus"s) {
                GetStatRequestsBus(key, request_id, array_request);
            } else {
                json::Node dict_node_bus{json::Dict{{"request_id"s,    request_id},
                                                    {"error_message"s, "not found"s}}};
                array_request.push_back(dict_node_bus);
            }


            if (type == "Stop"s) {
                std::string name = key["name"s].AsString();
                Stop stop;
                stop.name = name;
                auto getStopStat = requestHandler_.GetBusesStop(stop);

                if (getStopStat != std::nullopt) {
                    GetStatRequestsStop(key, request_id, array_request);
                } else {
                    json::Node dict_node_stop{json::Dict{{"request_id"s,    request_id},
                                                         {"error_message"s, "not found"s}}};
                    array_request.push_back(dict_node_stop);
                }
            }

            if (type == "Map"s) {
                GetStatRequestsMap(renderSettings, request_id, array_request);
            }

            if (type == "Route"s) {
                std::string_view stop_from = key["from"s].AsString();
                std::string_view stop_to = key["to"s].AsString();

                auto get_find_route = requestHandler_.FindRoute(transportRouter, router, stop_from, stop_to);
                if (get_find_route == std::nullopt) {
                    json::Node dict_node_stop{json::Dict{{"request_id"s,    request_id},
                                                         {"error_message"s, "not found"s}}};
                    array_request.push_back(dict_node_stop);
                    continue;
                }

                json::Array items;
                for (const auto &get_f_r: get_find_route->busnum) {
                    json::Node item_stop{json::Dict{{"stop_name"s, get_f_r.stop_name_},
                                                    {"time"s,      routingSettings.bus_wait_time_},
                                                    {"type"s,      "Wait"s}}};
                    items.push_back(item_stop);

                    json::Node item_bus{json::Dict{{"bus"s,        get_f_r.bus_num_},
                                                   {"span_count"s, get_f_r.span_count_},
                                                   {"time"s,       get_f_r.time_ - routingSettings.bus_wait_time_},
                                                   {"type"s,       get_f_r.type_}}};

                    items.push_back(item_bus);
                }

                json::Node dict_map{json::Dict{{"request_id"s, request_id},
                                               {"total_time"s, get_find_route->weight_},
                                               {"items"s,      items}}};

                array_request.push_back(dict_map);
            }
        }
        return array_request;
    }



std::optional<MapRenderer::RenderSettings> JsonReader::ParseRenderSettings(const json::Node &root_) {
    MapRenderer::RenderSettings renderSettings;

    std::map<std::string, Node> data_root_ = root_.AsDict();
    Node render_settings = data_root_["render_settings"s];
    if (render_settings == nullptr) {
        return std::nullopt;
    }
    std::map<std::string, Node> key = render_settings.AsDict();

    renderSettings.width = key["width"s].AsDouble();
    renderSettings.height = key["height"s].AsDouble();
    renderSettings.padding = key["padding"s].AsDouble();
    renderSettings.stop_radius = key["stop_radius"s].AsDouble();
    renderSettings.line_width = key["line_width"s].AsDouble();

    renderSettings.bus_label_font_size = key["bus_label_font_size"s].AsInt();
    renderSettings.bus_label_offset.first =  key["bus_label_offset"s].AsArray()[0].AsDouble();
    renderSettings.bus_label_offset.second =  key["bus_label_offset"s].AsArray()[1].AsDouble();

    renderSettings.stop_label_font_size = key["stop_label_font_size"s].AsInt();
    renderSettings.stop_label_offset.first = key["stop_label_offset"s].AsArray()[0].AsDouble();
    renderSettings.stop_label_offset.second = key["stop_label_offset"s].AsArray()[1].AsDouble();

    svg::Color underlayer_color;
    if (key["underlayer_color"s].IsArray()) {
        std::vector<Node> u_c = key["underlayer_color"s].AsArray();
        if (u_c.size() == 3) {
            svg::Rgb rgb;
            rgb.red = u_c[0].AsInt();
            rgb.green = u_c[1].AsInt();
            rgb.blue = u_c[2].AsInt();
            underlayer_color = rgb;
        } else if(u_c.size() == 4) {
            svg::Rgba rgba;
            rgba.red = u_c[0].AsInt();
            rgba.green = u_c[1].AsInt();
            rgba.blue = u_c[2].AsInt();
            rgba.opacity = u_c[3].AsDouble();
            underlayer_color = rgba;
        }
    } else {
        underlayer_color = key["underlayer_color"s].AsString();
    }
    renderSettings.underlayer_color = underlayer_color;
    renderSettings.underlayer_width = key["underlayer_width"s].AsDouble();

    std::vector<svg::Color> color_plate;
    for (const Node& c_p : key["color_palette"s].AsArray()) {
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

std::optional<TransportRouter::RoutingSettings> JsonReader::routingSettings(const json::Node &root_) {
    std::map<std::string, Node> data_root_ = root_.AsDict();
    Node routing_settings = data_root_["routing_settings"s];
    if (routing_settings == nullptr) {
        return std::nullopt;
    }

    std::map<std::string, Node> key = routing_settings.AsDict();
    int bus_wait_time = key["bus_wait_time"s].AsInt();
    double bus_velocity = key["bus_velocity"s].AsDouble();

    if (bus_wait_time < 0) {
        std::cerr << "time in not valid"s << std::endl;
        return std::nullopt;
    }

    int time = bus_wait_time;
    return TransportRouter::RoutingSettings{time, bus_velocity * (1000.0 / 60.0)};
}
}