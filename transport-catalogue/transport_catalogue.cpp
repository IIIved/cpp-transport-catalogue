#include "transport_catalogue.h"

#include "transport_catalogue.pb.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace transport::catalogue {

	TransportCatalogue::TransportCatalogue(
		std::vector<std::shared_ptr<Stop>>& stops,
		std::vector<std::shared_ptr<Bus>>& buses,
		RenderSettings render_settings,
		RoutingSettings router_settings
	) {

		for (auto& stop : stops) {
			stops_[stop->name] = stop;
		}

		for (auto& bus : buses) {
			buses_[bus->name] = bus;

			std::set<std::string_view> stops_uniq;
			for (auto& stop : bus->stops) {
				stops_uniq.insert(stop);
				stops_[stop]->buses.insert(bus->name);
			}
			bus->unique_stop_count = stops_uniq.size();

			double len_by_coordinates = 0;
			for (size_t i = 0; i + 1 < bus->stops.size(); i++) {
				len_by_coordinates += geo::ComputeDistance(
					stops_[bus->stops[i]]->coordinates,
					stops_[bus->stops[i + 1]]->coordinates
				);

				bus->route_length += RealLenBeetwenStops(stops_[bus->stops[i]], stops_[bus->stops[i + 1]]);
			}


			if (bus->is_roundtrip) {
				bus->stop_count = bus->stops.size();
			}
			else {
				bus->stop_count = bus->stops.size() * 2 - 1;
				len_by_coordinates *= 2;

				for (int i = bus->stops.size() - 1; i - 1 > -1; i--) {
					bus->route_length += RealLenBeetwenStops(stops_[bus->stops[i]], stops_[bus->stops[i - 1]]);
				}
			}

			bus->curvature = (bus->route_length / len_by_coordinates) * 1.0;
		}

		render_ = std::make_unique<MapRenderer>(stops_, buses_, render_settings);
		map_ = render_->render_map();


		router_ = std::make_unique<TransportRouter>(stops_,buses_,router_settings);
		router_->BuildGraph();
	}

	std::optional<Stop> TransportCatalogue::GetStopInfo(std::string_view stop_name) {
		auto result = StopByName(stop_name);
		if (result == nullptr) return {};
		return *result;
	}

	std::optional<Bus> TransportCatalogue::GetBusInfo(std::string_view bus_name) {
		auto result = BusByName(bus_name);
		if (result == nullptr) return {};
		return *result;
	}

	std::shared_ptr<Stop> TransportCatalogue::StopByName(std::string_view name) {
		auto it = stops_.find(name);
		if (it == stops_.end()) return nullptr;
		return it->second;
	}
	std::shared_ptr<Bus> TransportCatalogue::BusByName(std::string_view name) {
		auto it = buses_.find(name);
		if (it == buses_.end()) return nullptr;
		return it->second;
	}

	const std::string& TransportCatalogue::GetMap() {
		return map_;
	}

	std::shared_ptr<std::vector<RouteItem>> TransportCatalogue::findRouteInBase(std::string_view from, std::string_view to) {
		return router_->findRoute(from.data(), to.data());
	}

	std::string TransportCatalogue::Serialize() const {
		TCProto::TransportCatalogue db_proto;

		for (const auto& [name, stop] : stops_) {
			TCProto::Stop& proto_stop = *db_proto.add_map_stops();
			proto_stop.set_name(stop->name);
			proto_stop.set_lat(stop->coordinates.lat);
			proto_stop.set_lng(stop->coordinates.lng);

			for (const std::string_view& bus_name : stop->buses) {
				proto_stop.add_bus_name(std::string(bus_name));
			}

			for (const auto& [name, len] : stop->road_distanse) {
				auto& road =  *proto_stop.add_road_distanse();
				road.set_name(name);
				road.set_len(len);
			}
		}


		for (const auto& [name, bus] : buses_) {
			TCProto::Bus& proto_bus = *db_proto.add_map_buses();
			proto_bus.set_name(bus->name);
			proto_bus.set_is_roundtrip(bus->is_roundtrip);
			proto_bus.set_stop_count(bus->stop_count);
			proto_bus.set_unique_stop_count(bus->unique_stop_count);
			proto_bus.set_route_length(bus->route_length);
			proto_bus.set_curvature(bus->curvature);

			for (const std::string& stop : bus->stops) {
				proto_bus.add_stops_name(stop);
			}
		}

		db_proto.set_catalogue_map(map_);

		render_->SerializeSettings(*db_proto.mutable_renderer_settings());

		router_->SerializeSettings(*db_proto.mutable_routing_settings());
		router_->SerializeData(*db_proto.mutable_router());

		return db_proto.SerializeAsString();
	}


	void TransportCatalogue::Deserialize(const std::string& data) {
		TCProto::TransportCatalogue proto;
		assert(proto.ParseFromString(data));


		for (const TCProto::Stop& proto_stop : proto.map_stops()) {
			Stop stop;
			stop.name = proto_stop.name();
			stop.coordinates = { proto_stop.lat(), proto_stop.lng() };

			for (const auto& name : proto_stop.bus_name()) {
				stop.buses.insert(name);
			}

			for (const auto& road : proto_stop.road_distanse()) {
				stop.road_distanse[road.name()] = road.len();
			}

			auto ptr = std::make_shared<Stop>(stop);
			stops_[ptr->name] = ptr;
		}

		for (const TCProto::Bus& proto_bus : proto.map_buses()) {
			Bus bus;

			bus.name = proto_bus.name();
			bus.is_roundtrip = proto_bus.is_roundtrip();
			bus.stop_count = proto_bus.stop_count();
			bus.unique_stop_count = proto_bus.unique_stop_count();
			bus.route_length = proto_bus.route_length();
			bus.curvature = proto_bus.curvature();

			for (const auto& name : proto_bus.stops_name()) {
				bus.stops.push_back(name);
			}

			auto ptr = std::make_shared<Bus>(bus);
			buses_[ptr->name] = ptr;
		}


		render_ = std::make_unique<MapRenderer>(stops_, buses_, MapRenderer::DeserializeSettings(proto.renderer_settings()));
		map_ = proto.catalogue_map();

		router_ = std::make_unique<TransportRouter>(stops_, buses_, TransportRouter::DeserializeSettings(proto.routing_settings()));
		router_->DeserializeData(proto.router());

	}
}