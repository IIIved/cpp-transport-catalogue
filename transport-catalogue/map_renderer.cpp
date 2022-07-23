#include "map_renderer.h"

#include <utility>


namespace map_renderer {

    MapRenderer::MapRenderer(RenderSettings renderSettings)
            : renderSettings_(std::move(renderSettings)) {}

    bool detail::IsZero(double value) {
        return std::abs(value) < detail::EPSILON;
    }

    void MapRenderer::Render(const svg::Document &doc, std::ostream &output) {
        doc.Render(output);
    }
} //namespace map_renderer