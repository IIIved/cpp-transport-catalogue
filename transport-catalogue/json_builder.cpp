#include "json_builder.h"

namespace json {

    Builder::~Builder() {
        Clear();
    }

    bool Builder::IsDictKeyTop() {
        return (steps_.size() > 1) && steps_.top()->IsString();
    }

    void Builder::Clear() {
        while (!steps_.empty()) {
            steps_.pop();
        }
        state_ = state::START;
    }

    Node Builder::Build() {
        if (state_ == state::FINISH) {
            return *steps_.top();
        }
        else {
            throw std::logic_error("building of unready node");
        }
    }

    ValueBuilder Builder::Key(const std::string& key) {
        switch (state_) {
        case state::START:
            throw std::logic_error("empty node key add attempt");
            break;
        case state::CHANGE: {
            if (steps_.top().get()->IsMap()) {
                steps_.push(std::make_unique<Node>(key));
            }
            else {
                throw std::logic_error(IsDictKeyTop() ? "dict key entered twice" : "not dict node key add attempt");
            }
        }
                          break;
        case state::FINISH:
            throw std::logic_error("ready node key add attempt");
            break;
        default:
            throw std::logic_error("dict key common error");
        }

        return ValueBuilder(*this);
    }

    DictBuilder Builder::StartDict() {
        switch (state_) {
        case state::START:
            state_ = state::CHANGE;
            steps_.push(std::make_unique<Node>(Dict()));
            break;
        case state::CHANGE:
            if (!steps_.top().get()->IsMap()) {
                steps_.push(std::make_unique<Node>(Dict()));
            }
            else {
                throw std::logic_error("start dict in another dict error");
            }
            break;
        case state::FINISH:
            throw std::logic_error("ready node start dict attempt");
            break;
        default:
            throw std::logic_error("start dict common error");
        }

        return DictBuilder(*this);
    }
    ArrayBuilder Builder::StartArray() {
        switch (state_) {
        case state::START:
            state_ = state::CHANGE;
            steps_.push(std::make_unique<Node>(Array()));
            break;
        case state::CHANGE: {
            if (steps_.top().get()->IsMap()) {
                throw std::logic_error("start array error. enter a dict key first");
            }
            steps_.push(std::make_unique<Node>(Array()));
        }
                          break;
        case state::FINISH:
            throw std::logic_error("ready node start array attempt");
            break;
        default:
            throw std::logic_error("start array error");
        }
        return ArrayBuilder(*this);
    }

    Builder& Builder::Value(const Data& value) {
        switch (state_) {
        case state::START: {
            steps_.push(std::make_unique<Node>(
                std::visit([](auto& value) {
                    return Node(value);
                    }, value)
                )
            );
            state_ = state::FINISH;
        }
                         break;
        case state::CHANGE: {
            if (steps_.top()->IsArray()) {
                json::Array tmp = std::move(steps_.top()->AsArray());
                tmp.emplace_back(
                    std::visit([](auto& value) {
                        return Node(value);
                        }, value)
                );
                *steps_.top() = Node(std::move(tmp));
            }
            else if (IsDictKeyTop()) {
                std::string key = std::move(steps_.top()->AsString());
                steps_.pop();
                json::Dict dict = std::move(steps_.top().get()->AsMap());
                dict.insert({ key, std::visit([](auto& value) {
                    return Node(value);
                    }, value) });
                *steps_.top() = Node(std::move(dict));
            }
            else {
                throw std::logic_error("dict value without key add attempt");
            }
        } break;
        case state::FINISH:
            throw std::logic_error("ready node value add attempt");
            break;
        default:
            throw std::logic_error("value common error");
        }
        return *this;
    }

    Builder& Builder::EndDict() {
        switch (state_) {
        case state::START:
            throw std::logic_error("empty node end dict attempt");
            break;
        case state::CHANGE: {
            if (steps_.top().get()->IsMap()) {
                if (steps_.size() == 1) {
                    state_ = state::FINISH;
                }
                else {
                    json::Dict value = std::move(steps_.top().get()->AsMap());
                    steps_.pop();
                    Value(value);
                }
            }
            else {
                throw std::logic_error(steps_.top()->IsString() ? "dict value expected" : "it is not a dict");
            }
        } break;
        case state::FINISH:
            throw std::logic_error("ready node end dict attempt");
            break;
        default:
            throw std::logic_error("end dict common error");
        }
        return *this;
    }

    Builder& Builder::EndArray() {
        switch (state_) {
        case state::START:
            throw std::logic_error("empty node end array attempt");
            break;
        case state::CHANGE: {
            if (steps_.top()->IsArray()) {
                if (steps_.size() == 1) {
                    state_ = state::FINISH;
                }
                else {
                    json::Array value = std::move(steps_.top()->AsArray());
                    steps_.pop();
                    Value(value);
                }
            }
            else {
                throw std::logic_error("non-array node end array attempt");
            }
        } break;
        case state::FINISH:
            throw std::logic_error("ready node end array attempt");
            break;
        default:
            throw std::logic_error("end aray common error");
        }
        return *this;
    }


    ArrayBuilder ArrayBuilder::Value(const Data& value) {
        return ArrayBuilder(builder_.Value(value));
    }

    DictBuilder ArrayBuilder::StartDict() {
        return builder_.StartDict();
    }

    ArrayBuilder ArrayBuilder::StartArray() {
        return builder_.StartArray();
    }

    Builder& ArrayBuilder::EndArray() {
        return builder_.EndArray();
    }


    ValueBuilder DictBuilder::Key(const std::string& key) {
        return builder_.Key(key);
    }

    Builder& DictBuilder::EndDict() {
        return  builder_.EndDict();
    }


    DictBuilder ValueBuilder::Value(const Data& value) {
        return DictBuilder(builder_.Value(value));
    }

    DictBuilder ValueBuilder::StartDict() {
        return builder_.StartDict();
    }

    ArrayBuilder ValueBuilder::StartArray() {
        return builder_.StartArray();
    }

}