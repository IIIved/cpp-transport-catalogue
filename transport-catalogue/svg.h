#pragma once

#include <cstdint>
#include <iostream>
#include <ostream>
#include <memory>
#include <string>
#include <deque>
#include <optional>
#include <variant>

namespace svg {
    using namespace std::literals;

    struct Point {
        Point() = default;
        Point(double x, double y);
        double x = 0;
        double y = 0;
    };

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
    struct RenderContext {
        RenderContext(std::ostream& out);
        RenderContext(std::ostream& out, int indent_step, int indent = 0);

        RenderContext Indented() const;
        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
    * Класс ObjectContainer моделирует interface
    */

    class ObjectContainer {
    public:
        template<typename Obj>
        void Add(Obj obj) {
            objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() = default;

    protected:
        std::deque<std::unique_ptr<Object>> objects_;
    };

    /*
    * Класс Drawable моделирует interface
    */

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& objectContainer) const = 0;

        virtual ~Drawable() = default;
    };

/*
* Класс PathProps задает цвет заливки и контура
*/

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

    inline std::ostream& operator << (std::ostream& out, const StrokeLineCap& strokeLineCap) {
        switch (strokeLineCap) {
            case StrokeLineCap::BUTT:
                return out << "butt";
            case StrokeLineCap::ROUND:
                return out << "round";
            case StrokeLineCap::SQUARE:
                return out << "square";
        }

        return out;
    }

    inline std::ostream& operator << (std::ostream& out, const StrokeLineJoin& strokeLineJoin) {
        switch (strokeLineJoin) {
            case StrokeLineJoin::ARCS:
                return out << "arcs";
            case StrokeLineJoin::BEVEL:
                return out << "bevel";
            case StrokeLineJoin::MITER:
                return out << "miter";
            case StrokeLineJoin::MITER_CLIP:
                return out << "miter-clip";
            case StrokeLineJoin::ROUND:
                return out << "round";
        }

        return out;
    }

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t red, uint8_t green, uint8_t blue);

        uint8_t red = 0, green = 0, blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity);

        uint8_t red = 0, green = 0, blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{"none"};

    struct Visitor {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << "none"sv;
        }

        void operator()(const std::string& str) const {
            out << str;
        }

        void operator()(Rgb rgb) const {
            out << "rgb("sv << unsigned(rgb.red) << ","sv << unsigned(rgb.green) << ","sv <<  unsigned(rgb.blue) << ")"sv;
        }

        void operator()(Rgba rgba) const {
            out << "rgba("sv << unsigned(rgba.red) << ","sv <<  unsigned(rgba.green) << ","sv <<  unsigned(rgba.blue) << ","sv <<  rgba.opacity << ")"sv;
        }
    };

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
            line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            if (fill_color_) {
                out << "fill=\""sv;
                std::visit(Visitor{out}, *fill_color_);
                out << "\""sv;
            }

            if (stroke_color_) {
                out << " stroke=\""sv;
                std::visit(Visitor{out}, *stroke_color_);
                out << "\""sv;
            }

            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    /*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        std::deque<Point> points_;

        void RenderObject(const RenderContext& context) const override;
    };

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>

    private:
        Point pos_;
        Point offset_;
        uint32_t front_size_ = 1;

        std::string font_family_;
        std::string font_weight_;
        std::string data_;

        void RenderObject(const RenderContext& context) const override;
    };

    class Document : public ObjectContainer{
    public:

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj);

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;
    };

}  // namespace svg

namespace shapes {

    class Triangle : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
                : p1_(p1)
                , p2_(p2)
                , p3_(p3) {
        }

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override {
            container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
        }

    private:
        svg::Point p1_, p2_, p3_;
    };

    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays);

    class Star : public svg::Drawable {
    public:
        Star(svg::Point p, double length, double inner_rad, int num_rays) : p_(p), length_(length), inner_rad_(inner_rad), num_rays_(num_rays) {}

        void Draw(svg::ObjectContainer& container) const override {
            container.Add(CreateStar(p_, length_, inner_rad_, num_rays_).SetFillColor("red").SetStrokeColor("black"));
        }

    private:
        svg::Point p_;
        double length_;
        double inner_rad_;
        int num_rays_;
    };

    class Snowman : public svg::Drawable {
    public:
        Snowman(svg::Point p, double radius) : p_(p), radius_(radius) {}

        void Draw(svg::ObjectContainer& container) const override {
            container.Add(svg::Circle().SetCenter({p_.x, p_.y + 5.0 * radius_}).SetRadius(2.0 * radius_).
                    SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
            container.Add(svg::Circle().SetCenter({p_.x, p_.y + 2.0 * radius_}).SetRadius(1.5 * radius_).
                    SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
            container.Add(svg::Circle().SetCenter(p_).SetRadius(radius_).
                    SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        }

    private:
        svg::Point p_;
        double radius_;
    };

} // namespace shapes