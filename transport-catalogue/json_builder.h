#pragma once

#include "json.h"

#include<memory>
#include <stack>
#include <string>
#include <exception>

namespace json {

	class DictBuilder;
	class ArrayBuilder;
	class ValueBuilder;

	class Builder {
	public:
		Builder() = default;
		~Builder();

		ValueBuilder Key(const std::string& key);
		DictBuilder StartDict();
		ArrayBuilder StartArray();

		Builder& Value(const Data& value);
		Builder& EndDict();
		Builder& EndArray();

		void Clear();
		Node Build();
		bool IsDictKeyTop();

	private:
		enum class state { START, CHANGE, FINISH };
		state state_ = state::START;
		std::stack<std::unique_ptr<Node>> steps_;
	};


	class ArrayBuilder {
	public:
		ArrayBuilder(Builder& builder_)
			:builder_(builder_) {

		}

		ArrayBuilder Value(const Data& value);

		DictBuilder StartDict();

		ArrayBuilder StartArray();

		Builder& EndArray();

	private:
		Builder& builder_;
	};


	class DictBuilder {
	public:
		DictBuilder(Builder& builder_)
			:builder_(builder_) {

		}

		ValueBuilder Key(const std::string& key);

		Builder& EndDict();

	private:
		Builder& builder_;
	};



	class ValueBuilder {
	public:
		ValueBuilder(Builder& builder_)
			:builder_(builder_) {

		}

		DictBuilder Value(const Data& value);

		DictBuilder StartDict();

		ArrayBuilder StartArray();

	private:
		Builder& builder_;
	};

}