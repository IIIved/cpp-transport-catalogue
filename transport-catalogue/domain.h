#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace transport_catalogue {

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;

        bool operator==(const Stop &other) const;
    };

    struct Bus {
        std::string name;
        std::vector<std::string> stop_names;

        bool operator==(const Bus &other) const;
    };

}