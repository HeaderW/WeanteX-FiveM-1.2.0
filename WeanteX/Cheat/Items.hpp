#pragma once
#include "../KelesDispo/imgui.h"
#include "../KelesDispo/imgui_internal.h"
#include <map>


struct check_state
{
    ImVec4 background, text;
    float alpha_mark = 0.f;
};


struct begin_state
{
    ImVec4 background, text, outline;
    float open, alpha, combo_size = 0.f, shadow_opticaly;
    bool opened_combo = false, hovered = false;
    float arrow_roll;
};


namespace custom {
    static float CalcMaxPopupHeightFromItemCount(int items_count)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        if (items_count <= 0)
            return FLT_MAX;
        return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
    }
}