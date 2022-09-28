#pragma once

#include "json.h"
#include "geo.h"
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <deque>
#include <string_view>

namespace RequestHandler {

    class RequestHandler {
    public:
        RequestHandler(TransportCatalogue::TransportCatalogue& catalogue,
            rendering::MapRenderer& renderer,
            TransportCatalogue::transport_router::TransportRouter& router)
            :catalogue_(catalogue), renderer_(renderer), router_(router) {}

        void ReadInput(std::istream& input);
        void FillTransportCatalogue();
        void FillTransportRouter();
        void PrintRequests(std::ostream& out);
        void SerializeCatalog();
        void DeserializeCatalog();

    private:
        TransportCatalogue::TransportCatalogue& catalogue_;
        rendering::MapRenderer& renderer_;
        TransportCatalogue::transport_router::TransportRouter& router_;
        serialization::Serializator serializator;

        json::Document document_;
        std::deque<std::unique_ptr<json_reader::Request>> requests_to_fill_;
        std::deque<std::unique_ptr<json_reader::RequestStat>> requests_to_out_;

        void FillDocument(std::istream& in_stream);
        void FillRequestsToFill(const std::vector<json::Node>& array_);
        void FillRequestsToOut(const std::vector<json::Node>& array_);
        void FillRender(const std::map<std::string, json::Node>&);
        void FillSettingsRouter(const std::map<std::string, json::Node>&);
        void FillSettingsSerializator(const std::map<std::string, json::Node>&);
        void SetDistancesInCatalog();
    };

}//namespace RequestHandler