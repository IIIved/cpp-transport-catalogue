#pragma once

#include <unordered_map>
#include <string>
#include <deque>
#include <string_view>
#include <set>
#include <vector>
#include <utility>
#include <map>

#include "geo.h"
#include "domain.h"

namespace TransportCatalogue {

    class TransportCatalogue {
    public:
        using BusesAndStops = std::pair<std::set<const Bus*, detail::BusHasher>, std::map<std::string_view, const Stop*>>;
        void AddStop(std::string_view name, geo::Coordinates coordinates_);
        void AddBus(std::string_view name, bool loop, const std::vector<std::string>& stops_buses);
        void SetDistance(const std::string& from_, const std::string& where_, int distance_);

        const Bus* FindBus(std::string_view name) const;
        const Stop* FindStop(std::string_view name) const;

        int GetDistance(const Stop* stop1, const Stop* stop2) const;
        int GetDistanceInAnyDirection(std::string_view stop1, std::string_view stop2) const;

        BusesAndStops InfoForMap() const;
        detail::InformationBus GetInformationBus(std::string_view name) const;
        std::pair<bool, std::set<std::string>> GetInformationStop(std::string_view name) const;

        const std::deque<Stop>& GetStopsConst() const;
        const std::deque<Bus>& GetBusesConst() const;
        const std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::HasherPairStops>& GetDistancesConst() const;
        
    private:

        std::deque<Stop> stops_;
        std::deque<Bus> buses_;
      
        std::unordered_map<std::string_view, const Stop*> names_stops_;
        std::unordered_map<std::string_view, const Bus*> names_buses_;
     
        std::unordered_map<std::string_view, std::set<std::string>> buses_for_stops_; 
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::HasherPairStops> distances_; 

        geo::Coordinates GetCoordinates(std::string_view stop) const;
        double ComputeStraightDistanceBus(std::string_view name) const;
        int ComputeRealDistanceBus(std::string_view name_bus) const;
        int GetUniqueStops(const std::vector<std::string>& stops_buses) const;

    };

}//namespace TransportCatalogue