#include <fstream>
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "transport_router.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        TransportCatalogue::TransportCatalogue catalog;
        rendering::MapRenderer renderer;
        TransportCatalogue::transport_router::TransportRouter router(catalog);
        RequestHandler::RequestHandler reader(catalog, renderer, router);
        reader.ReadInput(std::cin);
        reader.FillTransportCatalogue();
        reader.SerializeCatalog();
    } else if (mode == "process_requests"sv) {
        TransportCatalogue::TransportCatalogue catalog;
        rendering::MapRenderer renderer;
        TransportCatalogue::transport_router::TransportRouter router(catalog);
        RequestHandler::RequestHandler reader(catalog, renderer, router);
        reader.ReadInput(std::cin); 
        reader.DeserializeCatalog();
        reader.FillTransportRouter();
        reader.PrintRequests(std::cout);
    } else {
        PrintUsage();
        return 1;
    }
}