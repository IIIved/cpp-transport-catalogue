#pragma once

#include "json.h"
#include "geo.h"
#include "svg.h"
#include "domain.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"


#include <string>
#include <map>
#include <vector>

namespace json::reader {
	using namespace transport::domains;
	using namespace transport::render;
	using namespace transport::router;
	using namespace transport::response;

	std::vector<std::shared_ptr<Stop>> ParseStop(const json::Array& base_requests);

	std::vector<std::shared_ptr<Bus>> ParseBus(const json::Array& base_requests);

	svg::Color ParseColor(const json::Node& node);

	transport::render::RenderSettings ParseRenderSetting(const json::Dict& render_settings);

	RoutingSettings ParseRouterSetting(const json::Dict& router_settings);

}
