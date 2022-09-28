#include "transport_catalogue.h"

#include <numeric>
#include <unordered_set>
#include <iomanip>
#include <iostream>

namespace TransportCatalogue {

    using namespace std::literals;

    void TransportCatalogue::AddStop(std::string_view name, geo::Coordinates coordinates_) {
        stops_.push_back({ name, std::move(coordinates_) });
        const Stop* st = &stops_[stops_.size() - 1];
        names_stops_.insert({ (*st).name_stop , std::move(st) });
        buses_for_stops_[(*st).name_stop];
    }

    void TransportCatalogue::AddBus(std::string_view name, bool loop, const std::vector<std::string>& stops_buses) {
        int num_stops = static_cast<int>(stops_buses.size());
        Bus bus(name, loop, { (loop) ? num_stops : (num_stops * 2 - 1), GetUniqueStops(stops_buses) });
        buses_.push_back(std::move(bus));
        const Bus* st = &buses_[buses_.size() - 1];
        (names_buses_).insert({ (*st).name_bus , std::move(st) });
        ((buses_[buses_.size() - 1]).stops_for_bus_).reserve(stops_buses.size());
        for (std::string_view stop_ : stops_buses) {
            ((buses_[buses_.size() - 1]).stops_for_bus_).push_back(names_stops_.at(stop_)->name_stop);
            (buses_for_stops_.at(stop_)).insert(buses_[buses_.size() - 1].name_bus);
        }

    }

    const Bus* TransportCatalogue::FindBus(std::string_view name) const {
        return names_buses_.at(name);
    }

    const Stop* TransportCatalogue::FindStop(std::string_view name) const {
        return names_stops_.at(name);
    }

    detail::InformationBus TransportCatalogue::GetInformationBus(std::string_view name) const {
        if (names_buses_.count(name) == 0) {
            return {};
        }
        int distance = ComputeRealDistanceBus(name);
        return { (*FindBus(name)).static_infom, distance, static_cast<double>(distance) / ComputeStraightDistanceBus(name) };
    }

    std::pair<bool, std::set<std::string>> TransportCatalogue::GetInformationStop(std::string_view name) const {
        if (names_stops_.count(name) == 0) {
            return { false, {} };
        }
        return { true, buses_for_stops_.at(std::string(name)) };
    }

    void TransportCatalogue::SetDistance(const std::string& from_, const std::string& where_, int distance_) {
        distances_[{FindStop(std::string_view(from_)), FindStop(where_)}] = distance_;
    }

    int TransportCatalogue::GetDistance(const Stop* stop1, const Stop* stop2) const {
        if (distances_.count({ stop1, stop2 }) == 0) {
            return 0;
        }
        return distances_.at({ stop1, stop2 });
    }

    int TransportCatalogue::GetDistanceInAnyDirection(std::string_view stop1, std::string_view stop2) const {
        const Stop* st1 = FindStop(stop1);
        const Stop* st2 = FindStop(stop2);
        int dist = GetDistance(st1, st2);
        if (dist == 0) {
            dist = GetDistance(st2, st1);
        }
        return dist;
    }

    geo::Coordinates TransportCatalogue::GetCoordinates(std::string_view stop) const {
        return{ (FindStop(stop)->coordinates).lat, (FindStop(stop)->coordinates).lng };
    }

    double TransportCatalogue::ComputeStraightDistanceBus(std::string_view name) const {
        double itog = 0.00;
        const std::vector<std::string_view>& stops = FindBus(name)->stops_for_bus_;
        for (size_t i = 0; i < stops.size() - 1; i++) {

            itog += ComputeDistance(GetCoordinates(stops[i]), GetCoordinates(stops[i + 1]));
        }
        if (!(*FindBus(name)).looping) {
            itog *= 2.00;
        }
        return itog;
    }

    int TransportCatalogue::GetUniqueStops(const std::vector<std::string>& stops_buses) const {
        return static_cast<int>(std::unordered_set<std::string_view>(stops_buses.begin(), stops_buses.end()).size());
    }

    int TransportCatalogue::ComputeRealDistanceBus(std::string_view name_bus) const {
        int result = 0;
        const std::vector<std::string_view>& stops = FindBus(name_bus)->stops_for_bus_;
        bool loop = (*FindBus(name_bus)).looping;
        for (size_t i = 0; i < stops.size() - 1; i++) {
            result += GetDistanceInAnyDirection(stops[i], stops[i + 1]);
            if (!loop) {
                result += GetDistanceInAnyDirection(stops[i + 1], stops[i]);
            }
        }
        return result;
    }

    TransportCatalogue::BusesAndStops TransportCatalogue::InfoForMap() const {
        std::set<const Bus*, detail::BusHasher> buses_for_map;
        std::map<std::string_view, const Stop*> stops_for_map;
        for (auto& elem : names_buses_) {
            buses_for_map.insert(elem.second);
            for (const auto& e : elem.second->stops_for_bus_) {
                stops_for_map.insert({ e ,names_stops_.at(e) });
            }
        }
        return make_pair(std::move(buses_for_map), std::move(stops_for_map));
    }

    const std::deque<Stop>& TransportCatalogue::GetStopsConst() const {
        return stops_;
    }

    const std::deque<Bus>& TransportCatalogue::GetBusesConst() const {
        return buses_;
    }

    const std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::HasherPairStops>& TransportCatalogue::GetDistancesConst() const {
        return distances_;
    }

}