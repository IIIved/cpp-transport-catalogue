#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace transport_catalogue {
    

const size_t PRIME_NUMBER = 37;

struct Stop {
    std::string name;
    geo::Coordinates coordinates_;

};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
};

struct Bus_info {
    std::string name;
    int route_stops;
    int unique_stops;
    int length;
    double curvature;
};

struct Stop_info {
    std::string name;
    bool is_exists;
    std::vector<Bus*> buses;
};

struct compare_BUS final {
    bool operator() (const Bus* left, const Bus* right) const {
        return left->name < right->name;
    }
};

using set_BUS = std::set<Bus*, compare_BUS>;

struct TwoStopsHasher {
    size_t operator()(const std::pair<Stop*, Stop*> stops) const {
        return reinterpret_cast<size_t>(stops.first)
             + reinterpret_cast<size_t>(stops.second) * PRIME_NUMBER;
    }
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddStop(const std::string_view name,
                 double lat, double lon);
    void ToEstablishStopDistances(const std::string_view name,
                          const std::vector<std::pair<std::string, int>>&
                          distances);
    void AddBus(const std::string_view name, bool annular,
                const std::vector<std::string>& stops);

    const Stop* FindStop(const std::string_view name) const;
    const Bus* FindBus(const std::string_view name) const;

    int СalculationDistanceBetweenStops(const std::string_view from ,
                             const std::string_view to) const;
    int СalculationDistanceBetweenStops(Stop* from , Stop* to) const;

    double CalculateRouteGeoLength(const std::string_view name) const;
    int CalculateRouteLength(const std::string_view name) const;

    const Bus_info
    GetBusInfo(std::string_view name) const;

    const Stop_info
    GetStopInfo(std::string_view name) const;

    const std::unordered_map<std::string_view, Stop*>&
    GetStops() const;
    const std::unordered_map<std::string_view, Bus*>&
    GetBuses() const;
    const std::unordered_map<Stop*, set_BUS>&
    GetStopBuses() const;
    const std::unordered_map<std::pair<Stop*, Stop*>, int, TwoStopsHasher>&
    GetDistances() const;

    void Clear();

private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stops_map_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> buses_map_;

    std::unordered_map<Stop*, set_BUS> stop_buses_map_;
    std::unordered_map<std::pair<Stop*, Stop*>, int, TwoStopsHasher>
    distances_;

    void InsertBusesToStop(Bus* bus);
};
    
}