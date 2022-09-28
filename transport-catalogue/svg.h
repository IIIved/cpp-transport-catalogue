#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <utility>
#include <optional>
#include <string_view>
#include <variant>
#include <iomanip>
#include <vector>

namespace svg {

    struct Rgb {
    public:
        Rgb() = default;
        Rgb(uint8_t red_, uint8_t green_, uint8_t blue_)
            :red(red_), green(green_), blue(blue_) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
    public:
        Rgba() = default;
        Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_)
            :red(red_), green(green_), blue(blue_), opacity(opacity_) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    struct OstreamColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const {
            using namespace std;
            out << "none"sv;
        }
        void operator()(svg::Rgb rgb_) const {
            using namespace std;
            out << "rgb("sv << static_cast<int>(rgb_.red) << ","sv << static_cast<int>(rgb_.green) << ","sv << static_cast<int>(rgb_.blue) << ")"sv;
        }
        void operator()(svg::Rgba rgba_) const {
            using namespace std;
            out << "rgba("sv << static_cast<int>(rgba_.red) << ","sv << static_cast<int>(rgba_.green) << ","sv << static_cast<int>(rgba_.blue) << ","sv << rgba_.opacity << ")"sv;
        }
        void operator()(std::string str_) const {
            using namespace std;
            out << str_;
        }

    };

    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    inline const Color NoneColor{ "none" };

    std::ostream& operator<<(std::ostream& os, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line);

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_linecap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_linejoin_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            RenderAttrAccessory(out, " fill=\\\""s, fill_color_);
            RenderAttrAccessory(out, " stroke=\\\""s, stroke_color_);
            RenderAttrAccessory(out, " stroke-width=\\\""s, stroke_width_);
            RenderAttrAccessory(out, " stroke-linecap=\\\""s, stroke_linecap_);
            RenderAttrAccessory(out, " stroke-linejoin=\\\""s, stroke_linejoin_);
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        template <typename type>
        void RenderAttrAccessory(std::ostream& out, const std::string& line, const type& attr) const {
            using namespace std::literals;
            if (attr) {
                out << line << *attr << "\\\""sv;
            }
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0.0;
        double y = 0.0;
    };

    class ObjectContainer;

    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point) {
            points_.push_back(point);
            return *this;
        }

        int GetSize() const {
            return points_.size();
        }

    private:
        std::deque<Point> points_ = {};

        void RenderObject(const RenderContext& context) const override;

    };

    class Text final : public Object, public PathProps<Text> {
    public:
        Text& SetPosition(Point pos) {
            pos_ = pos;
            return *this;
        }

        Text& SetOffset(Point offset) {
            offset_ = offset;
            return *this;
        }

        Text& SetFontSize(uint32_t size) {
            font_size_ = size;
            return *this;
        }

        Text& SetFontFamily(std::string font_family) {
            font_family_ = font_family;
            return *this;
        }

        Text& SetFontWeight(std::string font_weight) {
            font_weight_ = font_weight;
            return *this;
        }

        Text& SetData(std::string data) {
            data_ = data;
            return *this;
        }

        Point GetPoint() const {
            return pos_;
        }

    private:
        Point pos_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_ = "";
        std::string font_weight_ = "";
        std::string data_ = "";

        void RenderObject(const RenderContext& context) const override;
    };

    class ObjectContainer {
    public:
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    private:

    };

    class Document : public ObjectContainer {
    public:
        
        void AddPtr(std::unique_ptr<Object>&& obj) override {
            store_.emplace_back(std::move(obj));
        }

        
        void Render(std::ostream& out) const;

    private:
        std::deque<std::unique_ptr<Object>> store_ = {};
    };

}  // namespace svg