#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value =
    std::variant<std::nullptr_t, bool, int, double, std::string, Array, Dict>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
 public:
  using runtime_error::runtime_error;
};

class Node {
 public:
  Node() = default;
  Node(std::nullptr_t val);
  Node(bool val);
  Node(int val);
  Node(double val);
  Node(std::string val);
  Node(Array val);
  Node(Dict val);

  const Value& GetValue() const;

  bool IsInt() const;
  bool IsDouble() const;
  bool IsPureDouble() const;
  bool IsBool() const;
  bool IsString() const;
  bool IsNull() const;
  bool IsArray() const;
  bool IsMap() const;

  int AsInt() const;
  bool AsBool() const;
  double AsDouble() const;
  const std::string& AsString() const;
  const Array& AsArray() const;
  const Dict& AsMap() const;

  bool operator==(const Node& other) const;
  bool operator!=(const Node& other) const;

 private:
  Value value_;
};

class Document {
 public:
  explicit Document(Node root);

  const Node& GetRoot() const;

  bool operator==(const Document& other) const;
  bool operator!=(const Document& other) const;

 private:
  Node root_;
};

// ---------- Detail ----------

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
  std::ostream& out;
  int indent_step = 2;
  int indent = 0;

  void PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
      out.put(' ');
    }
  }

  // Возвращает новый контекст вывода с увеличенным смещением
  PrintContext Indented() const {
    return {out, indent_step, indent_step + indent};
  }
};

// Шаблон, подходящий для вывода double и int
template <typename T>
void PrintValue(const T& value, PrintContext& ctx) {
  auto& out = ctx.out;
  out << value;
}

}  // namespace json