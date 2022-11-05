#pragma once

#include <iostream>
#include <map>
#include <string>
#include <initializer_list>
#include <vector>
#include <variant>

namespace json {

	class Node;
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;
	using Data = std::variant<std::nullptr_t, Array, Dict, int, double, bool, std::string >;

	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node {
	public:
		Node() = default;
		Node(std::nullptr_t) {}
		Node(bool val);
		Node(int val);
		Node(double val);
		Node(std::string val);
		Node(Array array);
		Node(Dict map);

		bool AsBool() const;
		int AsInt() const;
		double AsDouble() const;
		const std::string& AsString() const;
		const Array& AsArray() const;
		const Dict& AsMap() const;

		bool IsNull() const;
		bool IsBool() const;
		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsString() const;
		bool IsArray() const;
		bool IsMap() const;

		bool operator==(const Node& other) const;
		bool operator!=(const Node& other) const;

		void Print(std::ostream& out) const;

	private:
		Data value_;

		struct Printer {
			std::ostream& output;

			void operator()(std::nullptr_t) const;
			void operator()(const Array& value) const;
			void operator()(const Dict& value) const;
			void operator()(const bool value) const;
			void operator()(const int value) const;
			void operator()(const double value) const;
			void operator()(const std::string_view value) const;
		};
	};

	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

		bool operator==(const Document& other) const {
			return root_ == other.root_;
		}
		bool operator!=(const Document& other) const {
			return root_ != other.root_;
		}

	private:
		Node root_;
	};

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);
}