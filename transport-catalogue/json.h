#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <iomanip>
#include <cctype>
#include <utility>
#include <string_view>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    bool IsPrint(char c);

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using Node_variant = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    struct NodePrinter;

    class Node final : private Node_variant {
    public:
        using variant::variant;
        using Value = variant;

        Node(json::Node::Value& value);

        bool IsArray() const;
        bool IsMap() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsBool() const;
        bool IsPureDouble() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;

        const Node_variant& GetValue() const;

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;
        Node& operator=(const Node& other) = default;
    };

    struct NodePrinter {
        std::ostream& output;

        void operator()(const Array& roots) const;
        void operator()(const Dict& roots) const;
        void operator()(bool root) const;
        void operator()(int root) const;
        void operator()(double root) const;
        void operator()(const std::string& roots) const;
        void operator()(std::nullptr_t) const;

    };

    class Document {
    public:
        Document() = default;
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs);
    inline bool operator!=(const Document& lhs, const Document& rhs);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json