#include "map_renderer.h"

using namespace std;

namespace rendering {

    void MapRenderer::SetSettings(const RenderSettings& settings) {
        settings_ = settings;
    }

    RenderSettings MapRenderer::GetSettings() const {
        return settings_;
    }

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
    }

    void MapRenderer::Render(std::ostream& out) {
        set<geo::Coordinates> coords;
        for (const auto& [k, v] : stops_for_map_) {
            coords.insert(v->coordinates);
        }
        SphereProjector projector(coords.begin(), coords.end(), settings_.width, settings_.height, settings_.padding);
        RenderBusRoutes(projector);
        RenderRoutesNames(projector);
        RenderStops(projector);
        RenderStopsNames(projector);
        document_render_.Render(out);
    }

    void MapRenderer::SetDateForMap(BusesAndStops&& info) {
        bus_for_map_ = std::move(info.first);
        stops_for_map_ = std::move(info.second);
    }

    void MapRenderer::RenderBusRoutes(const rendering::SphereProjector& projector) {
        int color_index = 0;
        for (const auto& route : bus_for_map_) {
            svg::Polyline line;
            vector<string_view> stops(route->stops_for_bus_.begin(), route->stops_for_bus_.end());
            if (!route->looping) {
                stops.insert(stops.end(), next(route->stops_for_bus_.rbegin()), route->stops_for_bus_.rend());
            }
            for (const auto& s : stops) {
                line.AddPoint(projector(stops_for_map_.at(s)->coordinates));
            }
            line.SetFillColor("none"s);
            line.SetStrokeColor(settings_.color_palette[color_index % settings_.color_palette.size()]);
            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            line.SetStrokeWidth(settings_.line_width);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            if (stops.size() != 0) {
                color_index++;
            }
            document_render_.Add(line);

        }
    }

    void MapRenderer::RenderRoutesNames(const SphereProjector& projector) {
        int color_index = 0;
        svg::Text background_, text;

        for (const auto& route : bus_for_map_) {
            vector<string_view> stops = { route->stops_for_bus_.front() };
            if (route->stops_for_bus_.front() != route->stops_for_bus_.back()) {
                stops.push_back(route->stops_for_bus_.back());
            }
            for (const auto& s : stops) {
                background_.SetFontFamily("Verdana"s)
                    .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1] })
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontWeight("bold"s)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetData(route->name_bus)
                    .SetPosition(projector(stops_for_map_.at(s)->coordinates));
                document_render_.Add(background_);
                text.SetFontFamily("Verdana"s)
                    .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1] })
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontWeight("bold"s)
                    .SetFillColor(settings_.color_palette[color_index % settings_.color_palette.size()])
                    .SetData(route->name_bus)
                    .SetPosition(projector(stops_for_map_.at(s)->coordinates));
                document_render_.Add(text);
            }
            color_index++;
        }
    }

    void MapRenderer::RenderStops(const SphereProjector& projector) {
        svg::Circle circle;
        for (const auto& [_, stop] : stops_for_map_) {
            circle.SetRadius(settings_.stop_radius)
                .SetFillColor("white"s)
                .SetCenter(projector(stop->coordinates));
            document_render_.Add(circle);
        }
    }

    void MapRenderer::RenderStopsNames(const SphereProjector& projector) {
        svg::Text text, background_;
        for (const auto& [k, v] : stops_for_map_) {
            background_.SetFontFamily("Verdana"s)
                .SetOffset({ settings_.stop_label_offset[0], settings_.stop_label_offset[1] })
                .SetFontSize(settings_.stop_label_font_size)
                .SetStrokeColor(settings_.underlayer_color)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData(v->name_stop)
                .SetPosition(projector(v->coordinates));
            document_render_.Add(background_);
           
            text.SetFontFamily("Verdana"s)
                .SetOffset({ settings_.stop_label_offset[0], settings_.stop_label_offset[1] })
                .SetFontSize(settings_.stop_label_font_size)
                .SetFillColor("black"s)
                .SetData(v->name_stop)
                .SetPosition(projector(v->coordinates));
            document_render_.Add(text);
        }
    }

}//namespace rendering