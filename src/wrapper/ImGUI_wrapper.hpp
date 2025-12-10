#pragma once

#include "imgui.h"
#include <string>

namespace ImGui {
    using Vec4 = ImVec4;

    template<typename T>
    using Vector = ImVector<T>;

    template<typename... Args>
    void Text(const std::string& str) {
        ImGui::TextUnformatted(str.c_str(), nullptr);
    }
} // namespace ImGui
