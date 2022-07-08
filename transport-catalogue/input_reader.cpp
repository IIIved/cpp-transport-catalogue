#include "input_reader.h"

using namespace std::string_view_literals;

using namespace transport_catalogue;

namespace input_reader {
    
const std::string_view WHITESPACE = " \f\n\r\t\v"sv;

std::vector<std::pair<std::string, std::string>>
QueriesToDataBase(TransportCatalogue& db) {
    std::unordered_map<std::string,
        std::vector<std::pair<std::string, int>>> distances;
    std::unordered_map<std::string,
        std::pair<bool, std::vector<std::string>>> buses;
    std::vector<std::pair<std::string, std::string>> queries;
    std::string line;
    int count = 0;
    std::cin >> count;
    ++count;
    while (count > 0) {
        std::getline(std::cin, line);
        if (line.size() == 0) {
            continue;
        }
        --count;
        std::string_view line_sv = Trim(line);
        
        if (IsIntNumber(line_sv)) {
            count = std::atoi(std::string(line_sv).data());
            continue;
        }

        if (line_sv.find(':') != line_sv.npos) {
            auto tokens = Split(line_sv, ':');
            if (tokens.size() < 2) {
                continue;
            }

            auto pos = tokens[0].find(' ');
            if (pos == tokens[0].npos) {
                continue;
            }

            auto key = tokens[0].substr(0, pos);
            auto name = Trim(tokens[0].substr(pos));

            if (key == "Stop") {
                auto values = Split(tokens[1], ',');
                auto value_size = values.size();
                if (value_size < 2) {
                    continue;
                }
                double lat = 
                std::atof(std::string(values[0]).data());
                double lon = 
                std::atof(std::string(values[1]).data());
 
                db.AddStop(name, lat, lon);
                if (value_size < 3) {
                    continue;
                }

                auto& from_stop = distances[std::string(name)];
                for (int i = 2; i < value_size; ++i) {

                    auto pos = values[i].find("to"sv);
                    auto stop = std::string(Trim(values[i].substr(pos + 2)));
                    int d = std::atoi(
                            std::string(values[i].substr(0, pos)).data());
                    from_stop.push_back({stop, d});
                }
                continue;
            }

            if (key == "Bus") {
                bool round_trip = 
                    (tokens[1].find('-') != tokens[1].npos) ?
                    true : false;
                bool annular_trip = 
                    (tokens[1].find('>') != tokens[1].npos) ?
                    true : false;
                if (!round_trip && !annular_trip) {
                    continue;
                }
                char delimiter = annular_trip ? '>' : '-';
                std::vector<std::string> values;
                Split(tokens[1], delimiter, values, true);
             
                buses[std::string(name)] = {annular_trip,
                                             std::move(values)};
                continue;
            }
            continue;
        }


        auto pos = line_sv.find(' ');
        if (pos == line_sv.npos) {
            queries.push_back({std::string(line_sv), ""});
            continue;
        }
        queries.push_back({std::string(line_sv.substr(0, pos)),
                           std::string(Trim(line_sv.substr(pos)))});
    }

    for (auto& [key, stops] : distances) {
        db.AddStopDistances(key, std::move(stops));
    }

    for (auto& [key, stops] : buses) {
        db.AddBus(key, stops.first, std::move(stops.second));
    }

    return queries;
}

bool IsIntNumber(std::string_view value) {
    if (value.empty()) {
        return false;
    }
    std::string_view::const_iterator
    start = value.cbegin();
    if (value[0] == '+' || value[0] == '-') {
        ++start;
    }

    return std::all_of(start, value.cend(),
                       [](const char &c) {
                           return std::isdigit(c);
    });
}

std::string_view Trim(std::string_view value) {
    if (value.empty()) {
        return value;
    }

    auto pos = value.find_first_not_of(WHITESPACE);
    if (pos == value.npos || pos > value.size()) {
        return std::string_view();
    }
    value.remove_prefix(pos);

    pos = value.find_last_not_of(WHITESPACE);
    if (pos < value.size() + 1) {
        value.remove_suffix(value.size() - pos - 1);
    }

    return value;
}

std::vector<std::string_view>
Split(const std::string_view line, char delimiter, bool trimmed) {
    std::vector<std::string_view> tokens;
    size_t start;
    size_t end = 0;
    while ((start = line.find_first_not_of(delimiter, end))
           != line.npos) {
        end = line.find(delimiter, start);
        std::string_view token = trimmed ?
            Trim(line.substr(start, end - start)) :
            line.substr(start, end - start);
        tokens.push_back(token);
    }
    return tokens;
}

void Split(const std::string_view line, char delimiter,
           std::vector<std::string>& tokens, bool trimmed) {
    size_t start;
    size_t end = 0;
    while ((start = line.find_first_not_of(delimiter, end))
           != line.npos) {
        end = line.find(delimiter, start);
        std::string token = trimmed ?
            std::string(Trim(line.substr(start, end - start))) :
            std::string(line.substr(start, end - start));
        tokens.push_back(std::move(token));
    }
}
        
    }
