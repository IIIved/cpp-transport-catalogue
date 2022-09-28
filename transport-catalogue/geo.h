#pragma once

namespace geo {

    struct Coordinates {

        Coordinates() = default;
        Coordinates(Coordinates&& other) = default;
        Coordinates(const Coordinates& other) = default;
        Coordinates& operator=(Coordinates&& other) = default;
        Coordinates& operator=(const Coordinates& other) = default;

        Coordinates(double l, double r)
            : lat(l), lng(r) {
        }

        double lat = 0.0;
        double lng = 0.0;
    };

    bool operator <(const Coordinates& lhs, const Coordinates& rhs);
    double ComputeDistance(Coordinates from, Coordinates to);

}