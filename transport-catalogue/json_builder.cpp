#include "json_builder.h"

using namespace json;
using namespace std::literals;

Node Builder::Build() {
    if (nodes_stack_.size() != 1 || current_ != phase::FINISH) {
        throw std::logic_error("объект не завершен"s);
    }
    return nodes_stack_.back();
}

Builder &Builder::EndDict() {
    switch (current_) {
        case phase::START:
            throw std::logic_error("EndDict не создан"s);

        case phase::PROCESS:
            if (nodes_stack_.back().IsDict()) {
                if (nodes_stack_.size() == 1) {
                    current_ = phase::FINISH;
                } else {
                    Dict last_dict = nodes_stack_.back().AsDict();
                    nodes_stack_.pop_back();

                    if (nodes_stack_.back().IsArray()) {
                        Array arr = nodes_stack_.back().AsArray();
                        arr.push_back(last_dict);
                        nodes_stack_.back() = arr;
                    } else if (nodes_stack_.back().IsString() && nodes_stack_[nodes_stack_.size() - 2].IsDict()) {
                        std::string key = nodes_stack_.back().AsString();
                        nodes_stack_.pop_back();

                        Dict dict = nodes_stack_.back().AsDict();
                        dict[key] = last_dict;

                        nodes_stack_.back() = dict;
                    } else {
                        throw std::logic_error("EndDict - неудачный тип"s);
                    }
                }
            } else {
                throw std::logic_error("EndDict - нет словаря"s);
            }
            break;

        case phase::FINISH:
            throw std::logic_error("EndDict - КОНЕЦ"s);

    }

    return AsAdmin();
}

Builder &Builder::EndArray() {
    switch (current_) {
        case phase::START:
            throw std::logic_error("EndArray not created"s);

        case phase::PROCESS:
            if (nodes_stack_.back().IsArray()) {
                if (nodes_stack_.size() == 1) {
                    current_ = phase::FINISH;
                } else {
                    auto last_arr = nodes_stack_.back().AsArray();
                    nodes_stack_.pop_back();

                    if (nodes_stack_.back().IsString() && nodes_stack_[nodes_stack_.size() - 2].IsDict()) {
                        std::string key = nodes_stack_.back().AsString();
                        nodes_stack_.pop_back();

                        Dict dict = nodes_stack_.back().AsDict();
                        dict[key] = last_arr;

                        nodes_stack_.back() = dict;
                    } else if (nodes_stack_.back().IsArray()) {
                        Array last_last_arr = nodes_stack_.back().AsArray();
                        last_last_arr.push_back(last_arr);

                        nodes_stack_.back() = last_last_arr;
                    } else {
                        throw std::logic_error("EndArray - ошибка типа"s);
                    }
                }
            } else {
                throw std::logic_error("EndArray не является массивом"s);
            }
            break;

        case phase::FINISH:
            throw std::logic_error("EndArray - КОНЕЦ"s);
    }

    return AsAdmin();
}

Vocabulary& Builder::StartDict() {
    switch (current_) {
        case phase::START:
            nodes_stack_.push_back(Dict{});
            current_ = phase::PROCESS;
            break;

        case phase::PROCESS:
            if (nodes_stack_.back().IsArray() || nodes_stack_.back().IsString()) {
                nodes_stack_.push_back(Dict{});
            } else {
                throw std::logic_error("StartDict - PROCESS ошибка возвращаемого типа"s);
            }
            break;

        case phase::FINISH:
            throw std::logic_error("StartDict - КОНЕЦ"s);


    }

    return *std::make_shared<Vocabulary>(AsAdmin());
}

KeyBuilder& Builder::Key(const std::string &key) {
    if (nodes_stack_.back().IsDict() && current_ != phase::FINISH) {
        nodes_stack_.emplace_back(key);
    } else {
        throw std::logic_error("Ошибка!"s);
    }

    return *std::make_shared<KeyBuilder>(AsAdmin());
}

ArrayBuilder &Builder::StartArray() {
    switch (current_) {
        case phase::START:
            nodes_stack_.push_back(Array{});
            current_ = phase::PROCESS;
            break;

        case phase::PROCESS:
            if (nodes_stack_.back().IsDict()) {
                throw std::logic_error("StartArray - неудачный тип возврата"s);
            }
            nodes_stack_.push_back(Array{});
            break;

        case phase::FINISH:
            throw std::logic_error("StartArray - КОНЕЦ"s);
    }

    return *std::make_shared<ArrayBuilder>(AsAdmin());;
}

Builder &Builder::Value(const Node &value) {
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back().IsString()) {
            std::string key = nodes_stack_.back().AsString();
            nodes_stack_.pop_back();

            Dict dict = nodes_stack_.back().AsDict();
            dict.insert({key, value});
            nodes_stack_.back() = dict;
        } else if (nodes_stack_.back().IsArray()) {
            Array arr = nodes_stack_.back().AsArray();
            arr.push_back(value);
            nodes_stack_.back() = arr;
        } else {
            throw std::logic_error("значение вызова не удалось"s);
        }
    } else {
        if (!first_) {
            nodes_stack_.emplace_back(value);
            first_ = true;
            current_ = phase::FINISH;
        }
    }

    return AsAdmin();
}


