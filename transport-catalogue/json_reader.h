#pragma once

#include "json.h"
#include "geo.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "svg.h"
#include "domain.h"
#include "serialization.h"

#include <deque>
#include <string_view>
#include <optional>

namespace json_reader {

    struct Request {
        std::string type;

        virtual ~Request() = default;
    };

    struct RequestStat : public Request {
        std::optional<std::string> name;
        int id = 0;
    };

    struct RequestStatRoute : public RequestStat {
        std::string from;
        std::string to;
        int id = 0;
    };

    struct RequestStop : public Request {
        std::string name;
        geo::Coordinates coordinates;
        std::map<std::string, int> distances_;
    };

    struct RequestBus : public Request {
        std::string name;
        bool looping = false;
        std::vector<std::string> stops_;
    };

    struct Item {
        std::string type;
        double time = 0.0;

        Item(const std::string type_, double time_)
        : type(type_), time(time_){}

        virtual ~Item() = default;
    };

    struct ItemWait : public Item {
        std::string stop_name;

        ItemWait(std::string&& type_, double time_, const std::string& stop_name_)
        :Item(std::move(type_), time_), stop_name(stop_name_){}
    };

    struct ItemBus : public Item {
        std::string bus_name;
        int span_count = 0;

        ItemBus(std::string&& type_, double time_, const std::string& bus_name_, int span_count_)
            :Item(std::move(type_), time_), bus_name(bus_name_), span_count(span_count_) {}
    };

    std::map<std::string, int> MakeMapForStop(const json::Dict& dic);
    std::vector<std::string> MakeVectorForBus(const json::Array& arr);
    RequestStop MakeRequestStop(const json::Dict& dic);
    RequestBus MakeRequestBus(const json::Dict& dic);
    RequestStat MakeRequestStat(const json::Dict& dic);
    RequestStatRoute MakeRequestStatRoute(const json::Dict& dic);
    json::Node MakeNodeForError(int id);
    json::Node MakeNodeForStop(int id, std::set<std::string>&& buses_);
    json::Node MakeNodeForRoute(int id, double time, json::Array&& items);
    json::Node MakeNodeForBus(int id, TransportCatalogue::detail::InformationBus&& inform);
    svg::Color ColorFromNode(const json::Node& node);
    RenderSettings GetRenderSettingsForMap(const json::Dict& dic);
    TransportCatalogue::BusTimesSettings GetRenderSettingsForRouter(const json::Dict& dic);
    serialization::SerializationSettings GetSettingsForSerializator(const json::Dict& dic);
    json::Node MakeDictFromItem(Item* item);

}//namespace json_reader  