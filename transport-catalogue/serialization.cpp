#include "serialization.h"
#include <fstream>

namespace serialization {

	using cont_distances = std::unordered_map<std::pair<const TransportCatalogue::Stop*, const TransportCatalogue::Stop*>, int, TransportCatalogue::detail::HasherPairStops>;
	using ColorSvg = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

	void Serializator::SetSerializationSettings(SerializationSettings&& set) {
		catalog_set = set;
	}

	transport_catalogue_proto::Rgba GetFillRgbaProto(svg::Rgba rgba) {
		transport_catalogue_proto::Rgba rgba_proto;
		rgba_proto.set_b(rgba.blue);
		rgba_proto.set_g(rgba.green);
		rgba_proto.set_r(rgba.red);
		rgba_proto.set_a(rgba.opacity);
		return rgba_proto;
	}

	transport_catalogue_proto::Rgb GetFillRgbProto(svg::Rgb rgb) {
		transport_catalogue_proto::Rgb rgb_proto;
		rgb_proto.set_b(rgb.blue);
		rgb_proto.set_g(rgb.green);
		rgb_proto.set_r(rgb.red);
		return rgb_proto;
	}

	transport_catalogue_proto::StringColor GetFillStringColorProto(const std::string& str) {
		transport_catalogue_proto::StringColor string_proto;
		*string_proto.mutable_string_color() = str;
		return string_proto;
	}

	ColorSvg GetFillColorSvg(transport_catalogue_proto::Color color_p) {
		ColorSvg color;
		if (color_p.has_rgb()) {
			transport_catalogue_proto::Rgb rgb_proto = color_p.rgb();
			svg::Rgb rgb;
			rgb.blue = rgb_proto.b();
			rgb.green = rgb_proto.g();
			rgb.red = rgb_proto.r();
			color = rgb;
		}
		else if (color_p.has_rgba()) {
			transport_catalogue_proto::Rgba rgba_proto = color_p.rgba();
			svg::Rgba rgba;
			rgba.blue = rgba_proto.b();
			rgba.green = rgba_proto.g();
			rgba.red = rgba_proto.r();
			rgba.opacity = rgba_proto.a();
			color = rgba;
		}
		else if (color_p.has_str_color()) {
			transport_catalogue_proto::StringColor string_proto = color_p.str_color();
			color = string_proto.string_color();
		}
		return color;
	}

	transport_catalogue_proto::Color GetFillColorProto(ColorSvg color_svg) {
		transport_catalogue_proto::Color color_proto;
		if (std::holds_alternative<svg::Rgb>(color_svg)) {
			*color_proto.mutable_rgb() = GetFillRgbProto(std::get<svg::Rgb>(color_svg));
		}
		else if (std::holds_alternative<svg::Rgba>(color_svg)) {
			*color_proto.mutable_rgba() = GetFillRgbaProto(std::get<svg::Rgba>(color_svg));
		}
		else if (std::holds_alternative<std::string>(color_svg)) {
			*color_proto.mutable_str_color() = GetFillStringColorProto(std::get<std::string>(color_svg));
		}
		return color_proto;
	}

	transport_catalogue_proto::RenderSettings Serializator::GetProtoRenderSettings(RenderSettings sett) const {
		transport_catalogue_proto::RenderSettings set_proto;
		set_proto.set_width(sett.width);
		set_proto.set_height(sett.height);
		set_proto.set_padding(sett.padding);
		set_proto.set_line_width(sett.line_width);
		set_proto.set_stop_radius(sett.stop_radius);
		set_proto.set_bus_label_font_size(sett.bus_label_font_size);
		for (double i : sett.bus_label_offset) {
			set_proto.add_bus_label_offset(i);
		}
		set_proto.set_stop_label_font_size(sett.stop_label_font_size);
		for (double i : sett.stop_label_offset) {
			set_proto.add_stop_label_offset(i);
		}
		*set_proto.mutable_underlayer_color() = GetFillColorProto(sett.underlayer_color);
		
		set_proto.set_underlayer_width(sett.underlayer_width);

		for (const auto& color_ : sett.color_palette) {
			*set_proto.add_color_palette() = GetFillColorProto(color_);
		}
		return set_proto;
	}

	transport_catalogue_proto::BusTimesSettings Serializator::GetBusTimesSettingsProto(TransportCatalogue::BusTimesSettings sett) const {
		transport_catalogue_proto::BusTimesSettings sett_proto;
		sett_proto.set_bus_wait_time(sett.bus_wait_time_);
		sett_proto.set_bus_velocity_m_m(sett.bus_velocity_m_m_);
		return sett_proto;
	}

