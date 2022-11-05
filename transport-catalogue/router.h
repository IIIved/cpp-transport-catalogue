#pragma once

#include "graph.h"

#include "graph.pb.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace graph {

    template <typename Weight>
    class Router {
    private:
        using Graph = DirectedWeightedGraph<Weight>;

    public:
        explicit Router(const Graph& graph);

        struct RouteInfo {
            Weight weight;
            std::vector<EdgeId> edges;
        };

        std::shared_ptr<std::vector<size_t>> BuildRoute(VertexId from, VertexId to) const;

        void Serialize(GraphProto::Router& proto);
        static std::unique_ptr<Router> Deserialize(const GraphProto::Router& proto, const Graph& graph);

    private:

        Router(const Graph& graph, const GraphProto::Router& proto);

        struct RouteInternalData {
            Weight weight;
            std::optional<EdgeId> prev_edge;
        };
        using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;



        void InitializeRoutesInternalData(const Graph& graph) {
            const size_t vertex_count = graph.GetVertexCount();
            for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
                routes_internal_data_[vertex][vertex] = RouteInternalData{ ZERO_WEIGHT, std::nullopt };
                for (const EdgeId edge_id : graph.GetIncidentEdges(vertex)) {
                    const auto& edge = graph.GetEdge(edge_id);
                    if (edge.weight < ZERO_WEIGHT) {
                        throw std::domain_error("Edges' weights should be non-negative");
                    }
                    auto& route_internal_data = routes_internal_data_[vertex][edge.to];
                    if (!route_internal_data || route_internal_data->weight > edge.weight) {
                        route_internal_data = RouteInternalData{ edge.weight, edge_id };
                    }
                }
            }
        }

        void RelaxRoute(VertexId vertex_from, VertexId vertex_to, const RouteInternalData& route_from, const RouteInternalData& route_to) {
            auto& route_relaxing = routes_internal_data_[vertex_from][vertex_to];
            const Weight candidate_weight = route_from.weight + route_to.weight;
            if (!route_relaxing || candidate_weight < route_relaxing->weight) {
                route_relaxing = { candidate_weight,
                                  route_to.prev_edge ? route_to.prev_edge : route_from.prev_edge
                };
            }
        }

        void RelaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) {
            for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
                if (const auto& route_from = routes_internal_data_[vertex_from][vertex_through]) {
                    for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) {
                        if (const auto& route_to = routes_internal_data_[vertex_through][vertex_to]) {
                            RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                        }
                    }
                }
            }
        }

        static constexpr Weight ZERO_WEIGHT{};
        const Graph& graph_;
        RoutesInternalData routes_internal_data_;
    };


    template <typename Weight>
    Router<Weight>::Router(const Graph& graph)
        : graph_(graph)
        , routes_internal_data_(graph.GetVertexCount(),
            std::vector<std::optional<RouteInternalData>>(graph.GetVertexCount()))
    {
        InitializeRoutesInternalData(graph);

        const size_t vertex_count = graph.GetVertexCount();
        for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
            RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
        }
    }

    template <typename Weight>
    std::shared_ptr<std::vector<size_t>> Router<Weight>::BuildRoute(VertexId from, VertexId to) const {
        const auto& route_internal_data = routes_internal_data_.at(from).at(to);
        if (!route_internal_data) {
            return nullptr;
        }

        std::shared_ptr<std::vector<size_t>> edges = std::make_shared<std::vector<size_t>>();
        for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
            edge_id;
            edge_id = routes_internal_data_[from][graph_.GetEdge(*edge_id).from]->prev_edge)
        {
            edges->push_back(*edge_id);
        }
        std::reverse(edges->begin(), edges->end());

        return edges;
    }


    template <typename Weight>
    void Router<Weight>::Serialize(GraphProto::Router& proto) {

        for (const auto& internal_data : routes_internal_data_) {
            auto& internal_data_proto = *proto.add_internal_data();

            for (const auto& route_data : internal_data) {
                auto& route_data_proto = *internal_data_proto.add_route();

                if (route_data) {
                    route_data_proto.set_exists(true);
                    route_data_proto.set_weight(route_data->weight);

                    if (route_data->prev_edge) {
                        route_data_proto.set_has_prev_edge(true);
                        route_data_proto.set_prev_edge(*route_data->prev_edge);
                    }
                }
            }
        }
    }

    template <typename Weight>
    Router<Weight>::Router(const Graph& graph, const GraphProto::Router& proto)
        : graph_(graph)
    {

        routes_internal_data_.reserve(proto.internal_data_size());
        for (const auto& internal_data_proto : proto.internal_data()) {
            auto& internal_data = routes_internal_data_.emplace_back();
            internal_data.reserve(internal_data_proto.route_size());

            for (const auto& route_data_proto : internal_data_proto.route()) {
                auto& route_data = internal_data.emplace_back();

                if (route_data_proto.exists()) {
                    route_data = RouteInternalData{ route_data_proto.weight(), std::nullopt };

                    if (route_data_proto.has_prev_edge()) {
                        route_data->prev_edge = route_data_proto.prev_edge();
                    }
                }
            }
        }
    }

    template <typename Weight>
    std::unique_ptr<Router<Weight>> Router<Weight>::Deserialize(const GraphProto::Router& proto, const Graph& graph) {
        return std::unique_ptr<Router>(new Router(graph, proto));
    }
}