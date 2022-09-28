#pragma once

#include "geo.h"
#include <string>
#include <iostream>
#include <string_view>
#include <utility>
#include <vector>

namespace TransportCatalogue {

    struct Stop {

        Stop(std::string_view name, geo::Coordinates coordinates_);
        Stop(Stop&& other) = default;

        bool operator==(const Stop& other) const;

        std::string name_stop;
        geo::Coordinates coordinates;
    };

    struct BusStaticInformation {

        BusStaticInformation() = default;
        BusStaticInformation(int number, int number_un)
            :number_stops(number), number_stops_un(number_un) {
        }

        int number_stops = 0;
        int number_stops_un = 0;
    };

    struct Bus {

        Bus(std::string_view name, bool loop, BusStaticInformation inform);
        Bus(Bus&& other) = default;

        bool operator==(const Bus& other) const;

        std::vector<std::string_view> stops_for_bus_ = {};
        std::string name_bus;
        bool looping;
        BusStaticInformation static_infom;
    };

    struct BusTimesSettings {
        double bus_wait_time_ = 0.0;
        double bus_velocity_m_m_ = 0.0; //meters per minute
    };

    namespace detail {

        struct InformationBus {

            InformationBus() = default;
            InformationBus(BusStaticInformation inform, int di, double curv_)
                : static_infom_bus(inform), distance(di), curv(curv_) {
            }

            BusStaticInformation static_infom_bus;
            int distance = 0;
            double curv = 0.0;
        };

        bool operator==(const InformationBus& le, const InformationBus& other);

        std::ostream& operator<<(std::ostream& out, const InformationBus& doc);

        struct HasherPairStops {
            size_t operator()(const std::pair<const Stop*, const Stop*>& plate) const;
            std::hash<const void*> hasher;
        };

        struct BusHasher {
            bool operator()(const Bus* lhs, const Bus* rhs) const;
        };

    } //namespace detail

}//namespace TransportCatalogue 