	RenderSettings Serializator::GetRenderSettings(transport_catalogue_proto::RenderSettings set_proto) const {
		RenderSettings sett;
		sett.width = set_proto.width();
		sett.height = set_proto.height();
		sett.padding = set_proto.padding();
		sett.line_width = set_proto.line_width();
		sett.stop_radius = set_proto.stop_radius();
		sett.bus_label_font_size = set_proto.bus_label_font_size();
		int id = 0;
		for (double i : set_proto.bus_label_offset()) {
			sett.bus_label_offset[id++] = i;
		}
		sett.stop_label_font_size = set_proto.stop_label_font_size();
		id = 0;
		for (double i : set_proto.stop_label_offset()) {
			sett.stop_label_offset[id++] = i;
		}
		sett.underlayer_width = set_proto.underlayer_width();
		sett.underlayer_color = GetFillColorSvg(set_proto.underlayer_color());
		sett.color_palette.reserve(set_proto.color_palette_size());
		for (transport_catalogue_proto::Color color_p : set_proto.color_palette()) {
			sett.color_palette.push_back(GetFillColorSvg(color_p));
		}
		return sett;
	}

	TransportCatalogue::BusTimesSettings Serializator::GetBusTimesSettings(transport_catalogue_proto::BusTimesSettings sett_proto) const {
		TransportCatalogue::BusTimesSettings sett;
		sett.bus_wait_time_ = sett_proto.bus_wait_time();
		sett.bus_velocity_m_m_ = sett_proto.bus_velocity_m_m();
		return sett;
	}

	void Serializator::CatalogueSerialize(const TransportCatalogue::TransportCatalogue& catalog,
		                                  TransportCatalogue::BusTimesSettings time_settings,
		                                  RenderSettings settings) const {
		std::ofstream fout(catalog_set.file_name, std::ios::binary);
		transport_catalogue_proto::TransportCatalogue cat_proto;
		const std::deque<TransportCatalogue::Stop>& stops = catalog.GetStopsConst();
		const std::deque<TransportCatalogue::Bus>& buses = catalog.GetBusesConst();
		const cont_distances& dists = catalog.GetDistancesConst();

		std::map<std::string_view, uint32_t> indexes_st;
		uint32_t id = 0;
		size_t size = stops.size();
		for (const TransportCatalogue::Stop& st : stops) {
			transport_catalogue_proto::Stop* st_proto = cat_proto.add_stops();
			transport_catalogue_proto::Coordinates coord;
			coord.set_lat(st.coordinates.lat);
			coord.set_lng(st.coordinates.lng);
			st_proto->set_name(st.name_stop);
			*(st_proto->mutable_coordinates()) = coord;
			indexes_st[st.name_stop] = id++;
		}

		for (const TransportCatalogue::Bus& bs : buses) {
			transport_catalogue_proto::Bus* bs_proto = cat_proto.add_buses();
			for (const auto& st : bs.stops_for_bus_) {
				bs_proto->add_ind_stops(indexes_st[st]);
			}
			bs_proto->set_looping(bs.looping);
			bs_proto->set_name(bs.name_bus);
		}

		for (const auto& para : dists) {
			transport_catalogue_proto::Distance* ds_proto = cat_proto.add_distances();
			ds_proto->set_from(indexes_st[para.first.first->name_stop]);
			ds_proto->set_to(indexes_st[para.first.second->name_stop]);
			ds_proto->set_distance(para.second);
		}

		*cat_proto.mutable_render_settings() = GetProtoRenderSettings(settings);
		*cat_proto.mutable_time_settings() = GetBusTimesSettingsProto(time_settings);
        
        /*std::vector<std::string> lol_v(48900, "dfhgdfghlf uhgkjdfhgkrhtweit huerhtwuithwuthtueq heuwthewuth ruhid rtrhugjdhgdjrhgrutrhtrutrhtuhe");
		for (const auto& para : lol_v) {
			cat_proto.add_lol(para);
		}*/
        
		cat_proto.SerializeToOstream(&fout);
	}

	transport_catalogue_proto::TransportCatalogue Serializator::CatalogueDeserialize() const {
		std::ifstream input(catalog_set.file_name, std::ios::binary);
		transport_catalogue_proto::TransportCatalogue cat_proto;
		cat_proto.ParseFromIstream(&input);
    
		return cat_proto;
	}

	void Serializator::FillCatalogue(TransportCatalogue::TransportCatalogue& catalog, transport_catalogue_proto::TransportCatalogue cat_proto) const {
		std::map<uint32_t, std::string> ids_stops;
		uint32_t id = 0;
		for (const auto& st : cat_proto.stops()) {
			transport_catalogue_proto::Coordinates coord = st.coordinates();
			geo::Coordinates coordinates(coord.lat(), coord.lng());
			catalog.AddStop(st.name(), coordinates);
			ids_stops[id++] = st.name();
		}

		for (const auto& bs : cat_proto.buses()) {
			std::vector<std::string> stops_for_bus;
			stops_for_bus.reserve(bs.ind_stops_size());
			for (uint32_t id_s : bs.ind_stops()) {
				stops_for_bus.push_back(ids_stops[id_s]);
			}
			catalog.AddBus(bs.name(), bs.looping(), std::move(stops_for_bus));
		}

		for (const auto& dist : cat_proto.distances()) {
			catalog.SetDistance(ids_stops[dist.from()], ids_stops[dist.to()], dist.distance());
		}

	}

}