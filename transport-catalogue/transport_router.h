#pragma once

#include <string_view>
#include <string>
#include <map>
#include <set>
#include <unordered_map>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include "domain.h"

namespace transport_router {
    using namespace std::literals;

    class TransportRouter {
    public:
        struct RoutingSettings {
            int bus_wait_time_;
            double bus_velocity_;
        };

        struct GraphInfoStops {
            std::string_view bus_num;
            std::string_view stop_from;
            std::string_view stop_to;
            double width = 0.0;
            int span_count = 0;
        };

        explicit TransportRouter(const TransportRouter::RoutingSettings& routingSettings,
                                 transport_catalogue::TransportCatalogue& transportCatalogue)
                : routingSettings_(routingSettings), transportCatalogue_(transportCatalogue) {
        }

        void CreateGraph(graph::DirectedWeightedGraph<double>& directedWeightedGraph);
        std::set<const transport_catalogue::Bus*> AddAllBusRoute();
        void AddAllVertexStop();
        void AddEdges(graph::DirectedWeightedGraph<double>& directedWeightedGraph, const GraphInfoStops& graphInfoStops);

        struct DataBuild{
            std::string bus_num_;
            std::string stop_name_;
            int span_count_ = 0;
            double time_ = 0.0;
            std::string type_;
        };

        struct ResponseFindRoute {
            double weight_ = 0.0;
            std::vector<DataBuild> findRoute;
        };

        std::optional<ResponseFindRoute> FindRoute(const graph::Router<double>& router, std::string_view stop_from, std::string_view stop_to) const;

    private:
        enum ItemType {
            Bus,
            Stop
        };

        std::string getItemType(ItemType itemType) const {
            if (itemType == Bus)
                return "Bus"s;
            if (itemType == Stop)
                return "Stop"s;
            return "failed item type"s;
        }

        const RoutingSettings routingSettings_;
        transport_catalogue::TransportCatalogue& transportCatalogue_;

        std::vector<GraphInfoStops> data_;
        std::unordered_map<std::string, graph::VertexId> stop_name_vertex_id_;
    };

}