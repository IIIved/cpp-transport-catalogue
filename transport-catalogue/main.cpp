#include <iostream>
#include <optional>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

using namespace std;
using namespace transport_catalogue;
using namespace map_renderer;

int main() {
    TransportCatalogue transportCatalogue;
    MapRenderer mapRenderer;

    json::JsonReader jsonReader(transportCatalogue, mapRenderer);
    json::Node node = jsonReader.Load(cin).GetRoot();

    jsonReader.ParsingBasicQueries(node);

    std::optional getRenderSettings = jsonReader.ParseRenderSettings(node);
    MapRenderer::RenderSettings renderSettings;
    if (getRenderSettings != nullopt) {
        renderSettings = getRenderSettings.value();
    }

    std::vector<json::Node> arr_stat_req_map = jsonReader.GetStatRequests(node, renderSettings);

    std::cout << json::JsonReader::Print(arr_stat_req_map);

//    double WIDTH = node.AsMap().at("render_settings").AsMap().at("width").AsDouble();
//    double HEIGHT = node.AsMap().at("render_settings").AsMap().at("height").AsDouble();
//    double PADDING = node.AsMap().at("render_settings").AsMap().at("padding").AsDouble();
//
//    double line_width  = node.AsMap().at("render_settings").AsMap().at("line_width").AsDouble();
//
//    //Делаю вектор, в котором содержатся все остановки, которые нужно вывести.
//    std::vector<geo::Coordinates> all_coords;
//    for (auto stop : transportCatalogue.stops_) {
//        if (transportCatalogue.FindBusByStop(stop.name).size() != 0) {
//            all_coords.push_back(stop.coordinates);
//        }
//    }
//    //Создаю проектор сферы.
//    const SphereProjector proj{
//
//            all_coords.begin(), all_coords.end(), WIDTH, HEIGHT, PADDING
//    };
//    //Делаю цветовую палитру.
//    std::vector<svg::Color> color_plate;
//    for (const auto& c_p : node.AsMap().at("render_settings").AsMap().at("color_palette").AsArray()) {
//        if (c_p.IsArray()) {
//            if (c_p.AsArray().size() == 3) {
//                svg::Rgb rgb;
//                rgb.red = c_p.AsArray()[0].AsInt();
//                rgb.green = c_p.AsArray()[1].AsInt();
//                rgb.blue = c_p.AsArray()[2].AsInt();
//                color_plate.push_back(rgb);
//            } else if (c_p.AsArray().size() == 4) {
//                svg::Rgba rgba;
//                rgba.red = c_p.AsArray()[0].AsInt();
//                rgba.green = c_p.AsArray()[1].AsInt();
//                rgba.blue = c_p.AsArray()[2].AsInt();
//                rgba.opacity = c_p.AsArray()[3].AsDouble();
//                color_plate.push_back(rgba);
//            }
//        } else {
//            color_plate.push_back(c_p.AsString());
//        }
//    }


//
//    //Рисую документ, в котором учитываются все остановки, через которые прошел автобус.
//    svg::Document doc;
//    int color_i = 0;
//    for (auto bus : transportCatalogue.buses_to_stop_) {
//        svg::Polyline polyline;
//        for (const auto name : bus.stop_names) {
//            auto geo_coord = transportCatalogue.FindStop(name)->coordinates;
//            const svg::Point screen_coord = proj(geo_coord);
//            polyline.AddPoint(screen_coord);
//        }
//        if (color_i == static_cast<int>(color_plate.size())){
//            color_i = 0;
//        }
//        polyline.SetFillColor("none").SetStrokeColor(color_plate[color_i]).SetStrokeWidth(line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
//                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
//        doc.Add(polyline);
//        color_i++;
//    }
//
//
//    doc.Render(std::cout);

//<?xml version="1.0" encoding="UTF-8" ?>
//<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
//  <polyline points="99.2283,329.5 50,232.18 99.2283,329.5" fill="none" stroke="green" stroke-width="14" stroke-linecap="round" stroke-linejoin="round"/>
//  <polyline points="550,190.051 279.22,50 333.61,269.08 550,190.051" fill="none" stroke="rgb(255,160,0)" stroke-width="14" stroke-linecap="round" stroke-linejoin="round"/>
//</svg>


//    jsonReader.PostBaseRequestsInTransportCatalogue(node);
//
//    auto getRenderSettings = jsonReader.ParseRenderSettings(node);
//    MapRenderer::RenderSettings renderSettings;
//    if (getRenderSettings != nullopt) {
//        renderSettings = getRenderSettings.value();
//    }


    // auto array_stat_req = jsonReader.GetStatRequests(node, renderSettings);
    // std::cout << json_reader::JsonReader::Print(array_stat_req);

    return 0;
}