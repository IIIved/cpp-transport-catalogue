#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
    using namespace std::literals;

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    using Number = std::variant<int, double>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    struct JsonVisitor {
        std::ostream& output;
        void operator()(std::nullptr_t) const;
        void operator()(int value) const;
        void operator()(double d) const;
        void operator()(bool b) const;
        void operator()(const std::string& value) const;
        void operator()(const Array& array) const;
        void operator()(const Dict& map) const;
    };

    class Node final : private NodeValue {
    public:
        using variant::variant;
        using Value = variant;

        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        bool AsBool() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsString() const;
        bool IsBool() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsPureDouble() const;

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

        const Value& GetValue() const {
            return *this;
        }
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json