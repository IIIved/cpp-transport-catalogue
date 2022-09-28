#include "json_builder.h"

using namespace std::literals;

namespace json {

    json::Node Builder::Build() {
        if (status_ == Status::ENDED) {
            return *(nodes_stack_.back());
        }
        else {
            throw std::logic_error("Unable to build a node. Status is not ENDED."s);
        }
    }

    Builder::DictValueContext Builder::Key(std::string key) {
        switch (status_) {
        case Status::EMPTY:
        case Status::ENDED:
            throw std::logic_error("Couldn't create a key. Status is not IN_WORKING."s);
            break;
        case Status::IN_WORKING:
            nodes_stack_.push_back(std::make_unique<Node>(key));
            break;
        default:
            throw std::logic_error("Unknown error"s);
        }
        return DictValueContext(*this);
    }

    Builder& Builder::Value(json::Node::Value value) {
        switch (status_) {
        case Status::EMPTY:
        {
            nodes_stack_.push_back(std::make_unique<Node>(value));
            status_ = Status::ENDED;
            break;
        }
        case Status::IN_WORKING:
        {
            if (nodes_stack_.back()->IsArray()) {
                json::Array arr = nodes_stack_.back()->AsArray();
                arr.emplace_back(value);
                *(nodes_stack_.back()) = Node(std::move(arr));
                break;
            }
            else if (nodes_stack_.back()->IsString()) {
                std::string key = nodes_stack_.back()->AsString();
                nodes_stack_.pop_back();
                json::Dict dict = nodes_stack_.back()->AsMap();
                dict.insert({ key, value });
                *(nodes_stack_.back()) = Node(std::move(dict));
                break;
            }
            break;
        }
        case Status::ENDED:
            throw std::logic_error("Couldn't create a value. Status: ENDED."s);
            break;
        default:
            throw std::logic_error("Unknown error"s);
        }
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        switch (status_) {
        case Status::ENDED:
            throw std::logic_error("Couldn't create a Dict. Status ENDED."s);
            break;
        case Status::EMPTY:
        case Status::IN_WORKING:
            nodes_stack_.push_back(std::make_unique<Node>(Dict()));
            status_ = Status::IN_WORKING;
            break;
        default:
            throw std::logic_error("Unknown error"s);
        }
        return Builder::DictItemContext(*this);
    }

    Builder& Builder::EndDict() {
        switch (status_) {
        case Status::EMPTY:
        case Status::ENDED:
            throw std::logic_error("Couldn't end a Dict. Status is not IN_WORKING"s);
            break;
        case Status::IN_WORKING:
        {
            if (nodes_stack_.size() == 1) {
                status_ = Status::ENDED;
            }
            else {
                json::Dict value = nodes_stack_.back()->AsMap();
                nodes_stack_.pop_back();
                Value(std::move(value));
            }
            break;
        }
        default:
            throw std::logic_error("Unknown error"s);
        }
        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {
        switch (status_) {
        case Status::EMPTY:
        {
            nodes_stack_.push_back(std::make_unique<Node>(Array()));
            status_ = Status::IN_WORKING;
            break;
        }
        case Status::IN_WORKING:
        {
            nodes_stack_.push_back(std::make_unique<Node>(Array()));
            break;
        }
        case Status::ENDED:
            throw std::logic_error("Couldn't create a Array. Status: ENDED."s);
            break;
        default:
            throw std::logic_error("Unknown error"s);
        }
        return ArrayItemContext(*this);
    }

    Builder& Builder::EndArray() {
        switch (status_) {
        case Status::EMPTY:
        case Status::ENDED:
            throw std::logic_error("Couldn't end a Array. Status is not IN_WORKING."s);
            break;
        case Status::IN_WORKING:
        {
            if (nodes_stack_.size() == 1) {
                status_ = Status::ENDED;
            }
            else {
                json::Array value = nodes_stack_.back()->AsArray();
                nodes_stack_.pop_back();
                Value(std::move(value));
            }
            break;
        }
        default:
            throw std::logic_error("Unknown error"s);
        }
        return *this;
    }

    Builder::BuilderContext::BuilderContext(Builder& builder)
        : builder_(builder) {}

    Builder& Builder::BuilderContext::GetBuilder() {
        return builder_;
    }

    Builder::DictItemContext Builder::BuilderContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext Builder::BuilderContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::BuilderContext::EndArray() {
        return builder_.EndArray();
    }
    Builder::DictValueContext Builder::BuilderContext::Key(const std::string& key) {
        return builder_.Key(key);
    }
    Builder& Builder::BuilderContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::ArrayItemContext::ArrayItemContext(Builder& builder)
        : BuilderContext(builder) {}

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(const json::Node::Value& value) {
        return ArrayItemContext(GetBuilder().Value(value));
    }

    Builder::DictItemContext::DictItemContext(Builder& builder)
        : BuilderContext(builder) {}

    Builder::DictValueContext::DictValueContext(Builder& builder)
        : BuilderContext(builder) {}

    Builder::DictItemContext Builder::DictValueContext::Value(const Node::Value& value) {
        return DictItemContext(GetBuilder().Value(value));
    }

}