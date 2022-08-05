#include <utility>
#include <memory>

#include "json.h"

namespace json {

    class Builder {
    public:
        class DictBuilder;
        class ArrayBuilder;
        class KeyBuilder;

        DictBuilder& StartDict();
        Builder& EndDict();

        ArrayBuilder& StartArray();
        Builder& EndArray();

        KeyBuilder& Key(const std::string& key);
        Builder& Value(const Node& value);

        Node Build();

    public:
        class DictBuilder {
        public:
            DictBuilder(Builder& builder) : builder_(builder) {}

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

            DictBuilder& StartDict() {
                return builder_.StartDict();
            }

            ArrayBuilder& StartArray() {
                return builder_.StartArray();
            }

        private:
            Builder& builder_;
        };

        class KeyBuilder {
        public:
            KeyBuilder(Builder& builder) : builder_(builder) {}

            DictBuilder& StartDict() {
                return builder_.StartDict();
            }

            ArrayBuilder& StartArray() {
                return builder_.StartArray();
            }

            DictBuilder& Value(const Node& value) {
                return *std::make_shared<DictBuilder>(builder_.Value(value));
            }

        private:
            Builder& builder_;
        };

    private:
        Builder& AsAdmin() {
            return static_cast<Builder&>(*this);
        }
        enum stage {
            START,
            PROCESS,
            FINISH,
        };

        int current_ = stage::START;
        std::vector<Node> nodes_stack_;
        bool first_;
    };
}