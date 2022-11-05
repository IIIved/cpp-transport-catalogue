#include "domain.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>
#include <fstream>
#include <string_view>
#include <string>
#include <sstream>
#include <cassert>

using namespace std;
using namespace transport::catalogue;
using namespace transport::response;
using namespace json::reader;

string ReadFile(const string& file_name) {
	ifstream file(file_name, ios::binary | ios::ate);
	const ifstream::pos_type end = file.tellg();
	file.seekg(0, ios::beg);

	string str(end, '\0');
	file.read(&str[0], end);
	return str;
}

int main(int argc, const char* argv[]) {
	if (argc != 2) {
		cerr << "Usage: transport_catalogue [make_base|process_requests]\n"s;
		return 5;
	}

	const string_view mode(argv[1]);

	const auto input_doc = json::Load(cin);
	const auto& input_map = input_doc.GetRoot().AsMap();

	if (mode == "make_base") {

		std::vector<std::shared_ptr<Stop>> stops = json::reader::ParseStop(input_map.at("base_requests").AsArray());
		std::vector<std::shared_ptr<Bus>> buses = json::reader::ParseBus(input_map.at("base_requests").AsArray());

		TransportCatalogue mainBD(
			stops,
			buses,
			json::reader::ParseRenderSetting(input_map.at("render_settings").AsMap()),
			json::reader::ParseRouterSetting(input_map.at("routing_settings").AsMap())
		);

		const string& file_name = input_map.at("serialization_settings").AsMap().at("file").AsString();
		ofstream file(file_name);
		file << mainBD.Serialize();
	}
	else if (mode == "process_requests") {

		const string& file_name = input_map.at("serialization_settings").AsMap().at("file").AsString();
		
		TransportCatalogue mainBD;
		mainBD.Deserialize(ReadFile(file_name));

		RequestHelper requests(mainBD, input_map.at("stat_requests").AsArray());
		requests.GetResponses();
		requests.PrintResponse(std::cout);

	}
	return 0;
}