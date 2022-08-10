#pragma once

#include <string>
#include <sstream>
#include <optional>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "graph.h"

namespace json {

    class JsonReader {
    public:

        explicit JsonReader(transport_catalogue::TransportCatalogue &transportCatalogue,
                            renderer::MapRenderer &mapRenderer);

        json::Document LoadJSON(const std::string& s);
        json::Document Load(std::istream& input);
        static std::string Print(const json::Node &node);

        void PostRequestsInCatalogue(const json::Node& root_);

        std::optional<renderer::MapRenderer::RenderSettings> ParseRenderSettings(const json::Node& root_);
        std::optional<transport_router::TransportRouter::RoutingSettings> routingSettings(const json::Node& root_);

        void GetStatRequestsBus(std::map<std::string,json::Node> key, int request_id,json::Array array_request );
        void GetStatRequestsStop(std::map<std::string,json::Node> key, int request_id,json::Array array_request );
        void GetStatRequestsMap(const renderer::MapRenderer::RenderSettings &renderSettings, int request_id,json::Array array_request );
        void GetStatRequestsRoute(std::map<std::string,json::Node> key,json::Array array_request, int request_id,const graph::Router<double>& router,const transport_router::TransportRouter& transportRouter,const transport_router::TransportRouter::RoutingSettings& routingSettings);
        json::Array GetStatRequests(const json::Node& root_, const renderer::MapRenderer::RenderSettings &renderSettings,
                                    const transport_router::TransportRouter::RoutingSettings& routingSettings);

    private:
        transport_catalogue::TransportCatalogue &transportCatalogue_;
        renderer::MapRenderer &mapRenderer_;
        request_handler::RequestHandler requestHandler_;

        std::vector<std::pair<std::string, bool>> buses_route;
    };

}