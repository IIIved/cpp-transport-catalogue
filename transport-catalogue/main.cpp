#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <fstream>
using namespace input_reader;
using namespace stat_reader;
using namespace std::string_literals;
using namespace std::string_view_literals;
//ParsingInformationAboutTheBus

int main() {


    TransportCatalogue tc;
    auto queries = QueriesToDataBase(tc);   
    PrintInfo(tc, queries);


    return 0;
}