#include "domain.h"

bool transport_catalogue::Stop::operator==(const transport_catalogue::Stop &other) const {
    if (name != other.name) return false;
    return true;
}

bool transport_catalogue::Bus::operator==(const transport_catalogue::Bus &other) const {
    if (name != other.name) return false;
    return true;
}