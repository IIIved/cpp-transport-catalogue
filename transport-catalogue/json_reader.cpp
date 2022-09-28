#include "json_reader.h"

#include <iostream>

using namespace std::literals;

namespace json_reader {

    std::map<std::string, int> MakeMapForStop(const json::Dict& dic) {
        std::map<std::string, int> res;
        for (const auto& elem : dic) {
            res[elem.first] = elem.second.AsInt();
        }
        return res;
    }
    std::vector<std::string> MakeVectorForBus(const json::Array& arr) {
        std::vector<std::string> res;
        for (const auto& elem : arr) {
            res.push_back(elem.AsString());
        }
        return res;
    }

    RequestStop MakeRequestStop(const json::Dict& dic) {
        RequestStop stop;
        stop.type = "Stop"s;
        stop.name = dic.at("name"s).AsString();
        stop.coordinates = { dic.at("latitude"s).AsDouble(), dic.at("longitude"s).AsDouble() };
        if (dic.count("road_distances"s)) {
            stop.distances_ = move(MakeMapForStop(dic.at("road_distances"s).AsMap()));
        }

        return stop;
    }

    RequestBus MakeRequestBus(const json::Dict& dic) {
        RequestBus bus;
        bus.type = "Stop"s;
        bus.name = dic.at("name"s).AsString();
        bus.stops_ = move(MakeVectorForBus(dic.at("stops"s).AsArray()));
        bus.looping = dic.at("is_roundtrip"s).AsBool();
        return bus;
    }

    RequestStat MakeRequestStat(const json::Dict& dic) {
        RequestStat res;
        res.id = dic.at("id"s).AsInt();
        if (dic.at("type"s).AsString() == "Bus"s) {
            res.name = dic.at("name"s).AsString();
            res.type = "Bus"s;
        }
        else if (dic.at("type"s).AsString() == "Stop"s) {
            res.name = dic.at("name"s).AsString();
            res.type = "Stop"s;
        }
        else if (dic.at("type"s).AsString() == "Map"s) {
            res.type = "Map"s;
        }
        return res;
    }

    RequestStatRoute MakeRequestStatRoute(const json::Dict& dic) {
        RequestStatRoute res;
        res.id = dic.at("id"s).AsInt();
        res.type = "Route"s;
        res.from = dic.at("from"s).AsString();
        res.to = dic.at("to"s).AsString();

        return res;
    }

    json::Node MakeNodeForError(int id) {
        return json::Builder{}.StartDict()
            .Key("request_id"s).Value(id)
            .Key("error_message"s).Value("not found"s)
            .EndDict().Build();
    }

    json::Node MakeNodeForRoute(int id, double time, json::Array&& items) {
        return json::Builder{}.StartDict()
            .Key("request_id"s).Value(id)
            .Key("total_time"s).Value(time)
            .Key("items"s).Value(items)
            .EndDict().Build();
    }

    json::Node MakeNodeForBus(int id, TransportCatalogue::detail::InformationBus&& inform) {
        return json::Builder{}.StartDict()
            .Key("curvature"s).Value(inform.curv)
            .Key("request_id"s).Value(id)
            .Key("route_length"s).Value(inform.distance)
            .Key("stop_count"s).Value(static_cast<int>(inform.static_infom_bus.number_stops))
            .Key("unique_stop_count"s).Value(static_cast<int>(inform.static_infom_bus.number_stops_un))
            .EndDict().Build();
    }

    json::Node MakeNodeForStop(int id, std::set<std::string>&& buses_) {
        return json::Builder{}.StartDict()
            .Key("buses"s).Value(json::Array{ buses_.begin(), buses_.end() })
            .Key("request_id"s).Value(id)
            .EndDict().Build();
    }

    svg::Color ColorFromNode(const json::Node& node) {
        if (node.IsArray()) {
            if (node.AsArray().size() == 3) {
                svg::Rgb rgb;
                rgb.red = node.AsArray()[0].AsInt();
                rgb.green = node.AsArray()[1].AsInt();
                rgb.blue = node.AsArray()[2].AsInt();
                return rgb;
            }
            else {
                svg::Rgba rgba;
                rgba.red = node.AsArray()[0].AsInt();
                rgba.green = node.AsArray()[1].AsInt();
                rgba.blue = node.AsArray()[2].AsInt();
                rgba.opacity = node.AsArray()[3].AsDouble();
                return rgba;
            }
        }
        else {
            return node.AsString();
        }
    }

    RenderSettings GetRenderSettingsForMap(const json::Dict& dic) {
        RenderSettings res;
        res.width = dic.at("width"s).AsDouble();
        res.height = dic.at("height"s).AsDouble();
        res.padding = dic.at("padding"s).AsDouble();
        res.line_width = dic.at("line_width"s).AsDouble();
        res.stop_radius = dic.at("stop_radius"s).AsDouble();
        res.bus_label_font_size = dic.at("bus_label_font_size"s).AsInt();
        res.bus_label_offset[0] = dic.at("bus_label_offset"s).AsArray()[0].AsDouble();
        res.bus_label_offset[1] = dic.at("bus_label_offset"s).AsArray()[1].AsDouble();
        res.stop_label_font_size = dic.at("stop_label_font_size"s).AsInt();
        res.stop_label_offset[0] = dic.at("stop_label_offset"s).AsArray()[0].AsDouble();
        res.stop_label_offset[1] = dic.at("stop_label_offset"s).AsArray()[1].AsDouble();
        res.underlayer_color = ColorFromNode(dic.at("underlayer_color"s));
        res.underlayer_width = dic.at("underlayer_width"s).AsDouble();
        for (const auto& node : dic.at("color_palette"s).AsArray()) {
            res.color_palette.push_back(ColorFromNode(node));
        }
        return res;
    }

    TransportCatalogue::BusTimesSettings GetRenderSettingsForRouter(const json::Dict& dic) {
        return { dic.at("bus_wait_time"s).AsDouble(), (dic.at("bus_velocity"s).AsDouble() * 1000.0) / 60.0 };
    }

    serialization::SerializationSettings GetSettingsForSerializator(const json::Dict& dic) {
        return { dic.at("file"s).AsString() };
    }

    json::Node MakeDictFromItem(Item* item) {
        if (item->type == "Wait"s) {
            ItemWait* route = dynamic_cast<ItemWait*>(item);
            return json::Builder{}.StartDict()
                .Key("type"s).Value(route->type)
                .Key("stop_name"s).Value(route->stop_name)
                .Key("time"s).Value(route->time)
                .EndDict().Build();
        }
        else if (item->type == "Bus"s) {
            ItemBus* route = dynamic_cast<ItemBus*>(item);
            return json::Builder{}.StartDict()
                .Key("type"s).Value(route->type)
                .Key("bus"s).Value(route->bus_name)
                .Key("span_count"s).Value(route->span_count)
                .Key("time"s).Value(route->time)
                .EndDict().Build();
        }
        return {};
    }

}//namespace json_reader