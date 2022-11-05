#include "transport_router.h"

#include "transport_router.pb.h"
#include <iostream>
namespace transport::router {

    using namespace transport::domains;

    void TransportRouter::SerializeSettings(TCProto::RoutingSettings& proto) {
        proto.set_bus_wait_time(settings_.bus_wait_time);
        proto.set_bus_velocity(settings_.bus_velocity);
    }

    RoutingSettings TransportRouter::DeserializeSettings(const TCProto::RoutingSettings& proto) {
        return { proto.bus_wait_time(), proto.bus_velocity() };
    }

    TransportRouter::TransportRouter(
        std::map<std::string_view, std::shared_ptr<Stop>>& stops,
        std::map<std::string_view, std::shared_ptr<Bus>>& buses,
        RoutingSettings settings
    )
        : settings_(settings)
        , stops_(stops)
        , buses_(buses) {
    }

    void TransportRouter::BuildGraph() {
        graph_ = graph::DirectedWeightedGraph<double>(stops_.size());
 
        FillVertexes();
        FillEdges();

        search_in_graph_ = std::make_unique<graph::Router<double>>(graph_);
    }

    void TransportRouter::FillVertexes() {
        size_t i = 0;
        for (auto [_, stop] : stops_) {
            graph_vertexes_[stop] = i++;
        }
    }
    void TransportRouter::FillEdges() {
        for (auto [_, route] : buses_) {

            std::vector<double> distance_forward;
            distance_forward.resize(route->stops.size());
            std::vector<double> distance_reverse;
            distance_reverse.resize(route->stops.size());

            double forward_sum = 0.0;
            double reverse_sum = 0.0;

            for (size_t index = 1; index < route->stops.size(); index++) {
                forward_sum += RealLenBeetwenStops(stops_.at(route->stops[index - 1]), stops_.at(route->stops[index]));
                distance_forward[index] = forward_sum;
                reverse_sum += RealLenBeetwenStops(stops_.at(route->stops[index]), stops_.at(route->stops[index - 1]));
                distance_reverse[index] = reverse_sum;
            }

            for (int s = 0; s + 1 < route->stops.size(); s++) {
                for (int s1 = s + 1; s1 < route->stops.size(); s1++) {
                    RouteItem item;
                    item.start_stop_idx = stops_.at(route->stops[s]);
                    item.finish_stop_idx = stops_.at(route->stops[s1]);
                    item.bus = route;
                    item.stop_count = std::abs(s - s1);
                    item.wait_time = settings_.bus_wait_time;
                    item.trip_time = (s < s1 ? distance_forward[s1] - distance_forward[s] : distance_reverse[s] - distance_reverse[s1]) / settings_.bus_velocity;

                    int id = graph_.AddEdge(
                        graph::Edge<double>{
                        graph_vertexes_[item.start_stop_idx],
                            graph_vertexes_[item.finish_stop_idx],
                            item.trip_time + item.wait_time
                    }
                    );
                    graph_edges_.push_back(std::move(item));

                    if (!route->is_roundtrip) {
                        RouteItem item;
                        item.start_stop_idx = stops_.at(route->stops[s1]);
                        item.finish_stop_idx = stops_.at(route->stops[s]);
                        item.bus = route;
                        item.stop_count = std::abs(s - s1);
                        item.wait_time = settings_.bus_wait_time;
                        item.trip_time = (s1 < s ? distance_forward[s] - distance_forward[s1] : distance_reverse[s1] - distance_reverse[s]) / settings_.bus_velocity;

                        int id = graph_.AddEdge(
                            graph::Edge<double>{
                            graph_vertexes_[item.start_stop_idx],
                                graph_vertexes_[item.finish_stop_idx],
                                item.trip_time + item.wait_time
                        }
                        );
                        graph_edges_.push_back(std::move(item));
                    }
                }
            }
        }
    }

    std::shared_ptr<std::vector <RouteItem>> TransportRouter::findRoute(const std::string_view from, const std::string_view to) {
        std::shared_ptr<Stop> stop_from = stops_.at(from);
        std::shared_ptr<Stop> stop_to = stops_.at(to);
        if (stop_from == nullptr || stop_to == nullptr) return nullptr;

        std::shared_ptr<std::vector<RouteItem>> res = std::make_shared<std::vector<RouteItem>>();
        if (stop_from == stop_to)   return res;


        std::shared_ptr<std::vector<size_t>> res_tmp = search_in_graph_->BuildRoute(graph_vertexes_.at(stop_from), graph_vertexes_.at(stop_to));
        if (res_tmp == nullptr)  return nullptr;

        res->reserve(res_tmp->size());
        for (auto e = res_tmp->begin(); e != res_tmp->end(); e++) {
            res->push_back(graph_edges_.at(*e));
        }
        return res;
    }

    const RoutingSettings& TransportRouter::GetSettings() const {
        return settings_;
    }

    void TransportRouter::SerializeData(TCProto::TransportRouter& proto) const {
        graph_.Serialize(*proto.mutable_graph());
        search_in_graph_->Serialize(*proto.mutable_router());

        for (const auto& item : graph_edges_) {
            TCProto::RouteItem& proto_edge = *proto.add_graph_edges();
            proto_edge.set_start_stop(item.start_stop_idx->name);
            proto_edge.set_finish_stop(item.finish_stop_idx->name);
            proto_edge.set_bus(item.bus->name);
            proto_edge.set_stop_count(item.stop_count);
            proto_edge.set_trip_time(item.trip_time);
            proto_edge.set_wait_time(item.wait_time);
        }

        for (const auto& [name, value] : graph_vertexes_) {
            TCProto::GraphVertexes& proto_vertex = *proto.add_graph_vertexes();
            proto_vertex.set_stop_name(name->name);
            proto_vertex.set_index(value);
        }

    }

    void TransportRouter::DeserializeData(const TCProto::TransportRouter& proto) {
        graph_ = graph::DirectedWeightedGraph<double>::Deserialize(proto.graph());
       
        for (const auto& proto_edge : proto.graph_edges()) {
            RouteItem tmp;
            tmp.start_stop_idx = stops_.at(proto_edge.start_stop());
            tmp.finish_stop_idx = stops_.at(proto_edge.finish_stop());
            tmp.bus = buses_.at(proto_edge.bus());
            tmp.stop_count = proto_edge.stop_count();
            tmp.wait_time = proto_edge.wait_time();
            tmp.trip_time = proto_edge.trip_time();
           
            graph_edges_.push_back(std::move(tmp));

        }

        for (const auto& proto_vertex : proto.graph_vertexes()) {
            graph_vertexes_[ stops_.at(proto_vertex.stop_name()) ] = proto_vertex.index();
        }

        search_in_graph_ = graph::Router<double>::Deserialize(proto.router(), graph_);
    }
}