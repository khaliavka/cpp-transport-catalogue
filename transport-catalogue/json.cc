#include "json.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <variant>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
  Array result;
  char c;
  for (; input >> c && c != ']';) {
    if (c != ',') {
      input.putback(c);
    }
    result.push_back(LoadNode(input));
  }
  if (c != ']') {
    throw ParsingError("Failed to read array from stream"s);
  }
  return Node(move(result));
}

// using Number = std::variant<int, double>;

Node LoadNumber(std::istream& input) {
  using namespace std::literals;

  std::string parsed_num;

  // Считывает в parsed_num очередной символ из input
  auto read_char = [&parsed_num, &input] {
    parsed_num += static_cast<char>(input.get());
    if (!input) {
      throw ParsingError("Failed to read number from stream"s);
    }
  };

  // Считывает одну или более цифр в parsed_num из input
  auto read_digits = [&input, read_char] {
    if (!std::isdigit(input.peek())) {
      throw ParsingError("A digit is expected"s);
    }
    while (std::isdigit(input.peek())) {
      read_char();
    }
  };

  if (input.peek() == '-') {
    read_char();
  }
  // Парсим целую часть числа
  if (input.peek() == '0') {
    read_char();
    // После 0 в JSON не могут идти другие цифры
  } else {
    read_digits();
  }

  bool is_int = true;
  // Парсим дробную часть числа
  if (input.peek() == '.') {
    read_char();
    read_digits();
    is_int = false;
  }

  // Парсим экспоненциальную часть числа
  if (int ch = input.peek(); ch == 'e' || ch == 'E') {
    read_char();
    if (ch = input.peek(); ch == '+' || ch == '-') {
      read_char();
    }
    read_digits();
    is_int = false;
  }

  try {
    if (is_int) {
      // Сначала пробуем преобразовать строку в int
      try {
        return Node{std::stoi(parsed_num)};
      } catch (...) {
        // В случае неудачи, например, при переполнении,
        // код ниже попробует преобразовать строку в double
      }
    }
    return Node{std::stod(parsed_num)};
  } catch (...) {
    throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
  }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
  using namespace std::literals;

  auto it = std::istreambuf_iterator<char>(input);
  auto end = std::istreambuf_iterator<char>();
  std::string s;
  while (true) {
    if (it == end) {
      // Поток закончился до того, как встретили закрывающую кавычку?
      throw ParsingError("String parsing error");
    }
    const char ch = *it;
    if (ch == '"') {
      // Встретили закрывающую кавычку
      ++it;
      break;
    } else if (ch == '\\') {
      // Встретили начало escape-последовательности
      ++it;
      if (it == end) {
        // Поток завершился сразу после символа обратной косой черты
        throw ParsingError("String parsing error");
      }
      const char escaped_char = *(it);
      // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
      switch (escaped_char) {
        case 'n':
          s.push_back('\n');
          break;
        case 't':
          s.push_back('\t');
          break;
        case 'r':
          s.push_back('\r');
          break;
        case '"':
          s.push_back('"');
          break;
        case '\\':
          s.push_back('\\');
          break;
        default:
          // Встретили неизвестную escape-последовательность
          throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
      }
    } else if (ch == '\n' || ch == '\r') {
      // Строковый литерал внутри- JSON не может прерываться символами \r или \n
      throw ParsingError("Unexpected end of line"s);
    } else {
      // Просто считываем очередной символ и помещаем его в результирующую
      // строку
      s.push_back(ch);
    }
    ++it;
  }

  return Node(move(s));
}

Node LoadDict(istream& input) {
  Dict result;
  char c;
  for (; input >> c && c != '}';) {
    if (c == ',') {
      input >> c;
    }

    string key = LoadString(input).AsString();
    input >> c;
    result.insert({move(key), LoadNode(input)});
  }

  if (c != '}') {
    throw ParsingError("Failed to read dict from stream"s);
  }

  return Node(move(result));
}

Node LoadLiteral(istream& input, const string& value) {
  char c;
  string nl{};
  input >> c;
  while (input) {
    if (c == ',' || c == '}' || c == ']') {
      input.putback(c);
      break;
    }
    nl.push_back(c);
    if (nl.size() > value.size()) {
      throw ParsingError("Failed to read null"s);
    }
    input >> c;
  }
  if (nl != value) {
    throw ParsingError("Failed to read null"s);
  }
  if (nl == "true"s) {
    return Node{true};
  }
  if (nl == "false"s) {
    return Node{false};
  }
  return {};
}

Node LoadNode(istream& input) {
  char c;
  input >> c;

  if (c == '[') {
    return LoadArray(input);
  } else if (c == '{') {
    return LoadDict(input);
  } else if (c == '"') {
    return LoadString(input);
  } else if (c == 'n') {
    input.putback(c);
    return LoadLiteral(input, "null"s);
  } else if (c == 't') {
    input.putback(c);
    return LoadLiteral(input, "true"s);
  } else if (c == 'f') {
    input.putback(c);
    return LoadLiteral(input, "false"s);
  } else {
    input.putback(c);
    return LoadNumber(input);
  }
}

}  // namespace

