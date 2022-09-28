#include "domain.h"

#include <iomanip>

namespace TransportCatalogue {
    using namespace std::literals;

    Stop::Stop(std::string_view name, geo::Coordinates coordinates_)
        : name_stop(std::string(name)), coordinates(coordinates_) {
    }

    bool Stop::operator==(const Stop& other) const {
        return name_stop == other.name_stop;
    }

    Bus::Bus(std::string_view name, bool loop, BusStaticInformation inform)
        : name_bus(std::string(name)), looping(loop), static_infom(inform) {
    }

    bool Bus::operator==(const Bus& other) const {
        return name_bus == other.name_bus;
    }

    namespace detail {

        bool operator==(const InformationBus& le, const InformationBus& other) {
            return le.curv == other.curv && le.distance == other.distance;
        }

        std::ostream& operator<<(std::ostream& out, const InformationBus& doc) {
            out << doc.static_infom_bus.number_stops << " stops on route, "s;
            out << doc.static_infom_bus.number_stops_un << " unique stops, "s;
            out << std::setprecision(6) << doc.distance << " route length, "s;
            out << std::setprecision(6) << doc.curv << " curvature"s;
            return out;
        }

        size_t HasherPairStops::operator()(const std::pair<const Stop*, const Stop*>& plate) const {
            return hasher(plate.first) + hasher(plate.second) * 38;
        }

        bool BusHasher::operator()(const Bus* lhs, const Bus* rhs) const {
            return std::lexicographical_compare(lhs->name_bus.begin(), lhs->name_bus.end(),
                rhs->name_bus.begin(), rhs->name_bus.end());
        }

    }//namespace detail

}//namespace TransportCatalogue