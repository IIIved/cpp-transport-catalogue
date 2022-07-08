#include "transport_catalogue.h"

// public:
namespace transport_catalogue {
void TransportCatalogue::AddStop(const std::string_view name,
                                 double lat, double lon) {
    stops_.push_back({std::string(name), lat, lon});
    stops_map_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddStopDistances(const std::string_view name,
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
    BUS bus;
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



const BUS* 
TransportCatalogue::FindBus(const std::string_view name) const {
    return (buses_map_.count(name) > 0) ?
            buses_map_.at(name) : nullptr;
 }

const STOP*
TransportCatalogue::FindStop(const std::string_view name) const {
    return (stops_map_.count(name) > 0) ?
            stops_map_.at(name) : nullptr;
 }



int TransportCatalogue::DistanceBetweenStops(
    STOP* stop1, STOP* stop2) const {
    auto pair_stop = std::pair(stop1, stop2);
    if (distances_.count(pair_stop) > 0) {
        return distances_.at(pair_stop);
    }
    pair_stop = std::pair(stop2, stop1);
    if (distances_.count(pair_stop) > 0) {
        return distances_.at(pair_stop);
    }
    return 0;
}

int TransportCatalogue::DistanceBetweenStops(
    const std::string_view stop1, const std::string_view stop2) const {
    if (stops_map_.count(stop1) == 0 || stops_map_.count(stop2) == 0) {
        throw std::invalid_argument("Invalid Stop in Distances");
    }
    return DistanceBetweenStops(stops_map_.at(stop1),
                                stops_map_.at(stop2));
}



int 
TransportCatalogue::RouteLength(const std::string_view name) const {
    if (buses_map_.count(name) == 0) {
        return 0;
    }
    int result = 0;
    STOP* from_stop;
    bool is_first_stop = true;
    for (auto& stop : buses_map_.at(name)->stops) {
        if (is_first_stop) {
            from_stop = stop;
            is_first_stop = false;
            continue;
        }
        result += DistanceBetweenStops(from_stop, stop);
        from_stop = stop;
    }
    return result;
}

double
TransportCatalogue::RouteGeoLength(const std::string_view name) const {
    if (buses_map_.count(name) == 0) {
        return 0.0;
    }
    double result = 0.0;
    double prev_latitude;
    double prev_longitude;
    bool is_first_stop = true;
    for (auto& stop : buses_map_.at(name)->stops) {
        if (is_first_stop) {
            prev_latitude = stop->latitude;
            prev_longitude = stop->longitude;
            is_first_stop = false;
            continue;
        }
        result += geo::ComputeDistance(
                  {prev_latitude, prev_longitude},
                  {stop->latitude, stop->longitude});
        prev_latitude = stop->latitude;
        prev_longitude = stop->longitude;
    }
    return result;
}
    


const STOP_Info
TransportCatalogue::GetStopInfo(std::string_view name) const {
    if (stops_map_.count(name) > 0) {
        STOP* stop = stops_map_.at(name);
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

const BUS_Info
TransportCatalogue::GetBusInfo(std::string_view name) const {
    int route_stops = 0;
    int unique_stops = 0;
    int length = 0;
    double curvature = 0.0;
    if (buses_map_.count(name) > 0) {
        const auto& stops = buses_map_.at(name)->stops;
        route_stops = stops.size();
        std::unordered_set<STOP*> tmp_stops = {stops.begin(),
                                               stops.end()};
        unique_stops = tmp_stops.size();
        length = RouteLength(name);
        curvature = length / RouteGeoLength(name);
    }

    return {std::string(name), route_stops, unique_stops,
            length, curvature};
}

const std::unordered_map<std::string_view, STOP*>&
TransportCatalogue::GetStops() const {
    return stops_map_;
}

const std::unordered_map<std::string_view, BUS*>&
TransportCatalogue::GetBuses() const {
    return buses_map_;
}

const std::unordered_map<STOP*, set_BUS>&
TransportCatalogue::GetStopBuses() const {
    return stop_buses_map_;
}

const std::unordered_map<std::pair<STOP*, STOP*>, int, TwoStopsHasher>&
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

void TransportCatalogue::InsertBusesToStop(BUS* bus) {
    for (const auto& stop : bus->stops) {
        stop_buses_map_[stop].insert(bus);
    }
}
}
