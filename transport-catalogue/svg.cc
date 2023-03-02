#include "svg.h"

namespace svg {

using namespace std::literals;

// ---------- Object ----------

void Object::Render(const RenderContext& context) const {
  context.RenderIndent();

  // Делегируем вывод тега своим подклассам
  RenderObject(context);

  context.out << std::endl;
}

// ---------- Circle ----------

Circle& Circle::SetCenter(Point center) {
  center_ = center;
  return *this;
}

Circle& Circle::SetRadius(double radius) {
  radius_ = radius;
  return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
  out << "r=\""sv << radius_ << "\""sv;
  RenderAttrs(out);
  out << "/>"sv;
}

// ---------- Polyline ----------

Polyline& Polyline::AddPoint(Point point) {
  points_.emplace_back(std::move(point));
  return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<polyline points=\""sv;
  bool first = true;
  for (const auto p : points_) {
    if (first) {
      out << p.x << ","sv << p.y;
      first = false;
      continue;
    }
    out << " "sv << p.x << ","sv << p.y;
  }
  out << "\""sv;
  RenderAttrs(out);
  out << "/>"sv;
}

// ---------- Text ----------

Text& Text::SetPosition(Point pos) {
  position_ = pos;
  return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
  offset_ = offset;
  return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
  font_size_ = size;
  return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
  font_family_ = std::move(font_family);
  return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
  font_weight_ = std::move(font_weight);
  return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
  data_ = std::move(data);
  return *this;
}

void Text::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<text x=\""sv << position_.x << "\""sv
      << " y=\""sv << position_.y << "\""sv
      << " dx=\""sv << offset_.x << "\""sv
      << " dy=\""sv << offset_.y << "\""sv
      << " font-size=\""sv << font_size_ << "\""sv;
  if (!font_family_.empty()) {
    out << " font-family=\""sv << font_family_ << "\""sv;
  }
  if (!font_weight_.empty()) {
    out << " font-weight=\""sv << font_weight_ << "\""sv;
  }
  RenderAttrs(out);
  out << ">"sv;

  for (const char c : data_) {
    if (c == '\"') {
      out << "&quot;"sv;
      continue;
    }
    if (c == '\'') {
      out << "&apos;"sv;
      continue;
    }
    if (c == '<') {
      out << "&lt;"sv;
      continue;
    }
    if (c == '>') {
      out << "&gt;"sv;
      continue;
    }
    if (c == '&') {
      out << "&amp;"sv;
      continue;
    }
    out << c;
  }
  out << "</text>"sv;
}

// ---------- Document ----------

void Document::Render(std::ostream& out) const {
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv
      << std::endl;
  RenderContext ctx(out, 2, 2);
  for (const auto& o : objects_) {
    o.get()->Render(ctx);
  }
  out << "</svg>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
  objects_.push_back(std::move(obj));
}

// ---------- Detail ----------

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& c) {
  using namespace std::literals;
  switch (c) {
    case StrokeLineCap::BUTT:
      out << "butt"sv;
      break;
    case StrokeLineCap::ROUND:
      out << "round"sv;
      break;
    case StrokeLineCap::SQUARE:
      out << "square"sv;
      break;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& j) {
  using namespace std::literals;
  switch (j) {
    case StrokeLineJoin::ARCS:
      out << "arcs"sv;
      break;
    case StrokeLineJoin::BEVEL:
      out << "bevel"sv;
      break;
    case StrokeLineJoin::MITER:
      out << "miter"sv;
      break;
    case StrokeLineJoin::MITER_CLIP:
      out << "miter-clip"sv;
      break;
    case StrokeLineJoin::ROUND:
      out << "round"sv;
      break;
  }
  return out;
}

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double o)
    : red(r), green(g), blue(b), opacity(o) {}

std::ostream& operator<<(std::ostream& out, const Rgb& rgb) {
  out << "rgb("sv << std::to_string(rgb.red) << ","sv
      << std::to_string(rgb.green) << ","sv << std::to_string(rgb.blue)
      << ")"sv;
  return out;
}

std::ostream& operator<<(std::ostream& out, const Rgba& rgba) {
  out << "rgba("sv << std::to_string(rgba.red) << ","sv
      << std::to_string(rgba.green) << ","sv << std::to_string(rgba.blue)
      << ","sv << rgba.opacity << ")"sv;
  return out;
}

void OstreamColorPrinter::operator()(std::monostate) const { out << "none"sv; }
void OstreamColorPrinter::operator()(const std::string& s) const { out << s; }
void OstreamColorPrinter::operator()(const Rgb& rgb) const { out << rgb; }
void OstreamColorPrinter::operator()(const Rgba& rgba) const { out << rgba; }

std::ostream& operator<<(std::ostream& out, const Color& c) {
  std::visit(OstreamColorPrinter{out}, c);
  return out;
}

}  // namespace svg