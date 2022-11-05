#pragma once

#include "svg.h"
#include "domain.h"
#include "geo.h"
#include "json.h"

#include "map_renderer.pb.h"

#include <vector>
#include <string>
#include <array>
#include <math.h>
#include <unordered_map>
#include <algorithm>
#include <optional>


namespace transport::render {
	
	using namespace transport::domains;

	inline const double EPSILON = 1e-6;

	struct RenderSettings {
		RenderSettings() = default;

		double width_ = 0.0;
		double height_ = 0.0;
		double padding_ = 0.0;
		double line_width_ = 0.0;
		double stop_radius_ = 0.0;
		int bus_label_font_size_ = 0;
		double underlayer_width_ = 0.0;
		int stop_label_font_size_ = 0;
		svg::Point bus_label_offset_{ 0 ,0 };
		svg::Point stop_label_offset_{ 0 ,0 };
		svg::Color underlayer_color_{};
		std::vector<svg::Color> color_palette_{};
	};


	class MapScaler {
	public:
		explicit inline MapScaler() = default;

		template <typename TStopIt>
		MapScaler(TStopIt points_begin, TStopIt points_end, double max_width,
			double max_height, double padding)
			: padding_(padding)
		{
			if (points_begin == points_end) {
				return;
			}

			const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
				return lhs.second->coordinates.lng < rhs.second->coordinates.lng;
				});
			min_lon_ = (*left_it).second->coordinates.lng;
			const double max_lon = (*right_it).second->coordinates.lng;

			const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
				return lhs.second->coordinates.lat < rhs.second->coordinates.lat;
				});
			const double min_lat = (*bottom_it).second->coordinates.lat;
			max_lat_ = (*top_it).second->coordinates.lat;

			std::optional<double> width_zoom;
			if (!IsZero(max_lon - min_lon_)) {
				width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
			}

			std::optional<double> height_zoom;
			if (!IsZero(max_lat_ - min_lat)) {
				height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
			}

			if (width_zoom && height_zoom) {
				zoom_ = std::min(width_zoom.value(), height_zoom.value());
			}
			else if (width_zoom) {
				zoom_ = width_zoom.value();
			}
			else if (height_zoom) {
				zoom_ = height_zoom.value();
			}
		}

		bool IsZero(double value) {
			return std::abs(value) < EPSILON;
		}

		svg::Point operator()(geo::Coordinates coords) const {
			return { (coords.lng - min_lon_) * zoom_ + padding_,
				(max_lat_ - coords.lat) * zoom_ + padding_ };
		}

	private:
		double padding_ = 0.0;
		double min_lon_ = 0.0;
		double max_lat_ = 0.0;
		double zoom_ = 0.0;
	};


	class MapRenderer {
	public:

		MapRenderer(
			std::map<std::string_view, std::shared_ptr<Stop>>& stops_unique,
			std::map<std::string_view, std::shared_ptr<Bus>>& buses_unique,
			RenderSettings settings
		);

		std::string render_map() const;
		
		void SerializeSettings(TCProto::RenderSettings& proto);
		static RenderSettings DeserializeSettings(const TCProto::RenderSettings& proto);

	private:
		const std::map<std::string_view, std::shared_ptr<Stop>>& stops_unique_;
		const std::map<std::string_view, std::shared_ptr<Bus>>& buses_unique_;
		const RenderSettings settings_;
		
		std::unique_ptr<MapScaler> scaler_;

		void CreateRoutes(svg::Document& result) const;
		void CreateRouteNumber(svg::Document& result) const;
		void CreateStopPoint(svg::Document& result) const;
		void CreateStopName(svg::Document& result) const;
		
	};
	
}