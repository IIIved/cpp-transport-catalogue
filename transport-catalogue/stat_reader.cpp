#include "stat_reader.h"
#include "geo.h"

const int precision = 9;

namespace stat_reader {
    using namespace geo;

void PrintInfo(TransportCatalogue& db,
    const std::vector<std::pair<std::string, std::string>>
    queries, std::ostream& out) {
    for (const auto& query : queries) {
        if (query.first == "Bus") {
            auto bus_info = db.GetBusInfo(query.second);
            PrintBusInfo(bus_info,  out);
            continue;
        }
        if (query.first == "Stop") {
            auto stop_info = db.GetStopInfo(query.second);
            PrintStopInfo(stop_info, out);
        }
    }
}

void PrintBusInfo(const Bus_info& bus,
                  std::ostream& out) {
    out << std::setprecision(precision)
        << "Bus " << bus.name << ": ";
    if (bus.route_stops > 0) {
        out << bus.route_stops << " stops on route, "
            << bus.unique_stops << " unique stops, "
            << bus.length << " route length, "
            << bus.curvature << " curvature";
    } else {
        out << "not found";
    }
    out << std::endl;
}

void PrintStopInfo(const Stop_info& stop, std::ostream& out) {
    out << "Stop " << stop.name << ": ";
    if (stop.is_exists) {
        if (stop.buses.size() > 0) {
            out << "buses";
            for (auto bus : stop.buses) {
                out << " " << bus->name;
            }
        } else {
            out << "no buses";
        }
    } else {
        out << "not found";
    }
    out << std::endl;
}

void PrintStops(const TransportCatalogue& tc,
                std::ostream& out) {
    for (auto& [key, coords] : tc.GetStops()) {
        out << std::setprecision(precision)
            << "Stop [" << key
            << "]: " << coords->coordinates_.lat
            << ", " << coords->coordinates_.lng << std::endl;
    }
}

void PrintBuses(const TransportCatalogue& tc,
                std::ostream& out) {
    for (auto& [key, bus] : tc.GetBuses()) {
        out << "Bus [" << key << "]: ";
        auto count = bus->stops.size();
        for (auto& stop : bus->stops) {
            out << "[" << stop->name << "]"
                << ((--count > 0) ? " > " : "");
        }
        out << std::endl;
    }
}

void PrintDistances(const TransportCatalogue& tc,
                    std::ostream& out) {
    for (auto& [key, distance] : tc.GetDistances()) {
        out << "Distance between [" << key.first->name
            << "] - [" << key.second->name << "] = "
            << distance << std::endl;
    }
}
    
}