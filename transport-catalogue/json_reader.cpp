#include "json_reader.h"


#include <iomanip>
#include <vector>
#include <iostream>
#include <cassert>
#include <sstream>
#include <string>


namespace json::reader {

	using namespace std;

	std::vector<std::shared_ptr<Stop>> ParseStop(const json::Array& base_requests) {
		std::vector<std::shared_ptr<Stop>> result;
		for (const auto& request : base_requests) {
			const std::string& type = request.AsMap().at("type"s).AsString();
			if (type == "Stop"s) {
				auto stop = std::make_shared<Stop>();
				stop->Parse(request.AsMap());
				result.push_back(stop);
			}
		}
		return result;
	}

	std::vector<std::shared_ptr<Bus>> ParseBus(const json::Array& base_requests) {
		std::vector<std::shared_ptr<Bus>> result;
		for (const auto& request : base_requests) {
			const std::string& type = request.AsMap().at("type"s).AsString();
			if (type == "Bus"s) {
				auto bus = std::make_shared<Bus>();
				bus->Parse(request.AsMap());
				result.push_back(bus);
			}
		}
		return result;
	}

	svg::Color ParseColor(const json::Node& node) {
		if (node.IsString()) {
			return svg::Color{ node.AsString() };
		}
		if (node.IsArray()) {

			const auto& node_array = node.AsArray();
			size_t size = node_array.size();
			if (size == 4) {
			
				return svg::Rgba{
					static_cast<uint8_t>(node_array[0].AsInt()),
					static_cast<uint8_t>(node_array[1].AsInt()),
					static_cast<uint8_t>(node_array[2].AsInt()),
					node_array[3].AsDouble() 
				};

			}
			if (size == 3) {
				return svg::Rgb{
					static_cast<uint8_t>(node_array[0].AsInt()),
					static_cast<uint8_t>(node_array[1].AsInt()),
					static_cast<uint8_t>(node_array[2].AsInt()) 
				};
			}
		}
		else {
			throw  std::logic_error("unknown node");
		}
		return svg::Color{};
	}

	transport::render::RenderSettings ParseRenderSetting(const json::Dict& render_settings) {
		transport::render::RenderSettings result;

		result.width_ = render_settings.at("width"s).AsDouble();
		result.height_ = render_settings.at("height"s).AsDouble();
		result.padding_ = render_settings.at("padding"s).AsDouble();
		result.stop_radius_ = render_settings.at("stop_radius"s).AsDouble();
		result.line_width_ = render_settings.at("line_width"s).AsDouble();
		result.bus_label_font_size_ = render_settings.at("bus_label_font_size"s).AsInt();
		result.underlayer_width_ = render_settings.at("underlayer_width"s).AsDouble();
		result.stop_label_font_size_ = render_settings.at("stop_label_font_size"s).AsInt();

		const json::Array& tmp_bus_label_offset = render_settings.at("bus_label_offset"s).AsArray();	
		result.bus_label_offset_.x = tmp_bus_label_offset[0].AsDouble();
		result.bus_label_offset_.y = tmp_bus_label_offset[1].AsDouble();

		const json::Array& tmp_stop_label_offset = render_settings.at("stop_label_offset"s).AsArray();
		result.stop_label_offset_.x = tmp_stop_label_offset[0].AsDouble();
		result.stop_label_offset_.y = tmp_stop_label_offset[1].AsDouble();

		result.underlayer_color_ = ParseColor(render_settings.at("underlayer_color"s));

		for (const auto& color : render_settings.at("color_palette"s).AsArray()) {
			result.color_palette_.push_back(ParseColor(color));
		}
		return result;
	}

	RoutingSettings ParseRouterSetting(const json::Dict& router_settings) {

		RoutingSettings result(
			router_settings.at("bus_wait_time"s).AsInt(),
			router_settings.at("bus_velocity"s).AsDouble()
		);

		return result;
	}

}
