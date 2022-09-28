#pragma once

#include "json.h"
#include <memory>
#include <utility>

namespace json {

    class Builder {
    private:

        class DictValueContext;
        class DictItemContext;
        class ArrayItemContext;

        class BuilderContext {
        public:
            BuilderContext(Builder& builder);

            DictItemContext StartDict();
            ArrayItemContext StartArray();
            Builder& EndArray();
            DictValueContext Key(const std::string& key);
            Builder& EndDict();
            Builder& GetBuilder();

        private:
            Builder& builder_;
        };

        class ArrayItemContext : public BuilderContext {
        public:
            ArrayItemContext(Builder& builder);

            ArrayItemContext Value(const json::Node::Value& value);
            DictValueContext Key(const std::string& key) = delete;
            Builder& EndDict() = delete;

        };//class ArrayItemContext 

        class DictItemContext : public BuilderContext {
        public:
            DictItemContext(Builder& builder);

            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            Builder& EndArray() = delete;

        };//class DictItemContext

        class DictValueContext : public BuilderContext {
        public:
            DictValueContext(Builder& builder);

            DictItemContext Value(const Node::Value& value);
            Builder& EndArray() = delete;
            DictValueContext Key(const std::string& key) = delete;
            Builder& EndDict() = delete;

        };//class DictValueContext - âîçâðàùàåòñÿ ïîñëå âûçîâà Key è îæèäàåò ëèáî Value ëèáî íà÷àëî Dict èëè Array

    public:
        Builder() = default;
        Node Build();
        DictValueContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();

    private:
        enum class Status {
            EMPTY,
            IN_WORKING,
            ENDED
        };

        std::vector<std::unique_ptr<Node>> nodes_stack_; 
        Status status_ = Status::EMPTY; //

    };//class Builder

} //namespace json