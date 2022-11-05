#include "serialization.h"


namespace svg {

    void SerializeColor(const Color& color, SvgProto::Color& proto) {
        if (std::holds_alternative<std::monostate>(color)) {
            proto.set_is_none(true);
        }
        else if (std::holds_alternative<std::string>(color)) {
            const std::string& name = std::get<std::string>(color);
            proto.set_name(name);
        }
        else {
            const bool has_opacity = std::holds_alternative<Rgba>(color);
            auto& rgba_proto = *proto.mutable_rgba();
            if (has_opacity) {
                const Rgba& rgb = std::get<Rgba>(color);
                rgba_proto.set_red(rgb.red);
                rgba_proto.set_green(rgb.green);
                rgba_proto.set_blue(rgb.blue);
                rgba_proto.set_has_opacity(true);
                rgba_proto.set_opacity(std::get<Rgba>(color).opacity);
            }
            else {
                const Rgb& rgb = std::get<Rgb>(color);
                rgba_proto.set_red(rgb.red);
                rgba_proto.set_green(rgb.green);
                rgba_proto.set_blue(rgb.blue);
            }
        }
    }

    Color DeserializeColor(const SvgProto::Color& proto) {
        if (proto.is_none()) {
            return std::monostate{};
        }

        if (!proto.has_rgba()) {
            return proto.name();
        }

        const auto& rgba_proto = proto.rgba();
        const auto red = static_cast<uint8_t>(rgba_proto.red());
        const auto green = static_cast<uint8_t>(rgba_proto.green());
        const auto blue = static_cast<uint8_t>(rgba_proto.blue());

        if (rgba_proto.has_opacity()) {
            return Rgba(red, green, blue, rgba_proto.opacity());
        }
        else {
            return  Rgb(red, green, blue);
        }
    }

}
