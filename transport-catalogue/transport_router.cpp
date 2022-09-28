#include "transport_router.h"

namespace TransportCatalogue {

	namespace transport_router {

		TransportRouter::TransportRouter(const TransportCatalogue& catalogue)
			:catalogue_(catalogue) {
		}

		void TransportRouter::FillGraph() {
			auto [buses, stops] = catalogue_.InfoForMap();
			graph_.SetVertexCount(stops.size());
			FillStopsVertexes(stops);
			edges_.reserve(stops.size() * stops.size());
			for (const Bus* route : buses) {
				size_t size = route->stops_for_bus_.size();
				if (route->looping) {
					for (size_t i = 0; i < size - 1; i++) {
						double weight = 0.0;
						for (size_t j = i + 1u; j < size; j++) {
							weight += static_cast<double>(catalogue_.GetDistanceInAnyDirection(route->stops_for_bus_[j - 1u], route->stops_for_bus_[j]));
							AddRouteEdge({ 0, route->stops_for_bus_[i], route->name_bus, (j - i), weight }, route->stops_for_bus_[j]);
						}
					}
				}
				else {
					for (size_t i = 0; i < size - 1; i++) {
						double weight_forward = 0.0;
						double weight_back = 0.0;
						for (size_t j = i + 1u; j < size; j++) {
							weight_forward += static_cast<double>(catalogue_.GetDistanceInAnyDirection(route->stops_for_bus_[j - 1u], route->stops_for_bus_[j]));
							AddRouteEdge({ 0, route->stops_for_bus_[i], route->name_bus, (j - i), weight_forward }, route->stops_for_bus_[j]);

							weight_back += static_cast<double>(catalogue_.GetDistanceInAnyDirection(route->stops_for_bus_[j], route->stops_for_bus_[j - 1u]));
							AddRouteEdge({ 0, route->stops_for_bus_[j], route->name_bus, (j - i), weight_back }, route->stops_for_bus_[i]);
						}
					}
				}
			}
		}

		void TransportRouter::SetSettings(BusTimesSettings&& settings) {
			settings_ = std::move(settings);
		}

		BusTimesSettings TransportRouter::GetSettings() const {
			return settings_;
		}

		const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
			return graph_;
		}

		json::Node TransportRouter::GetRouteNode(const graph::Router<double>& router, const std::string& from, const std::string& to, int id) const {
			using namespace std::literals;
			using namespace json_reader;
			if (from == to) {
				return json_reader::MakeNodeForRoute(id, 0.0, {});
			}
			if (vertexes_.count(from) < 1 || vertexes_.count(to) < 1) {
				return MakeNodeForError(id);
			}
			auto route_info = router.BuildRoute(vertexes_.at(from), vertexes_.at(to));
			if (!route_info.has_value()) {
				return MakeNodeForError(id);
			}
			size_t size = (route_info->edges).size();
			json::Array array;
			array.reserve(size * 2u);
			for (graph::EdgeId id : route_info->edges) {
				ItemWait wait("Wait"s, settings_.bus_wait_time_, std::string(edges_[id].from));
				array.push_back(json_reader::MakeDictFromItem(&wait));
				ItemBus bus("Bus"s, edges_[id].weight - settings_.bus_wait_time_, std::string(edges_[id].name), edges_[id].span_count_);
				array.push_back(json_reader::MakeDictFromItem(&bus));
			}
			return json_reader::MakeNodeForRoute(id, route_info->weight, std::move(array));
		}

		void TransportRouter::FillStopsVertexes(const std::map<std::string_view, const Stop*>& stops) {
			size_t id = 0;
			for (const auto& stop : stops) {
				vertexes_[stop.first] = id++;
			}
		}

		void TransportRouter::AddRouteEdge(TGraphEdge&& graph_edge, std::string_view to) {
			graph_edge.weight = (graph_edge.weight / settings_.bus_velocity_m_m_) + settings_.bus_wait_time_;
			graph_edge.id = graph_.AddEdge({ vertexes_.at(graph_edge.from), vertexes_.at(to), graph_edge.weight });
			edges_.push_back(std::move(graph_edge));
		}

	} //transport_router

} //TransportCatalogue 