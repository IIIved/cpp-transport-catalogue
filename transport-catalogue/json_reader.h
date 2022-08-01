#pragma once

#include <string>
#include <sstream>
#include <optional>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json {

    class JsonReader {
    public:

        explicit JsonReader(transport_catalogue::TransportCatalogue &transportCatalogue, map_renderer::MapRenderer &mapRenderer);

        json::Document LoadJSON(const std::string& s);
        json::Document Load(std::istream& input);
        static std::string Print(const json::Node &node);

        void ParsingBasicQueries(const json::Node& root_);

        std::optional<map_renderer::MapRenderer::RenderSettings> ParseRenderSettings(const json::Node& root_);

        void GetStatRequestsBus(std::map<std::string,Node> key, int request_id,json::Array array_stat_req );
        void GetStatRequestsStop(std::map<std::string,Node> key, int request_id,json::Array array_stat_req );
        void GetStatRequestsMap(const map_renderer::MapRenderer::RenderSettings &renderSettings, int request_id,json::Array array_stat_req );

        json::Array GetStatRequests(const json::Node &root_, const map_renderer::MapRenderer::RenderSettings &renderSettings);
        std::optional<map_renderer::MapRenderer::RenderSettings> ParseRenderSettingsColor(const json::Node &root_,std::map<std::string, json::Node> key, svg::Color underlayer_color);

    private:
        transport_catalogue::TransportCatalogue &transportCatalogue_;
        map_renderer::MapRenderer &mapRenderer_;
        request_handler::RequestHandler requestHandler_;

        std::vector<std::pair<std::string, bool>> buses_route;
    };

}//namespace json