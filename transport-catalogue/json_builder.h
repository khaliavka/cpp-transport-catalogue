#pragma once

#include <vector>

#include "json.h"

namespace json {

class KeyContext;
class DictContext;
class ArrayContext;

class Builder {
 public:
  KeyContext Key(std::string);
  Builder& Value(Node::Value);
  DictContext StartDict();
  ArrayContext StartArray();
  Builder& EndDict();
  Builder& EndArray();
  Node Build();

 private:
  Node root_{};
  std::vector<Node*> nodes_stack_{};
  bool is_ready_ = false;
};

class BuilderContext {
 public:
  BuilderContext(Builder& b) : builder_{b} {}

  KeyContext Key(std::string);
  Builder& Value(Node::Value);
  DictContext StartDict();
  ArrayContext StartArray();
  Builder& EndDict();
  Builder& EndArray();
 private:
  Builder& builder_;
};

class KeyContext : private BuilderContext {
 public:
  KeyContext(Builder& b) : BuilderContext(b) {}

  DictContext Value(Node::Value);
  DictContext StartDict();
  ArrayContext StartArray();
};

class DictContext : private BuilderContext {
 public:
  DictContext(Builder& b) : BuilderContext(b) {}

  KeyContext Key(std::string);
  Builder& EndDict();
};

class ArrayContext : private BuilderContext {
 public:
  ArrayContext(Builder& b) : BuilderContext(b) {}

  ArrayContext Value(Node::Value);
  ArrayContext StartArray();
  DictContext StartDict();
  Builder& EndArray();
};

}  // namespace json