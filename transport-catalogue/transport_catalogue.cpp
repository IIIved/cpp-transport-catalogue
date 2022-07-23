#include "transport_catalogue.h"

#include <algorithm>

using namespace transport_catalogue;

void TransportCatalogue::AddBusRoute(const Bus& bus) {
    if (get_buses_to_stop_.find(bus.name) == get_buses_to_stop_.end()) {
        size_t size_unique_route = 0;
        for (const auto& stop_name : bus.stop_names) {
            if (stop_to_buses_[stop_name].find(bus.name) == stop_to_buses_[stop_name].end()) {
                ++size_unique_route;
            }

            stop_to_buses_[stop_name].insert(bus.name);
        }

        buses_to_stop_.push_back({bus.name, bus.stop_names});
        get_buses_to_stop_[buses_to_stop_.back().name] = &buses_to_stop_.back();
        buses_unique_route_[buses_to_stop_.back().name] = size_unique_route;
    }
}

const Stop* TransportCatalogue::FindStop(std::string_view stop_name) {
    auto search = get_stops_.find(stop_name);
    if (search != get_stops_.end()) {
        return get_stops_[stop_name];
    }
    return nullptr;
}

const Bus* TransportCatalogue::FindBus(std::string_view bus) {
    auto search = get_buses_to_stop_.find(bus);
    if (search != get_buses_to_stop_.end()) {
        return get_buses_to_stop_[bus];
    }
    return nullptr;
}

std::set<std::string> TransportCatalogue::FindBusByStop(const std::string& stop_name) {
    std::set<std::string> result;

    auto stop_to_bus = stop_to_buses_.find(stop_name);
    if (stop_to_bus != stop_to_buses_.end()) {
        result = stop_to_buses_[stop_name];
    }

    return result;
}

int TransportCatalogue::FindUniqueStopCount(std::string_view bus_name) {
    int result = 0;

    auto stop_to_bus = buses_unique_route_.find(bus_name);
    if (stop_to_bus != buses_unique_route_.end()) {
        result = buses_unique_route_[bus_name];
    }

    return result;
}

void TransportCatalogue::AddStop(const std::string& stop_name, const geo::Coordinates& coordinates) {
    if (FindStop(stop_name) == nullptr) {
        stops_.push_back({stop_name, {coordinates.lat, coordinates.lng}});
        get_stops_[stops_.back().name] = &stops_.back();
    }
}

void TransportCatalogue::AddDistance(const std::string&  from, std::string_view to, int distance) {
    const Stop* a = FindStop(from);
    const Stop* b = FindStop(to);
    if (a != nullptr && b != nullptr) {
        if (distance_between_stop_.find({a, b}) == distance_between_stop_.end()) {
            distance_between_stop_[{a, b}] = distance;
        }
    }
}

int TransportCatalogue::GetCalculateDistance(const Stop* first_route,
                                             const Stop* second_route) {
    if (distance_between_stop_.find({first_route, second_route}) != distance_between_stop_.end()) {
        return distance_between_stop_[{first_route, second_route}];
    } else {
        return distance_between_stop_[{second_route, first_route}];
    }
}

geo::Coordinates TransportCatalogue::GetCoordinatesByStop(std::string_view stop_name) const {
    geo::Coordinates result{};

    auto search = get_stops_.find(stop_name);
    if (search != get_stops_.end()) {
        result = search->second->coordinates;
        return result;
    }
    return result;
}
