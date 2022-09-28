#define _USE_MATH_DEFINES

#include "geo.h"
#include <cmath>

namespace geo {

    bool operator <(const Coordinates& lhs, const Coordinates& rhs) {
        return lhs.lat < rhs.lat || ((lhs.lat - rhs.lat) < 1e-6 && (lhs.lng < rhs.lng));
    }

    double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        static const double dr = M_PI / 180.;
        static const int mean_earth_radius = 6371000;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
            + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
            * mean_earth_radius;
    }

}