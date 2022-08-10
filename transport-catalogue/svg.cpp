
#include "svg.h"

#include <utility>

namespace svg {

    using namespace std::literals;

    Point::Point(double x, double y) : x(x), y(y) {
    }

    RenderContext::RenderContext(std::ostream& out) : out(out) {
    }

    RenderContext::RenderContext(std::ostream& out, int indent_step, int indent) : out(out), indent_step(indent_step), indent(indent) {
    }

    RenderContext RenderContext::Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    Rgb::Rgb(uint8_t red_, uint8_t green_, uint8_t blue_)
            :red(red_), green(green_), blue(blue_) {
    }

    Rgba::Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_)
            :red(red_), green(green_), blue(blue_), opacity(opacity_) {
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

// ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline &Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;

        bool first = true;
        for (const auto& point : points_) {
            if (!first) {
                out << " "sv;
            }
            out << point.x << ","sv << point.y;
            first = false;
        }
        out << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text &Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        front_size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text &Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<text "sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
        out << "\" " << "font-size=\""sv << front_size_;

        if (!font_family_.empty()) {
            out << "\" font-family=\""sv << font_family_;
        }

        if (!font_weight_.empty()) {
            out << "\" font-weight=\""sv << font_weight_;
        }

        out << "\">"sv << data_ << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object> &&obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream &out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\""sv << " version=\"1.1\">"sv << std::endl;
        for (const auto& obj : objects_) {
            out << "  "sv;
            obj->Render(out);
        }
        out << "</svg>"sv;
    }

}  // namespace svg