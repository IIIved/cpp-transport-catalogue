#pragma once

#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <string>
#include <vector>
#include <variant>

namespace transport::response {

	using namespace transport::catalogue;
	using namespace transport::router;
	using namespace std::literals;

	enum class RequestType
	{
		STOP, BUS, MAP, ROUTER
	};

	struct Request {
		Request() = default;

		int id;
		std::string name = ""s;
		std::string from = ""s;
		std::string to = ""s;
		RequestType type;
	};


	class RequestHelper {
	public:
		RequestHelper(TransportCatalogue& tc, const json::Array& stat_requests);

		void GetResponses();

		void PrintResponse(std::ostream& out);

	private:
		TransportCatalogue& catalogue_;
		std::vector<Request> requests_;
		json::Array responses_;

		json::Node CreateJsonResponseError(const int request_id);

		json::Node CreateJsonResponseStop(const int request_id, const domains::Stop& data);

		json::Node CreateJsonResponseBus(const int request_id, const domains::Bus data);

		json::Node CreateJsonResponseMap(const int request_id, const std::string map_render_data);

		json::Node CreateJsonResponseRoute(const int request_id, std::shared_ptr<std::vector<RouteItem>> route);
	};

}