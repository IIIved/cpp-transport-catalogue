#pragma once

#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>

using namespace transport_catalogue;

namespace stat_reader {
    
void PrintInfo(TransportCatalogue& db,
    const std::vector<std::pair<std::string, std::string>>
    queries, int precision = 9, std::ostream& out = std::cout);

void PrintBusInfo(const BUS_Info& bus, int precision = 9,
                  std::ostream& out = std::cout);
void PrintStopInfo(const STOP_Info& stop,
                   std::ostream& out = std::cout);

void PrintStops(const TransportCatalogue& tc, int precision = 9,
                std::ostream& out = std::cout);
void PrintBuses(const TransportCatalogue& tc,
                std::ostream& out = std::cout);
void PrintDistances(const TransportCatalogue& tc,
                    std::ostream& out = std::cout);
}