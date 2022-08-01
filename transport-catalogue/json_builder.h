#include <utility>
#include <memory>

#include "json.h"

namespace json {
    class ArrayBuilder;
    class Vocabulary;
    class KeyBuilder;

    class Builder {
    private:
        Builder& AsAdmin() {
            return static_cast<Builder&>(*this);
        }
    public:

        Vocabulary& StartDict();
        Builder& EndDict();

        ArrayBuilder& StartArray();
        Builder& EndArray();

        KeyBuilder& Key(const std::string& key);
        Builder& Value(const Node& value);
        Node Build();

    private:
        enum phase {
            START,
            PROCESS,
            FINISH,
        };

        int current_ = phase::START;
        std::vector<Node> nodes_stack_;
        bool first_;
    };

    class Vocabulary {
    public:
        Vocabulary(Builder& builder) : builder_(builder) {}

        KeyBuilder& Key(const std::string& key) {
            return builder_.Key(key);
        }

        Builder& EndDict() {
            return builder_.EndDict();
        }

    private:
        Builder& builder_;
    };

    class ArrayBuilder {
    public:
        ArrayBuilder(Builder& builder) : builder_(builder) {}

        Builder& EndArray() {
            return builder_.EndArray();
        }

        ArrayBuilder& Value(const Node& value) {
            return *std::make_shared<ArrayBuilder>(builder_.Value(value));
        }

        ArrayBuilder& StartArray() {
            return builder_.StartArray();
        }

        Vocabulary& StartDict() {
            return builder_.StartDict();
        }



    private:
        Builder& builder_;
    };

    class KeyBuilder {
    public:
        KeyBuilder(Builder& builder) : builder_(builder) {}

        Vocabulary& StartDict() {
            return builder_.StartDict();
        }

        KeyBuilder& StartArray() {
            return reinterpret_cast<KeyBuilder&>(builder_.StartArray());
        }

        Vocabulary& Value(const Node& value) {
            return *std::make_shared<Vocabulary>(builder_.Value(value));
        }

    private:
        Builder& builder_;
    };

}