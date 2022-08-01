#include "json.h"

#include <cctype> // для функции isalpha
#include <utility>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            Number result;

            std::string par_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&par_num, &input] {
                par_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в par_num из input
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
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
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
                    // Сначала пробуем преобразовать строку в int
                    try {
                        result = std::stoi(par_num);

                        return Node{get<int>(result)};
                    } catch (...) {
                        // В случае неудачи, например, при переполнении
                        // код ниже попробует преобразовать строку в double
                    }
                }
                result = std::stod(par_num);

                return Node{get<double>(result)};
            } catch (...) {
                throw ParsingError("Failed to convert "s + par_num + " to number"s);
            }
        }

        Node LoadString(istream& input) {
            string pars_string;

            // Считывает в parsed_string очередной символ из input
            auto read_char = [&pars_string, &input] {
                pars_string += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            size_t cnt_quota = 0;
            while (input.peek() != EOF) {
                char ch = input.peek();
                if (ch == '"') {
                    ++cnt_quota;
                    input.ignore();
                    break;
                } else if (ch == '\\') {
                    input.ignore();
                    ch = input.peek();
                    switch (ch) {
                        case 'n':
                            input.ignore();
                            pars_string += '\n';
                            break;
                        case 't':
                            input.ignore();
                            pars_string += '\t';
                            break;
                        case 'r':
                            input.ignore();
                            pars_string += '\r';
                            break;
                        case '"':
                            read_char();
                            break;
                        case '\\':
                            read_char();
                            break;
                        case ':':
                            read_char();
                            break;
                        case '/':
                            read_char();
                            break;
                        case '}':
                            read_char();
                            break;
                        case ']':
                            read_char();
                            break;
                        default:
                            throw ParsingError("Unrecognized escape sequence \\"s + ch);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    read_char();
                }
            }

            if (cnt_quota != 1) {
                throw ParsingError("String parsing error");
            }

            return Node(move(pars_string));
        }

        Node LoadBool(istream& input) {
            string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            while (input) {
                if (input.peek() == ',') {
                    break;
                } else if (isalpha(input.peek())) {
                    read_char();
                } else if (input.peek() == '}' || input.peek() == ']') {
                    break;
                } else {
                    input.ignore();
                }
            }

            if (parsed_num != "true"s && parsed_num != "false"s) {
                throw ParsingError("type bool failed"s);
            }

            return Node(parsed_num == "true"s);
        }

        Node LoadArray(istream& input) {
            Array result;
            size_t cnt_escape = 0;

            for (char c; input >> c;) {
                if (c == ']') {
                    ++cnt_escape;
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (cnt_escape < 1) {
                throw ParsingError("not even ["s);
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            size_t cnt_escape = 0;

            for (char c; input >> c;) {
                if (c == '}') {
                    ++cnt_escape;
                    break;
                }
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }

            if (cnt_escape < 1) {
                throw ParsingError("not even {"s);
            }

            return Node(move(result));
        }

        Node IsNull(istream& input) {
            string pars_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&pars_num, &input] {
                pars_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            while (input) {
                if (input.peek() == ',') {
                    break;
                } else if (isalpha(input.peek())) {
                    read_char();
                } else if (input.peek() == '}' || input.peek() == ']') {
                    break;
                } else {
                    input.ignore();
                }
            }

            if (pars_num != "null"s) {
                throw ParsingError("is null failed");
            }

            return Node(nullptr);
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            } else if (c == 'n') {
                input.putback(c);
                return IsNull(input);;
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    // ************************* /
    // ********* AS ************ /
    // ************************* /
    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("is not int type"s);
        }
        return get<int>(*this);
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return static_cast<double>(AsInt());
        }
        if (!IsDouble()) {
            throw logic_error("is not double type");
        }
        return get<double>(*this);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("is not string type");
        }
        return get<string>(*this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("is not bool type");
        }
        return get<bool>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("is not array type");
        }
        return get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw logic_error("is not map type");
        }
        return get<Dict>(*this);
    }

    // ************************* /
    // ********* IS ************/
    // ************************* /
    bool Node::IsNull() const {
        if (holds_alternative<nullptr_t>(*this)) {
            return true;
        }
        return false;
    }

    bool Node::IsInt() const {
        if (holds_alternative<int>(*this)) {
            return true;
        }
        return false;
    }

    bool Node::IsDouble() const {
        if (holds_alternative<double>(*this)) {
            return true;
        } else if (holds_alternative<int>(*this)) {
            return true;
        }
        return false;
    }

    bool Node::IsString() const {
        if (holds_alternative<string>(*this)) {
            return true;
        }
        return false;
    }

    bool Node::IsBool() const {
        if (holds_alternative<bool>(*this)) {
            return true;
        }
        return false;
    }


    bool Node::IsArray() const {
        if (holds_alternative<Array>(*this)) {
            return true;
        }
        return false;
    }

    bool Node::IsMap() const {
        if (holds_alternative<Dict>(*this)) {
            return true;
        }
        return false;
    }

    bool Node::IsDict() const {
        return std::holds_alternative<Dict>(*this);
    }

    bool Node::IsPureDouble() const {
        if (holds_alternative<double>(*this)) {
            return true;
        } else if (holds_alternative<int>(*this)) {
            return false;
        }
        return false;
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

    bool Document::operator==(const Document& other) const {
        return root_ == other.root_;
    }

    bool Document::operator!=(const Document& other) const {
        return root_ != other.root_;
    }



    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void JsonVisitor::operator()(std::nullptr_t) const {
        output << "null"s;
    }

    void JsonVisitor::operator()(int value) const {
        output << value;
    }

    void JsonVisitor::operator()(double d) const {
        output << d;
    }

    void JsonVisitor::operator()(bool b) const {
        output << boolalpha << b;
    }

    void JsonVisitor::operator()(const string &value) const {
        output << "\"";
        for (const char& ch : value) {
            switch (ch) {
                case '"':
                    output << "\\";
                    output << '"';
                    break;
                case '\\':
                    output << '\\';
                    output << ch;
                    break;
                case '\t':
                    output << "\\t";
                    break;
                case '\n':
                    output << "\\n";
                    break;
                case '\r':
                    output << "\\r";
                    break;
                case ']':
                    output << "\\]";
                    break;
                case '}':
                    output << "\\}";
                    break;
                default:
                    output << ch;
                    break;
            }
        }
        output << "\"";
    }

    void JsonVisitor::operator()(const Array &array) const {
        output << "["s;
        bool flag = false;

        for (const auto& arr : array) {
            if (flag) {
                output << ", "s;
            }
            std::visit(JsonVisitor{output}, arr.GetValue());
            flag = true;
        }

        output << "]"s;
    }

    void JsonVisitor::operator()(const Dict &map) const {
        output << "{ "s;

        bool flag = false;
        for (const auto& dict : map) {
            if (flag) {
                output << ", "s;
            }
            output << "\""s << dict.first << "\": "s;
            std::visit(JsonVisitor{output}, dict.second.GetValue());
            flag = true;
        }

        output << " }"s;
    }

    void Print(const Document& doc, std::ostream& output) {
        visit(JsonVisitor{output}, doc.GetRoot().GetValue());
    }

}  // namespace json