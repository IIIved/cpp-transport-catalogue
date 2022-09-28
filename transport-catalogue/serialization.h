#pragma once
#include <transport_catalogue.pb.h>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "domain.h"

namespace serialization {

	struct SerializationSettings {
		std::string file_name;
	};

	class Serializator {
	public:
		Serializator() = default;
		~Serializator() = default;

		void SetSerializationSettings(SerializationSettings&& set);

		void CatalogueSerialize(const TransportCatalogue::TransportCatalogue& catalog,
			                    TransportCatalogue::BusTimesSettings time_settings,
			                    RenderSettings settings) const;
		transport_catalogue_proto::TransportCatalogue CatalogueDeserialize() const;

		void FillCatalogue(TransportCatalogue::TransportCatalogue& catalog, transport_catalogue_proto::TransportCatalogue cat_proto) const;
		RenderSettings GetRenderSettings(transport_catalogue_proto::RenderSettings set_proto) const;
		TransportCatalogue::BusTimesSettings GetBusTimesSettings(transport_catalogue_proto::BusTimesSettings sett_proto) const;

	private:
		SerializationSettings catalog_set;

		transport_catalogue_proto::BusTimesSettings GetBusTimesSettingsProto(TransportCatalogue::BusTimesSettings sett) const;
		transport_catalogue_proto::RenderSettings GetProtoRenderSettings(RenderSettings settings) const;
	};

}//serialization