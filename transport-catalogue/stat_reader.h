#pragma once

#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include "geo.h"

using namespace transport_catalogue;

namespace stat_reader {
    
void PrintInfo(TransportCatalogue& db,
    const std::vector<std::pair<std::string, std::string>>
    queries, std::ostream& out = std::cout);

void PrintBusInfo(const Bus_info& bus,
                  std::ostream& out = std::cout);
void PrintStopInfo(const Stop_info& stop,
                   std::ostream& out = std::cout);

void PrintStops(const TransportCatalogue& tc,
                std::ostream& out = std::cout);
void PrintBuses(const TransportCatalogue& tc,
                std::ostream& out = std::cout);
void PrintDistances(const TransportCatalogue& tc,
                    std::ostream& out = std::cout);
}