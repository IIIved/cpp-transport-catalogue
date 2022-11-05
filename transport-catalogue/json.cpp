#include "json.h"

#include <cwctype>
#include <array>
#include <iomanip>



namespace json {

	using namespace std;

	using Number = std::variant<int, double>;

	Node LoadNode(istream& input);

	Node LoadArray(istream& input) {
		Array result;
		char c;
		while (input >> c) {
			if (c == ']') {
				return Node(move(result));
			}
			if (c != ',') {
				input.putback(c);
			}
			result.push_back(LoadNode(input));
		}
		throw json::ParsingError("Parsing: Array error"s);
	}

	Number LoadNumber(std::istream& input) {
		using namespace std::literals;

		std::string parsed_num;

		auto read_char = [&parsed_num, &input] {
			parsed_num += static_cast<char>(input.get());
			if (!input) {
				throw ParsingError("Failed to read number from stream"s);
			}
		};

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
		if (input.peek() == '0') {
			read_char();
		}
		else {
			read_digits();
		}

		bool is_int = true;
		if (input.peek() == '.') {
			read_char();
			read_digits();
			is_int = false;
		}

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

				try {
					return std::stoi(parsed_num);
				}
				catch (...) {}
			}
			return std::stod(parsed_num);
		}
		catch (...) {
			throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
		}
	}

	Node LoadString(istream& input) {
		string line;
		for (char c; input >> std::noskipws >> c;) {
			if (c == '"') {
				input >> std::skipws;
				return Node(move(line));
			}
			if (c == '\\') {
				input >> c;
				switch (c) {
				case '"': line.push_back('"'); break;
				case 'n': line.push_back('\n'); break;
				case 'r': line.push_back('\r'); break;
				case '\\': line.push_back('\\'); break;
				case 't': line.push_back('\t'); break;
				default:
					input >> std::skipws;
					throw json::ParsingError("bad string"s);
				}
			}
			else {
				line.push_back(c);
			}
		}
		throw json::ParsingError("String parsing error"s);
	}

	Node LoadDict(istream& input) {
		Dict result;
		char c;
		while (input >> c) {
			if (c == '}') {
				return Node(move(result));
			}
			if (c == ',') {
				input >> c;
			}
			string key = LoadString(input).AsString();
			input >> c;
			result.insert({ move(key), LoadNode(input) });
		}
		throw json::ParsingError("Map parsing error"s);
	}

	Node LoadBool(istream& input) {
		string value(5, '\0');

		for (int i = 0; i < 4; ++i) {
			input >> value[i];
		}

		if (value.substr(0, 4) == "true") {
			return Node(true);
		}

		input >> value[4];
		if (value == "false") {
			return Node(false);
		}

		throw json::ParsingError("Parsing: bool error"s);
	}

	Node LoadNull(istream& input) {
		string value(4, '\0');
		for (int i = 0; i < 4; ++i) {
			input >> value[i];
		}

		if (value == "null") {
			return Node(nullptr);
		}

		throw json::ParsingError("Parsing: null error"s);
	}

	Node LoadNode(istream& input) {
		char c;
		input >> c;

		if (c == '[') {
			return LoadArray(input);
		}
		else if (c == '{') {
			return LoadDict(input);
		}
		else if (c == '"') {
			return LoadString(input);
		}
		else if (c == 't' || c == 'f') {
			input.putback(c);
			return LoadBool(input);
		}
		else if (c == 'n') {
			input.putback(c);
			return LoadNull(input);
		}
		else {
			input.putback(c);
			Number x = LoadNumber(input);
			return (std::holds_alternative<int>(x) ? Node(std::get<int>(x)) : Node(std::get<double>(x)));
		}
	}

	Node::Node(bool val)
		: value_(val) {}

	Node::Node(int val)
		: value_(val) {}

	Node::Node(double val)
		: value_(val) {}

	Node::Node(std::string val)
		: value_(move(val)) {}

	Node::Node(Array array)
		: value_(move(array)) {}

	Node::Node(Dict map)
		: value_(move(map)) {}

	bool Node::AsBool() const {
		if (IsBool()) {
			return get<bool>(value_);
		}
		else {
			throw std::logic_error("Failed: return not bool"s);
		}
	}

	int Node::AsInt() const {
		if (IsInt()) {
			return get<int>(value_);
		}
		else {
			throw std::logic_error("Failed: return not int"s);
		}
	}

	double Node::AsDouble() const {
		if (IsPureDouble()) {
			return get<double>(value_);
		}
		else if (IsInt()) {
			return get<int>(value_);
		}
		else {
			throw std::logic_error("Failed: return not double"s);
		}
	}

	const string& Node::AsString() const {
		if (IsString()) {
			return get<std::string>(value_);
		}
		else {
			throw std::logic_error("Failed: return not string"s);
		}
	}

	const Array& Node::AsArray() const {
		if (IsArray()) {
			return get<Array>(value_);
		}
		else {
			throw std::logic_error("Failed: return not array"s);
		}
	}

	const Dict& Node::AsMap() const {
		if (IsMap()) {
			return get<Dict>(value_);
		}
		else {
			throw std::logic_error("Failed: return not Dict"s);
		}
	}

	bool Node::IsNull() const {
		return (value_ == Data());
		if (IsNull()) {
			return (value_ == Data());
		}
		else {
			throw std::logic_error("Failed: return not nullptr"s);
		}
	}

	bool Node::IsBool() const {
		return std::holds_alternative<bool>(value_);
	}

	bool Node::IsInt() const {
		return std::holds_alternative<int>(value_);
	}

	bool Node::IsDouble() const {
		return IsPureDouble() || IsInt();
	}

	bool Node::IsPureDouble() const {
		return std::holds_alternative<double>(value_);
	}

	bool Node::IsString() const {
		return std::holds_alternative<std::string>(value_);
	}

	bool Node::IsArray() const {
		return std::holds_alternative<Array>(value_);
	}

	bool Node::IsMap() const {
		return std::holds_alternative<Dict>(value_);
	}

	bool Node::operator==(const Node& other) const {
		return value_ == other.value_;
	}

	bool Node::operator!=(const Node& other) const {
		return value_ != other.value_;
	}


	Document::Document(Node root) : root_(move(root)) {}

	const Node& Document::GetRoot() const {
		return root_;
	}

	Document Load(istream& input) {
		return Document{ LoadNode(input) };
	}

	void Node::Print(std::ostream& out) const {
		std::visit(Printer{ out }, value_);
	}

	void Node::Printer::operator()(std::nullptr_t) const {
		output << "null"s;
	}

	void Node::Printer::operator()(const Array& value) const {
		output << "["s;
		bool first = true;
		for (auto it = value.begin(); it != value.end(); ++it) {
			if (!first) {
				output << ", ";
			}
			first = false;
			(*it).Print(output);
		}
		output << "]"s;
	}

	void Node::Printer::operator()(const Dict& value) const {
		output << '{';
		bool first = true;
		for (const auto& [key, node] : value) {
			if (!first) {
				output << ", ";
			}
			first = false;
			Printer{ output }(key);
			output << ": ";
			node.Print(output);
		}
		output << '}';
	}

	void Node::Printer::operator()(const bool value) const {
		if (value) {
			output << "true"s;
		}
		else {
			output << "false"s;
		}
	}

	void Node::Printer::operator()(const int value) const {
		output << value;
	}

	void Node::Printer::operator()(const double value) const {
		output << value;
	}

	void Node::Printer::operator()(const std::string_view value) const {
		string result = "\""s;
		for (const char c : value) {
			switch (c) {
			case '\\':
				result += "\\\\"s;
				break;
			case '\"':
				result += "\\\""s;
				break;
			case '\n':
				result += "\\n"s;
				break;
			case '\r':
				result += "\\r"s;
				break;
			case '\t':
				result += "\\t"s;
				break;
			default:
				result += c;
			}
		}
		output << result + "\""s;
	}

	void Print(const Document& doc, std::ostream& output) {
		doc.GetRoot().Print(output);
	}


}