#include "transport_catalogue.h"

// public:
namespace transport_catalogue {
void TransportCatalogue::AddStop(const std::string_view name,
                                 double lat, double lon) {
    stops_.push_back({std::string(name), lat, lon});
    stops_map_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::ToEstablishStopDistances(const std::string_view name,
        const std::vector<std::pair<std::string, int>>& distances) {
    if (stops_map_.count(name) == 0) {
        throw std::invalid_argument("Invalid Stop in Distances");
    }
    const auto& from_stop = stops_map_.at(name);
    for (auto& value : distances) {
        if (stops_map_.count(value.first) == 0) {
            throw std::invalid_argument("Invalid Stop in Distances");
        }
        distances_[{from_stop, stops_map_.at(value.first)}] =
            value.second;
    }
}

void TransportCatalogue::AddBus(const std::string_view name,
        bool annular,
        const std::vector<std::string>& stops) {
    Bus bus;
    for (auto& stop : stops) {
        if (stops_map_.count(stop) == 0) {
            throw std::invalid_argument("Invalid Stop in Bus");
        }
        bus.stops.push_back(stops_map_.at(stop));
    }
    if (!annular) {
        for (auto it = stops.rbegin() + 1;
             it != stops.rend(); ++it) {
            bus.stops.push_back(stops_map_.at(*it));
        }
    }
    bus.name = std::string(name);

    buses_.push_back(bus);
    buses_map_[buses_.back().name] = &buses_.back();
    InsertBusesToStop(&buses_.back());
}

const Stop*
TransportCatalogue::FindStop(const std::string_view name) const {
    return (stops_map_.count(name) > 0) ?
            stops_map_.at(name) : nullptr;
 }

const Bus*
TransportCatalogue::FindBus(const std::string_view name) const {
    return (buses_map_.count(name) > 0) ?
            buses_map_.at(name) : nullptr;
 }

int TransportCatalogue::СalculationDistanceBetweenStops(
    const std::string_view stop_from, const std::string_view stop_to) const {
    if (stops_map_.count(stop_from) == 0 || stops_map_.count(stop_to) == 0) {
        throw std::invalid_argument("Invalid Stop in Distances");
    }
    return СalculationDistanceBetweenStops(stops_map_.at(stop_from),
                                stops_map_.at(stop_to));
}

int TransportCatalogue::СalculationDistanceBetweenStops(
    Stop* stop_from, Stop* stop_to) const {
    auto pair_stop = std::pair(stop_from, stop_to);
    if (distances_.count(pair_stop) > 0) {
        return distances_.at(pair_stop);
    }
    pair_stop = std::pair(stop_to, stop_from);
    if (distances_.count(pair_stop) > 0) {
        return distances_.at(pair_stop);
    }
    return 0;
}

double 
TransportCatalogue::CalculateRouteGeoLength(const std::string_view name) const {
    if (buses_map_.count(name) == 0) {
        return 0.0;
    }
    double result = 0.0;
    double prev_latitude;
    double prev_longitude;
    bool is_first_stop = true;
    for (auto& stop : buses_map_.at(name)->stops) {
        if (is_first_stop) {
            prev_latitude = stop->coordinates_.lat;
            prev_longitude = stop->coordinates_.lng;
            is_first_stop = false;
            continue;
        }
        result += geo::ComputeDistance(
                  {prev_latitude, prev_longitude},
                  {stop->coordinates_.lat, stop->coordinates_.lng});
        prev_latitude = stop->coordinates_.lat;
        prev_longitude = stop->coordinates_.lng;
    }
    return result;
}

int 
TransportCatalogue::CalculateRouteLength(const std::string_view name) const {
    if (buses_map_.count(name) == 0) {
        return 0;
    }
    int result = 0;
    Stop* from_stop;
    bool is_first_stop = true;
    for (auto& stop : buses_map_.at(name)->stops) {
        if (is_first_stop) {
            from_stop = stop;
            is_first_stop = false;
            continue;
        }
        result += СalculationDistanceBetweenStops(from_stop, stop);
        from_stop = stop;
    }
    return result;
}
    
const Bus_info
TransportCatalogue::GetBusInfo(std::string_view name) const {
    int route_stops = 0;
    int unique_stops = 0;
    int length = 0;
    double curvature = 0.0;
    if (buses_map_.count(name) > 0) {
        const auto& stops = buses_map_.at(name)->stops;
        route_stops = stops.size();
        std::unordered_set<Stop*> tmp_stops = {stops.begin(),
                                               stops.end()};
        unique_stops = tmp_stops.size();
        length = CalculateRouteLength(name);
        curvature = length / CalculateRouteGeoLength(name);
    }

    return {std::string(name), route_stops, unique_stops,
            length, curvature};
}

const Stop_info
TransportCatalogue::GetStopInfo(std::string_view name) const {
    if (stops_map_.count(name) > 0) {
        Stop* stop = stops_map_.at(name);
        if (stop_buses_map_.count(stop) > 0) {
            auto& buses_at_stop = stop_buses_map_.at(stop);
            return {std::string(name), true,
                    {buses_at_stop.begin(), buses_at_stop.end()}};
        } else {
            return {std::string(name), true, {}};
        }
    }
    return {std::string(name), false, {}};
}

const std::unordered_map<std::string_view, Stop*>&
TransportCatalogue::GetStops() const {
    return stops_map_;
}

const std::unordered_map<std::string_view, Bus*>&
TransportCatalogue::GetBuses() const {
    return buses_map_;
}

const std::unordered_map<Stop*, set_BUS>&
TransportCatalogue::GetStopBuses() const {
    return stop_buses_map_;
}

const std::unordered_map<std::pair<Stop*, Stop*>, int, TwoStopsHasher>&
TransportCatalogue::GetDistances() const {
    return distances_;
}

void TransportCatalogue::Clear() {
    for (auto& [_, bus] : stop_buses_map_) {
        bus.clear();
    }
    stop_buses_map_.clear();

    distances_.clear();

    stops_map_.clear();
    stops_.clear();

    for (auto& [_, bus] : buses_map_) {
        bus->stops.clear();
    }
    buses_map_.clear();
    buses_.clear();
}

// private:

void TransportCatalogue::InsertBusesToStop(Bus* bus) {
    for (const auto& stop : bus->stops) {
        stop_buses_map_[stop].insert(bus);
    }
}
}