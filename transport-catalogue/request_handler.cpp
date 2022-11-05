#include "request_handler.h"


namespace transport::response {

	using namespace transport::catalogue;
	using namespace transport::router;
	using namespace std::literals;


	RequestHelper::RequestHelper(TransportCatalogue& tc, const json::Array& stat_requests)
		: catalogue_(tc)
	{
		requests_.reserve(stat_requests.size());
		for (const json::Node& n : stat_requests) {
			Request request;
			const auto &node_map =  n.AsMap();
			request.id = node_map.at("id"s).AsInt();

			std::string type = node_map.at("type"s).AsString();
			if (type == "Stop"s) {
				request.name = node_map.at("name"s).AsString();
				request.type = RequestType::STOP;
			}
			else if (type == "Bus"s) {
				request.name = node_map.at("name"s).AsString();
				request.type = RequestType::BUS;
			}
			else if (type == "Map"s) {
				request.type = RequestType::MAP;
			}
			else if (type == "Route"s) {
				request.from = node_map.at("from"s).AsString();
				request.to = node_map.at("to"s).AsString();
				request.type = RequestType::ROUTER;
			}
			else {
				throw json::ParsingError("Request invalid"s);
			}
			requests_.push_back(request);
		}
	}

	void RequestHelper::GetResponses() {
		for (const Request& request : requests_) {
			switch (request.type) {
			case RequestType::STOP: {
				if (auto stop_info = catalogue_.GetStopInfo(request.name); stop_info) {
					responses_.push_back(CreateJsonResponseStop(request.id, *stop_info));
				}
				else {
					responses_.push_back(CreateJsonResponseError(request.id));
				}
			} break;
			case RequestType::BUS: {
				if (auto route_info = catalogue_.GetBusInfo(request.name); route_info) {
					responses_.push_back(CreateJsonResponseBus(request.id, *route_info));
				}
				else {
					responses_.push_back(CreateJsonResponseError(request.id));
				}
			} break;
			case RequestType::MAP: {
				responses_.push_back(CreateJsonResponseMap(request.id, catalogue_.GetMap()));
			} break;
			case RequestType::ROUTER: {
				if (auto graph_router = catalogue_.findRouteInBase(request.from, request.to); graph_router) {
					responses_.push_back(CreateJsonResponseRoute(request.id, graph_router).AsMap());
				}
				else {
					responses_.push_back(CreateJsonResponseError(request.id));
				}

			} break;
			default:
				throw std::logic_error("unknown type");
			}
		}

	}


	void RequestHelper::PrintResponse(std::ostream& out) {
		json::Print(json::Document{ responses_ }, out);
	}

	json::Node RequestHelper::CreateJsonResponseError(const int request_id) {
		return json::Builder{}.
			StartDict().
			Key("request_id"s).Value(request_id).
			Key("error_message"s).Value("not found"s).
			EndDict().
			Build();
	}


	json::Node RequestHelper::CreateJsonResponseStop(const int request_id, const domains::Stop& data) {
		json::Builder builder;
		builder.StartArray();
		for (auto& bus : data.buses) {
			builder.Value(bus);
		}
		json::Node tmp = builder.EndArray().Build();
		return json::Builder{}.
			StartDict().
			Key("request_id"s).Value(request_id).
			Key("buses").Value(tmp.AsArray()).
			EndDict().
			Build();
	}

	json::Node RequestHelper::CreateJsonResponseBus(const int request_id, const domains::Bus data) {
		return json::Builder{}.
			StartDict().
			Key("request_id"s).Value(request_id).
			Key("route_length"s).Value(data.route_length).
			Key("stop_count"s).Value(data.stop_count).
			Key("unique_stop_count"s).Value(data.unique_stop_count).
			Key("curvature"s).Value(data.curvature).
			EndDict().
			Build();
	}

	json::Node RequestHelper::CreateJsonResponseMap(const int request_id, const std::string map_render_data) {
		return json::Builder{}.
			StartDict().
			Key("map"s).Value(map_render_data).
			Key("request_id"s).Value(request_id).
			EndDict().
			Build();
	}

	json::Node RequestHelper::CreateJsonResponseRoute(const int request_id, std::shared_ptr<std::vector<RouteItem>> route) {

		json::Builder builder_;
		builder_.StartArray();
		double total_time = 0.0;
		for (auto it = route->begin(); it != route->end(); it++) {
			total_time += (it->wait_time + it->trip_time);
			builder_.StartDict()
				.Key("type"s).Value("Wait"s)
				.Key("stop_name"s).Value(it->start_stop_idx->name)
				.Key("time"s).Value(it->wait_time / 60)
				.EndDict()

				.StartDict()
				.Key("type"s).Value("Bus"s)
				.Key("bus"s).Value(it->bus->name)
				.Key("span_count"s).Value(it->stop_count)
				.Key("time"s).Value(it->trip_time / 60)
				.EndDict();
		}
		json::Node x = builder_.EndArray().Build();

		return json::Builder{}
			.StartDict()
			.Key("request_id"s).Value(request_id)
			.Key("total_time"s).Value(total_time / 60)
			.Key("items"s).Value(x.AsArray())
			.EndDict().Build();
	}
}
