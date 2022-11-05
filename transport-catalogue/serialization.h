#pragma once

#include "svg.h"
#include "svg.pb.h"

namespace svg {

    void SerializeColor(const Color& color, SvgProto::Color& proto);

    Color DeserializeColor(const SvgProto::Color& proto);

}
