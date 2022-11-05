#include "map_renderer.h"
#include "serialization.h"
#include "json.h"

#include "map_renderer.pb.h"

#include <set>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace transport::render {

	
	using namespace transport::domains;
	using namespace std::literals;
	
	MapRenderer::MapRenderer(
		std::map<std::string_view, std::shared_ptr<Stop>>& stops_unique,
		std::map<std::string_view, std::shared_ptr<Bus>>& buses_unique,
		RenderSettings settings
	)
		: stops_unique_(stops_unique)
		, buses_unique_(buses_unique)
		, settings_(settings)
	{
		scaler_ = std::make_unique<MapScaler>(stops_unique_.begin(), stops_unique_.end(), settings_.width_, settings_.height_, settings_.padding_);
	}


	std::string MapRenderer::render_map() const {
		svg::Document result;

		CreateRoutes(result);
		CreateRouteNumber(result);
		CreateStopPoint(result);
		CreateStopName(result);
		
		std::stringstream  render;
		result.Render(render);
		return render.str();
	}

	void MapRenderer::CreateRoutes(svg::Document& result) const {
		size_t color = 0;
		for (const auto& [_, bus] : buses_unique_) {
			if (bus->unique_stop_count < 2) continue;

			svg::Polyline route_line = svg::Polyline()
				.SetStrokeColor(settings_.color_palette_[color])
				.SetStrokeWidth(settings_.line_width_)
				.SetFillColor({})
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			for (const std::string& stop : bus->stops) {
				route_line.AddPoint(scaler_->operator()(stops_unique_.at(stop)->coordinates));
			}

			if (!bus->is_roundtrip) {
				for (int stop = bus->stops.size() - 2; stop > -1; stop--) {
					route_line.AddPoint(scaler_->operator()(stops_unique_.at(bus->stops[stop])->coordinates));
				}
			}

			result.Add(route_line);

			color++;
			if (color == settings_.color_palette_.size())	color = 0;
		}
	}
	
	void MapRenderer::CreateRouteNumber(svg::Document& result) const {
		int color = 0;
		for (const auto& [_, bus] : buses_unique_) {

			std::vector<svg::Text> bus_numbers;

			svg::Text first_background = svg::Text()
				.SetFillColor(settings_.underlayer_color_)
				.SetStrokeColor(settings_.underlayer_color_)
				.SetStrokeWidth(settings_.underlayer_width_)
				.SetFontWeight("bold")
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
				.SetPosition(scaler_->operator()(stops_unique_.at(bus->stops[0])->coordinates))
				.SetOffset(settings_.bus_label_offset_)
				.SetFontSize(settings_.bus_label_font_size_)
				.SetFontFamily("Verdana")
				.SetData(bus->name);
			bus_numbers.push_back(first_background);

			svg::Text first_number = svg::Text()
				.SetFillColor(settings_.color_palette_[color])
				.SetFontWeight("bold")
				.SetPosition(scaler_->operator()(stops_unique_.at(bus->stops[0])->coordinates))
				.SetOffset(settings_.bus_label_offset_)
				.SetFontSize(settings_.bus_label_font_size_)
				.SetFontFamily("Verdana")
				.SetData(bus->name);

			bus_numbers.push_back(first_number);

			if (!bus->is_roundtrip) {
				if (bus->stops.size() > 1 && stops_unique_.at(bus->stops[0])->name != stops_unique_.at(bus->stops[bus->stops.size() - 1])->name) {
					svg::Text second_background = svg::Text()
						.SetFillColor(settings_.underlayer_color_)
						.SetStrokeColor(settings_.underlayer_color_)
						.SetStrokeWidth(settings_.underlayer_width_)
						.SetFontWeight("bold")
						.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
						.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
						.SetPosition(scaler_->operator()(stops_unique_.at(bus->stops[bus->stops.size() - 1])->coordinates))
						.SetOffset(settings_.bus_label_offset_)
						.SetFontSize(settings_.bus_label_font_size_)
						.SetFontFamily("Verdana")
						.SetData(bus->name);
					bus_numbers.push_back(second_background);

					svg::Text second_number = svg::Text()
						.SetFillColor(settings_.color_palette_[color])
						.SetFontWeight("bold")
						.SetPosition(scaler_->operator()(stops_unique_.at(bus->stops[bus->stops.size() - 1])->coordinates))
						.SetOffset(settings_.bus_label_offset_)
						.SetFontSize(settings_.bus_label_font_size_)
						.SetFontFamily("Verdana")
						.SetData(bus->name);

					bus_numbers.push_back(second_number);
				}
			}


			for (auto& number : bus_numbers) {
				result.Add(number);
			}

			color++;
			if (color == settings_.color_palette_.size())  color = 0;
		}
	}

	void MapRenderer::CreateStopPoint(svg::Document& result) const {
		for (const auto& [_, stop] : stops_unique_) {
			svg::Circle point = svg::Circle()
				.SetCenter(scaler_->operator()(stop->coordinates))
				.SetRadius(settings_.stop_radius_)
				.SetFillColor("white"s);
			result.Add(point);
		}
	}

	void MapRenderer::CreateStopName(svg::Document& result) const {
		for (const auto& [_, stop] : stops_unique_) {
			svg::Text background = svg::Text()
				.SetData(stop->name)
				.SetPosition(scaler_->operator()(stop->coordinates))
				.SetOffset(settings_.stop_label_offset_)
				.SetFontSize(settings_.stop_label_font_size_)
				.SetFontFamily("Verdana")
				.SetFontWeight({})
				.SetFillColor(settings_.underlayer_color_)
				.SetStrokeColor(settings_.underlayer_color_)
				.SetStrokeWidth(settings_.underlayer_width_)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			result.Add(background);

			svg::Text number = svg::Text()
				.SetData(stop->name)
				.SetPosition(scaler_->operator()(stop->coordinates))
				.SetOffset(settings_.stop_label_offset_)
				.SetFontSize(settings_.stop_label_font_size_)
				.SetFontFamily("Verdana")
				.SetFontWeight({})
				.SetFillColor("black");

			result.Add(number);
		}
	}
	
	void MapRenderer::SerializeSettings(TCProto::RenderSettings& proto) {
		proto.set_width(settings_.width_);
		proto.set_height(settings_.height_);
		proto.set_padding(settings_.padding_);
		proto.set_stop_radius(settings_.stop_radius_);
		proto.set_line_width(settings_.line_width_);
		proto.set_bus_label_font_size(settings_.bus_label_font_size_);
		proto.set_underlayer_width(settings_.underlayer_width_);
		proto.set_stop_label_font_size(settings_.stop_label_font_size_);
		proto.set_bus_label_offset_x(settings_.bus_label_offset_.x);
		proto.set_bus_label_offset_y(settings_.bus_label_offset_.y);
		proto.set_stop_label_offset_x(settings_.stop_label_offset_.x);
		proto.set_stop_label_offset_y(settings_.stop_label_offset_.y);

		svg::SerializeColor(settings_.underlayer_color_, *proto.mutable_underlayer_color());

		for (const svg::Color& color : settings_.color_palette_) {
			svg::SerializeColor(color, *proto.add_palette());
		}
	
	}

	RenderSettings MapRenderer::DeserializeSettings(const TCProto::RenderSettings& proto) {
		RenderSettings settings;
		settings.width_ = proto.width();
		settings.height_ = proto.height();
		settings.padding_ = proto.padding();
		settings.stop_radius_ = proto.stop_radius();
		settings.line_width_ = proto.line_width();
		settings.bus_label_font_size_ = proto.bus_label_font_size();
		settings.underlayer_width_ = proto.underlayer_width();
		settings.stop_label_font_size_ = proto.stop_label_font_size();
		settings.bus_label_offset_ = { proto.bus_label_offset_x(), proto.bus_label_offset_y() };
		settings.stop_label_offset_ = { proto.stop_label_offset_x(), proto.stop_label_offset_y() };

		settings.underlayer_color_ = svg::DeserializeColor(proto.underlayer_color());

		settings.color_palette_.reserve(proto.palette_size());
		for (const auto& color : proto.palette()) {
			settings.color_palette_.push_back(svg::DeserializeColor(color));
		}
		return settings;
	}

	
}