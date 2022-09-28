#pragma once

#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"
#include "json_reader.h"

#include <vector>
#include <string>
#include <string_view>
#include <set>

namespace TransportCatalogue {

	namespace transport_router {

		struct TGraphEdge {
			graph::EdgeId id;
			std::string_view from;
			std::string_view name;
			size_t span_count_;
			double weight;

		};

		class TransportRouter {
		public:
			using BusesAndStops = std::pair<std::set<const Bus*, detail::BusHasher>, std::map<std::string_view, const Stop*>>;

			TransportRouter(const TransportCatalogue& catalogue);
			void SetSettings(BusTimesSettings&& settings);
			BusTimesSettings GetSettings() const;
			void FillGraph();

			const graph::DirectedWeightedGraph<double>& GetGraph() const;
			json::Node GetRouteNode(const graph::Router<double>& router, const std::string& from, const std::string& to, int id) const;

		private:
			graph::DirectedWeightedGraph<double> graph_;
			const TransportCatalogue& catalogue_;

			BusTimesSettings settings_;
			std::vector<TGraphEdge> edges_;
			std::unordered_map<std::string_view, graph::VertexId> vertexes_;

			void FillStopsVertexes(const std::map<std::string_view, const Stop*>& stops);
			void AddRouteEdge(TGraphEdge&& graph_edge, std::string_view to);

		};

	} //transport_router

} //TransportCatalogue