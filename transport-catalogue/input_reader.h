#pragma once

#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip> 
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

using namespace transport_catalogue;

namespace input_reader {

std::vector<std::pair<std::string, std::string>>
QueriesToDataBase(TransportCatalogue& db);

bool IsIntNumber(std::string_view symbols);

std::string_view Trim(std::string_view value);

std::vector<std::string_view>
Split(const std::string_view line, char delimiter,
      bool trimmed = false);
void Split(const std::string_view line, char delimiter,
           std::vector<std::string>& tokens, bool trimmed = false);

}
