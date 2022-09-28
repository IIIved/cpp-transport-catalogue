#include "svg.h"
#include <iomanip>

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
    }

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    std::ostream& operator<<(std::ostream& os, const Color& color) {
        std::visit(OstreamColorPrinter{ os }, color);
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line) {
        using namespace std;
        std::string_view s;
        switch (line) {
        case StrokeLineCap::BUTT:
            s = "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            s = "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            s = "square"sv;
            break;
        }
        return (os << s);
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line) {
        using namespace std;
        string s;
        switch (line) {
        case StrokeLineJoin::ARCS:
            s = "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            s = "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            s = "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            s = "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            s = "round"s;
            break;
        }
        return (os << s);
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\\\""sv << double(center_.x) << "\\\" cy=\\\""sv << double(center_.y) << "\\\" "sv;
        out << "r=\\\""sv << radius_ << "\\\" "sv;
        RenderAttrs(context.out);
        out << "/>\\n"sv;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\\\""sv;
        for (int i = 0; i < GetSize(); i++) {
            if (i == GetSize() - 1) {
                out << (points_[i]).x << "," << (points_[i]).y;
            }
            else {
                out << (points_[i]).x << "," << (points_[i]).y << " ";
            }
        }
        out << "\\\""sv;
        RenderAttrs(context.out);
        out << "/>\\n"sv;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(context.out);
        out << " x=\\\""sv << pos_.x << "\\\" y=\\\""sv << pos_.y << "\\\" "sv; //  _<text x="35" y="20" _
        out << "dx=\\\""sv << offset_.x << "\\\" dy=\\\""sv << offset_.y << "\\\" "sv; // _dx="0" dy="6" _
        out << "font-size=\\\""sv << font_size_ << "\\\""; // _font-size="12"_
        if (font_family_ != "") {
            out << " font-family=\\\""sv << font_family_ << "\\\""; // _ font-family="Verdana"_
        }
        if (font_weight_ != "") {
            out << " font-weight=\\\""sv << font_weight_ << "\\\""; // _ font-weight="bold"
        }
        out << ">"sv << data_; // _font-weight="bold">Hello C++_
        out << "</text>\\n"sv;
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\\\"1.0\\\" encoding=\\\"UTF-8\\\" ?>\\n"sv;
        out << "<svg xmlns=\\\"http://www.w3.org/2000/svg\\\" version=\\\"1.1\\\">\\n"sv;
        for (size_t i = 0; i < store_.size(); i++) {
            //out << "It is size " << store_.size() << std::endl;
            RenderContext cont(out, 0, 2);
            (store_[i])->Render(cont);
        }
        out << "</svg>"sv;
    }

}  // namespace svg