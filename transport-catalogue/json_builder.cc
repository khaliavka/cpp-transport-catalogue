#include "json_builder.h"

#include <string>

#include "json.h"

namespace json {

using namespace std::literals;

// ---------- Builder ----------

KeyContext Builder::Key(std::string key) {
  if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
    throw std::logic_error("not a dict"s);
  }
  Dict& dict = nodes_stack_.back()->AsModDict();
  auto it = dict.emplace(std::move(key), Node{});
  nodes_stack_.emplace_back(&it.first->second);
  return {*this};
}

Builder& Builder::Value(Node::Value val) {
  if (is_ready_) {
    throw std::logic_error("root is ready"s);
  }
  if (nodes_stack_.empty()) {
    root_ = std::move(Node{val});
    is_ready_ = true;
    return *this;
  }
  if (nodes_stack_.back()->IsDict()) {
    throw std::logic_error("can't place value without key"s);
  }
  if (nodes_stack_.back()->IsArray()) {
    nodes_stack_.back()->AsModArray().emplace_back(val);
    return *this;
  }
  *nodes_stack_.back() = std::move(Node{val});
  nodes_stack_.pop_back();
  return *this;
}

DictContext Builder::StartDict() {
  if (is_ready_) {
    throw std::logic_error("root is ready"s);
  }
  if (nodes_stack_.empty()) {
    root_ = std::move(Node{Dict{}});
    nodes_stack_.emplace_back(&root_);
    return {*this};
  }
  if (nodes_stack_.back()->IsDict()) {
    throw std::logic_error("dict {{"s);
  }
  if (nodes_stack_.back()->IsArray()) {
    auto& new_dict =
        nodes_stack_.back()->AsModArray().emplace_back(Node{Dict{}});
    nodes_stack_.emplace_back(&new_dict);
    return {*this};
  }
  *nodes_stack_.back() = std::move(Node{Dict{}});

  return {*this};
}

ArrayContext Builder::StartArray() {
  if (is_ready_) {
    throw std::logic_error("root is ready"s);
  }
  if (nodes_stack_.empty()) {
    root_ = std::move(Node{Array{}});
    nodes_stack_.emplace_back(&root_);
    return {*this};
  }
  if (nodes_stack_.back()->IsDict()) {
    throw std::logic_error("dict {["s);
  }
  if (nodes_stack_.back()->IsArray()) {
    auto new_arr =
        nodes_stack_.back()->AsModArray().emplace_back(Node{Array{}});
    nodes_stack_.emplace_back(&new_arr);
    return {*this};
  }
  *nodes_stack_.back() = std::move(Node{Array{}});
  return {*this};
}

Builder& Builder::EndDict() {
  if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
    throw std::logic_error("can't end }");
  }
  nodes_stack_.pop_back();
  is_ready_ = nodes_stack_.empty();
  return *this;
}

Builder& Builder::EndArray() {
  if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
    throw std::logic_error("can't end ]");
  }
  nodes_stack_.pop_back();
  is_ready_ = nodes_stack_.empty();
  return *this;
}

Node Builder::Build() {
  if (!is_ready_) {
    throw std::logic_error("root is not ready."s);
  }
  return root_;
}

// ---------- BuilderContext ----------

KeyContext BuilderContext::Key(std::string s) {
  return builder_.Key(std::move(s));
}

Builder& BuilderContext::Value(Node::Value val) {
  return builder_.Value(std::move(val));
}

DictContext BuilderContext::StartDict() { return builder_.StartDict(); }

ArrayContext BuilderContext::StartArray() { return builder_.StartArray(); }

Builder& BuilderContext::EndDict() { return builder_.EndDict(); }

Builder& BuilderContext::EndArray() { return builder_.EndArray(); }

// ---------- KeyContext ----------

DictContext KeyContext::Value(Node::Value val) {
  return {BuilderContext::Value(std::move(val))};
}

DictContext KeyContext::StartDict() { return BuilderContext::StartDict(); }

ArrayContext KeyContext::StartArray() { return BuilderContext::StartArray(); }

// ---------- DictContext ----------

KeyContext DictContext::Key(std::string s) {
  return BuilderContext::Key(std::move(s));
}

Builder& DictContext::EndDict() { return BuilderContext::EndDict(); }

// ---------- ArrayContext ----------

ArrayContext ArrayContext::Value(Node::Value val) {
  return {BuilderContext::Value(std::move(val))};
}

ArrayContext ArrayContext::StartArray() { return BuilderContext::StartArray(); }

DictContext ArrayContext::StartDict() { return BuilderContext::StartDict(); }

Builder& ArrayContext::EndArray() { return BuilderContext::EndArray(); }

}  // namespace json