// ---------- Node ----------
Node::Node(nullptr_t val) : value_(val) {}
Node::Node(bool val) : value_(val) {}
Node::Node(int val) : value_(val) {}
Node::Node(double val) : value_(val) {}
Node::Node(std::string val) : value_(move(val)) {}
Node::Node(Array val) : value_(move(val)) {}
Node::Node(Dict val) : value_(move(val)) {}

const Value& Node::GetValue() const { return value_; }

bool Node::IsInt() const { return holds_alternative<int>(value_); }

bool Node::IsDouble() const {
  return holds_alternative<int>(value_) || holds_alternative<double>(value_);
}

bool Node::IsPureDouble() const { return holds_alternative<double>(value_); }

bool Node::IsBool() const { return holds_alternative<bool>(value_); }

bool Node::IsString() const { return holds_alternative<string>(value_); }

bool Node::IsNull() const { return holds_alternative<nullptr_t>(value_); }

bool Node::IsArray() const { return holds_alternative<Array>(value_); }

bool Node::IsMap() const { return holds_alternative<Dict>(value_); }

int Node::AsInt() const {
  if (!IsInt()) {
    throw logic_error("logic_error"s);
  }
  return get<int>(value_);
}

bool Node::AsBool() const {
  if (!IsBool()) {
    throw logic_error("logic_error"s);
  }
  return get<bool>(value_);
}

double Node::AsDouble() const {
  if (!IsDouble()) {
    throw logic_error("logic_error"s);
  }
  return IsPureDouble() ? get<double>(value_) : get<int>(value_);
}

const std::string& Node::AsString() const {
  if (!IsString()) {
    throw logic_error("logic_error"s);
  }
  return get<string>(value_);
}

const Array& Node::AsArray() const {
  if (!IsArray()) {
    throw logic_error("logic_error"s);
  }
  return get<Array>(value_);
}

const Dict& Node::AsMap() const {
  if (!IsMap()) {
    throw logic_error("logic_error"s);
  }
  return get<Dict>(value_);
}

bool Node::operator==(const Node& other) const {
  return value_ == other.value_;
}

bool Node::operator!=(const Node& other) const { return !(*this == other); }

// ---------- Document ----------

Document::Document(Node root) : root_(move(root)) {}

const Node& Document::GetRoot() const { return root_; }

bool Document::operator==(const Document& other) const {
  return root_ == other.root_;
}
bool Document::operator!=(const Document& other) const {
  return !(*this == other);
}

// ---------- Detail ----------

Document Load(istream& input) { return Document{LoadNode(input)}; }

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, PrintContext& ctx) {
  auto& out = ctx.out;
  out << "null"sv;
}

void PrintValue(bool b, PrintContext& ctx) {
  auto& out = ctx.out;
  out << boolalpha << b;
}

void PrintValue(const std::string& str, PrintContext& ctx) {
  auto& out = ctx.out;
  out << '"';
  for (char c : str) {
    switch (c) {
      case '"':
        out << '\\' << c;
        break;
      case '\\':
        out << '\\' << c;
        break;
      case '\n':
        out << '\\' << 'n';
        break;
      case '\r':
        out << '\\' << 'r';
        break;
      default:
        out << c;
        break;
    }
  }
  out << '"';
}

void PrintNode(const Node& node, PrintContext& ctx);

void PrintValue(const Array& arr, PrintContext& ctx) {
  auto& out = ctx.out;
  out << "["s << endl;
  auto indented = ctx.Indented();
  bool is_first = true;
  for (const auto& el : arr) {
    if (is_first) {
      PrintNode(el, indented);
      is_first = false;
      continue;
    }
    out << ","s << endl;
    PrintNode(el, indented);
  }
  out << endl << "]"s;
}

void PrintValue(const Dict& dict, PrintContext& ctx) {
  auto& out = ctx.out;
  out << "{"s << endl;
  bool is_first = true;
  for (const auto& [key, value] : dict) {
    if (is_first) {
      PrintValue(key, ctx);
      out << ": "s;
      PrintNode(value, ctx);
      is_first = false;
      continue;
    }
    out << ","s << endl;
    PrintValue(key, ctx);
    out << ": "s;
    PrintNode(value, ctx);
  }
  out << endl << "}"s;
}

void PrintNode(const Node& node, PrintContext& ctx) {
  ctx.PrintIndent();
  visit([&ctx](const auto& v) { PrintValue(v, ctx); }, node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
  PrintContext ctx{output, 0, 0};
  PrintNode(doc.GetRoot(), ctx);
}

}  // namespace json