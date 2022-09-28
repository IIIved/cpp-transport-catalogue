#pragma once

#include <set>
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
#include "svg.h"
#include "domain.h"
#include "geo.h"
#include <string>

struct RenderSettings {
    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    double width = 0.0;
    double height = 0.0;
    double padding = 0.0;
    double line_width = 0.0;
    double stop_radius = 0.0;
    int bus_label_font_size = 0;
    double bus_label_offset[2] = { 0.0, 0.0 };
    int stop_label_font_size = 0;
    double stop_label_offset[2] = { 0.0, 0.0 };
    Color underlayer_color;
    double underlayer_width = 0.0;
    std::vector<Color> color_palette;
};

namespace rendering {

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
            double max_height, double padding);

        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRenderer {
    public:
        using BusesAndStops = std::pair<std::set<const TransportCatalogue::Bus*, TransportCatalogue::detail::BusHasher>, std::map<std::string_view, const TransportCatalogue::Stop*>>;

        MapRenderer() = default;
        void Render(std::ostream& out);
        void SetSettings(const RenderSettings&);
        RenderSettings GetSettings() const;
        void SetDateForMap(BusesAndStops&& info);
    private:
        void RenderBusRoutes(const SphereProjector&);
        void RenderRoutesNames(const SphereProjector&);
        void RenderStops(const SphereProjector&);
        void RenderStopsNames(const SphereProjector&);
        RenderSettings settings_;
        svg::Document document_render_;
        std::set<const TransportCatalogue::Bus*, TransportCatalogue::detail::BusHasher> bus_for_map_;
        std::map<std::string_view, const TransportCatalogue::Stop*> stops_for_map_;
    };

    template <typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
        double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lng < rhs.lng;
                });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
            return lhs.lat < rhs.lat;
                });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = static_cast<double>((max_width - 2.0 * static_cast<double>(padding)) / (max_lon - min_lon_));
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = static_cast<double>((max_height - 2.0 * static_cast<double>(padding)) / (max_lat_ - min_lat));
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }

    }

}//namespace renderer