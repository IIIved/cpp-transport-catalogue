#include "domain.h"
#include <iostream>
#include <iomanip>
#include <ostream>

namespace transport::domains {
	using namespace std::literals;

	void Stop::Parse(const json::Dict& request) {
		name = request.at("name"s).AsString();
		coordinates = {
			request.at("latitude"s).AsDouble(),
			request.at("longitude"s).AsDouble()
		};

		for (const auto& [stop_name, distance] : request.at("road_distances"s).AsMap()) {
			road_distanse[stop_name] = distance.AsInt();
		}
	}

	void Bus::Parse(const json::Dict& request) {
		name = request.at("name"s).AsString();
		is_roundtrip = request.at("is_roundtrip"s).AsBool();

		for (const auto& stop_name : request.at("stops"s).AsArray()) {
			stops.push_back(stop_name.AsString());
		}
	}

	int RealLenBeetwenStops(std::shared_ptr<Stop> from, std::shared_ptr<Stop> to) {
		if (from == nullptr || to == nullptr) throw std::invalid_argument("stops no find");

		auto it = from->road_distanse.find(to->name);
		if (it != from->road_distanse.end()) return it->second;

		it = to->road_distanse.find(from->name);
		if (it != to->road_distanse.end()) return it->second;

		return 0;
	}
	
}