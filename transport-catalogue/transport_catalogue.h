#pragma once

#include "geo.h"
#include "domain.h"

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <deque>

namespace transport_catalogue {

namespace detail {

struct pair_hash {
    template<class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

}

class TransportCatalogue {
public:
    void AddBusRoute(const Bus &bus);
    void AddStop(const std::string &stop_name, const geo::Coordinates &coordinates);
    void AddDistance(const std::string &from, std::string_view to, int distance);

    const Stop *FindStop(std::string_view stop_name);
    const Bus *FindBus(std::string_view bus);
    std::set<std::string> FindBusByStop(const std::string &stop_name);
    int FindUniqueStopCount(std::string_view bus);

    int GetCalculateDistance(const Stop *first_route, const Stop *second_route);

    geo::Coordinates GetCoordinatesByStop(std::string_view stop_name) const;

private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, const Stop *> get_stops_;

    std::deque<Bus> buses_to_stop_;
    std::unordered_map<std::string_view, const Bus *> get_buses_to_stop_;

    using key_distance = std::pair<const Stop *, const Stop *>;
    std::unordered_map<key_distance, int, detail::pair_hash> distance_between_stop_;

    std::unordered_map<std::string, std::set<std::string>> stop_to_buses_;
    std::unordered_map<std::string_view, size_t> buses_unique_route_;
};

}