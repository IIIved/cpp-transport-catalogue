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

struct STOP {
    std::string name;
    double latitude;
    double longitude;
};

struct BUS {
    std::string name;
    std::vector<STOP*> stops;
};

struct BUS_Info {
    std::string name;
    int route_stops;
    int unique_stops;
    int length;
    double curvature;
};

struct STOP_Info {
    std::string name;
    bool exists;
    std::vector<BUS*> buses;
};

struct compare_BUS final {
    bool operator() (const BUS* left, const BUS* right) const {
        return left->name < right->name;
    }
};

using set_BUS = std::set<BUS*, compare_BUS>;

struct TwoStopsHasher {
    size_t operator()(const std::pair<STOP*, STOP*> stops) const {
        return reinterpret_cast<size_t>(stops.first)
             + reinterpret_cast<size_t>(stops.second) * PRIME_NUMBER;
    }
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddStop(const std::string_view name,
                 double lat, double lon);
    void AddBus(const std::string_view name, bool annular,
                const std::vector<std::string>& stops);
    
    void AddStopDistances(const std::string_view name,
                          const std::vector<std::pair<std::string, int>>&
                          distances);

    const STOP* FindStop(const std::string_view name) const;
    const BUS* FindBus(const std::string_view name) const;

    int DistanceBetweenStops(const std::string_view stop1,
                             const std::string_view stop2) const;
    int DistanceBetweenStops(STOP* stop1, STOP* stop2) const;

    double RouteGeoLength(const std::string_view name) const;
    int RouteLength(const std::string_view name) const;

    const BUS_Info
    GetBusInfo(std::string_view name) const;
    const STOP_Info
    GetStopInfo(std::string_view name) const;

    const std::unordered_map<std::string_view, STOP*>&
    GetStops() const;
    const std::unordered_map<std::string_view, BUS*>&
    GetBuses() const;
    const std::unordered_map<STOP*, set_BUS>&
    GetStopBuses() const;
    const std::unordered_map<std::pair<STOP*, STOP*>, int, TwoStopsHasher>&
    GetDistances() const;

    void Clear();

private:
    std::deque<STOP> stops_;
    std::unordered_map<std::string_view, STOP*> stops_map_;
    std::deque<BUS> buses_;
    std::unordered_map<std::string_view, BUS*> buses_map_;

    std::unordered_map<STOP*, set_BUS> stop_buses_map_;
    std::unordered_map<std::pair<STOP*, STOP*>, int, TwoStopsHasher>
    distances_;

    void InsertBusesToStop(BUS* bus);
};
    
}
