#include "json.h"

#include <stdexcept>
#include <exception>
#include <algorithm>

using namespace std;

namespace json {

    bool IsPrint(char c) {
        return std::isprint(static_cast<unsigned char>(c)) && c != '\\' && c != '\"';
    }

    namespace {

        using namespace std::literals;

        Node LoadNode(std::istream& input);

        Node LoadString(std::istream& input);

        std::string LoadLiteral(std::istream& input) {
            std::string s;
            while (std::isalpha(input.peek())) {
                s.push_back(static_cast<char>(input.get()));
            }
            return s;
        }

        Node LoadArray(std::istream& input) {
            std::vector<Node> result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw ParsingError("Array parsing error"s);
            }
            return Node(std::move(result));
        }

        Node LoadDict(std::istream& input) {
            Dict dict;
            for (char c; input >> c && c != '}';) {
                if (c == '"') {
                    std::string key = LoadString(input).AsString();
                    if (input >> c && c == ':') {
                        if (dict.find(key) != dict.end()) {
                            throw ParsingError("Dictionary parsing error key: "s + key);
                        }
                        dict.emplace(std::move(key), LoadNode(input));
                    }
                    else {
                        throw ParsingError("Dictionary parsing error: "s + c);
                    }
                }
                else if (c != ',') {
                    throw ParsingError("Dictionary parsing error: "s + c);
                }
            }
            if (!input) {
                throw ParsingError("Dictionary parsing error"s);
            }
            return Node(std::move(dict));
        }

        Node LoadString(std::istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error"s);
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error"s);
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(std::move(s));
        }

        Node LoadBool(std::istream& input) {
            const auto s = LoadLiteral(input);
            if (s == "true"sv) {
                return Node{ true };
            }
            else if (s == "false"sv) {
                return Node{ false };
            }
            else {
                throw ParsingError("Parsing error: "s + s);
            }
        }

        Node LoadNull(std::istream& input) {
            if (auto literal = LoadLiteral(input); literal == "null"sv) {
                return Node{ nullptr };
            }
            else {
                throw ParsingError("Parsing error: "s + literal);
            }
        }

        Node LoadNumber(std::istream& input) {
            std::string parsed_num;

            // Ñ÷èòûâàåò â parsed_num î÷åðåäíîé ñèìâîë èç input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Ñ÷èòûâàåò îäíó èëè áîëåå öèôð â parsed_num èç input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Ïàðñèì öåëóþ ÷àñòü ÷èñëà
            if (input.peek() == '0') {
                read_char();
                // Ïîñëå 0 â JSON íå ìîãóò èäòè äðóãèå öèôðû
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Ïàðñèì äðîáíóþ ÷àñòü ÷èñëà
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Ïàðñèì ýêñïîíåíöèàëüíóþ ÷àñòü ÷èñëà
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Ñíà÷àëà ïðîáóåì ïðåîáðàçîâàòü ñòðîêó â int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // Â ñëó÷àå íåóäà÷è, íàïðèìåð, ïðè ïåðåïîëíåíèè
                        // êîä íèæå ïîïðîáóåò ïðåîáðàçîâàòü ñòðîêó â double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadNode(std::istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("Failed parsing"s);
            }
            switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Node::Node(json::Node::Value& value) {
        this->swap(value);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<int>(*this) || std::holds_alternative<double>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    using namespace std::literals;

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(*this);
        }
        throw std::logic_error("Failed AsArray"s);
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(*this);
        }
        throw std::logic_error("Failed AsMap"s);
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(*this);
        }
        throw std::logic_error("Failed AsInt"s);
    }

    const string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(*this);
        }
        throw std::logic_error("Failed AsString"s);
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return static_cast<double>(std::get<int>(*this));
        }
        else if (IsPureDouble()) {
            return std::get<double>(*this);
        }
        throw std::logic_error("Failed AsDouble"s);
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(*this);
        }
        throw std::logic_error("Failed AsBool"s);
    }

    const Node_variant& Node::GetValue() const {
        return *this;
    }

    bool Node::operator==(const Node& other) const {
        return GetValue() == other.GetValue();
    }

    bool Node::operator!=(const Node& other) const {
        return GetValue() != other.GetValue();
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void NodePrinter::operator()(std::nullptr_t) const {
        output << "null"s;
    }

    void NodePrinter::operator()(const Array& roots) const {
        output << "["s;
        bool start_ = false;
        for (const auto& noda : roots) {
            if (start_) {
                output << ", "s;
            }
            std::visit(NodePrinter{ output }, noda.GetValue());
            start_ = true;
        }
        output << "]"s;
    }

    void NodePrinter::operator()(const Dict& roots) const {
        output << "{"s;
        bool start_ = false;
        for (const auto& [key, node] : roots) {
            if (start_) {
                output << ", "s;
            }
            output << "\""s << key << "\": "s;
            std::visit(NodePrinter{ output }, node.GetValue());
            output << std::endl;
            start_ = true;
        }
        output << "}"s;

    }

    void NodePrinter::operator()(bool root) const {
        output << boolalpha << root;
    }

    void NodePrinter::operator()(int root) const {
        output << root;
    }

    void NodePrinter::operator()(double root) const {
        output << root;
    }

    void NodePrinter::operator()(const std::string& roots) const {
        output << "\""s;
        for (char c : roots) {
            switch (c) {
            case '\\':
                output << "\\\\"s;
                break;
            case '"':
                output << "\\\""s;
                break;
            case '\n':
                output << "\\n"s;
                break;
            case '\t':
                output << "\\t"s;
                break;
            case '\r':
                output << "\\r"s;
                break;
            default:
                output << c;
            }
        }
        output << "\""s;
    }

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    void Print(const Document& doc, std::ostream& output) {
        std::visit(NodePrinter{ output }, doc.GetRoot().GetValue());
    }

}  // namespace json