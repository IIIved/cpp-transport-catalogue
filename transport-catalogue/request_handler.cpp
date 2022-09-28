#include "request_handler.h"
#include "router.h"

using namespace std::literals;

namespace RequestHandler {

    void RequestHandler::FillRequestsToFill(const std::vector<json::Node>& array_) {
        using namespace json_reader;
        for (const auto& element_ : array_) {
            if (element_.AsMap().count("type"s)) {
                if (element_.AsMap().at("type"s).AsString() == "Stop"s) {
                    requests_to_fill_.push_front(std::make_unique<RequestStop>(MakeRequestStop(element_.AsMap())));
                }
                else if (element_.AsMap().at("type"s).AsString() == "Bus"s) {
                    requests_to_fill_.push_back(std::make_unique<RequestBus>(MakeRequestBus(element_.AsMap())));
                }
            }
        }
    }

    void RequestHandler::FillRequestsToOut(const std::vector<json::Node>& array_) {
        using namespace json_reader;
        for (const auto& element_ : array_) {
            if (element_.AsMap().count("type"s)) {
                if (element_.AsMap().at("type"s).AsString() == "Bus"s
                    || element_.AsMap().at("type"s).AsString() == "Stop"s
                    || element_.AsMap().at("type"s).AsString() == "Map"s) {
                    requests_to_out_.push_back(std::make_unique<RequestStat>(MakeRequestStat(element_.AsMap())));
                }
                else if (element_.AsMap().at("type"s).AsString() == "Route"s) {
                    requests_to_out_.push_back(std::make_unique<RequestStatRoute>(MakeRequestStatRoute(element_.AsMap())));
                }
            }
        }
    }

    void RequestHandler::FillDocument(std::istream& in_stream) {
        document_ = json::Load(in_stream);
    }

    void RequestHandler::ReadInput(std::istream& in_stream) {
        FillDocument(in_stream);
        for (const auto& [type, value] : document_.GetRoot().AsMap()) {
            if (type == "base_requests"s) {
                FillRequestsToFill(value.AsArray());
            }
            else if (type == "stat_requests"s) {
                FillRequestsToOut(value.AsArray());
            }
            else if (type == "render_settings"s) {
                FillRender(value.AsMap());
            }
            else if (type == "routing_settings"s) {
                FillSettingsRouter(value.AsMap());
            }
            else if (type == "serialization_settings"s) {
                FillSettingsSerializator(value.AsMap());
            }
        }
    }

    void RequestHandler::SetDistancesInCatalog() {
        using namespace json_reader;
        for (const auto& data_ : requests_to_fill_) {
            if (RequestStop* stop = dynamic_cast<RequestStop*>(data_.get())) {
                for (const auto& [name2, dist] : stop->distances_) {
                    catalogue_.SetDistance(stop->name, name2, dist);
                }
            }
        }
    }

    void RequestHandler::FillTransportCatalogue() {
        using namespace json_reader;
        if (requests_to_fill_.empty()) {
            return;
        }
        for (const auto& data_ : requests_to_fill_) {
            if (RequestStop* stop = dynamic_cast<RequestStop*>(data_.get())) {
                catalogue_.AddStop(stop->name, stop->coordinates);
            }
            else if (RequestBus* bus = dynamic_cast<RequestBus*>(data_.get())) {
                catalogue_.AddBus(bus->name, bus->looping, bus->stops_);
            }
        }
        SetDistancesInCatalog();
    }

    void RequestHandler::FillTransportRouter() {
        router_.FillGraph();
    }

    void RequestHandler::PrintRequests(std::ostream& out) {
        using namespace json_reader;
        bool start_ = false;
        graph::Router router(router_.GetGraph());
        out << "["s << std::endl;
        for (const auto& request_ : requests_to_out_) {
            if (start_) {
                out << "," << std::endl;
            }
            if (request_->type == "Stop"s) {
                auto [has_answer, buses] = catalogue_.GetInformationStop(std::string_view((request_->name).value()));
                json::Node node;
                if (!has_answer) {
                    node = MakeNodeForError(request_->id);
                }
                else {
                    node = MakeNodeForStop(request_->id, std::move(buses));
                }
                json::Print(json::Document(node), out);
            }
            else if (request_->type == "Bus"s) {
                TransportCatalogue::detail::InformationBus answer = catalogue_.GetInformationBus(std::string_view((request_->name).value()));
                json::Node node;
                if (answer == TransportCatalogue::detail::InformationBus{}) {
                    node = MakeNodeForError(request_->id);
                }
                else {
                    node = MakeNodeForBus(request_->id, std::move(answer));
                }
                json::Print(json::Document(node), out);
            }
            else if (request_->type == "Map"s) {
                out << "{\n\"map\": \"";
                renderer_.SetDateForMap(catalogue_.InfoForMap());
                renderer_.Render(out);
                out << "\"," << std::endl << "\"request_id\": "s << request_->id << std::endl << "}";
            }
            else if (request_->type == "Route"s) {
                RequestStatRoute* route = dynamic_cast<RequestStatRoute*>(request_.get());
                json::Node node = router_.GetRouteNode(router, route->from, route->to, route->id);
                json::Print(json::Document(node), out);
            }
            start_ = true;
        }
        out << "\n]"s << std::endl;
    }

    void RequestHandler::FillRender(const std::map<std::string, json::Node>& dic) {
        using namespace json_reader;
        renderer_.SetSettings(GetRenderSettingsForMap(dic));
    }

    void RequestHandler::FillSettingsRouter(const std::map<std::string, json::Node>& dic) {
        using namespace json_reader;
        router_.SetSettings(GetRenderSettingsForRouter(dic));
    }

    void RequestHandler::FillSettingsSerializator(const std::map<std::string, json::Node>& dic) {
        using namespace json_reader;
        serializator.SetSerializationSettings(GetSettingsForSerializator(dic));
    }

    void RequestHandler::SerializeCatalog() {
        serializator.CatalogueSerialize(catalogue_, router_.GetSettings(), renderer_.GetSettings());
    }

    void RequestHandler::DeserializeCatalog() {
        transport_catalogue_proto::TransportCatalogue tcp = serializator.CatalogueDeserialize();
        serializator.FillCatalogue(catalogue_, tcp);
        renderer_.SetSettings(serializator.GetRenderSettings(tcp.render_settings()));
        router_.SetSettings(serializator.GetBusTimesSettings(tcp.time_settings()));
    }

}//namespace RequestHandler