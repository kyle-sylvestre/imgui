#include "imgui.h"
#include <stdio.h>

namespace ImGui
{
    bool BeginInputTextCombo(const char* label, ImGuiComboFlags flags)
    {
        bool result = false;
        const char* preview_value = "";
        flags |= ImGuiComboFlags_NoPreview;
        IM_ASSERT( !(flags & ImGuiComboFlags_NoArrowButton) );

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = GetCurrentWindow();

        ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        if (window->SkipItems)
            return false;

        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

        const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : CalcItemWidth();
        //const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
        //const ImRect total_bb(bb.Min, bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

        ImRect bb(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        const float value_x2 = ImMax(bb.Min.x, bb.Max.x - arrow_size);
        ImRect arrow_rect(ImVec2(value_x2, bb.Min.y), bb.Max);
        const ImRect total_bb(arrow_rect.Min, arrow_rect.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

        ImGui::SameLine(arrow_rect.Min.x);
        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id, &total_bb))
            return false;

        // Open on click
        bool hovered, held;
        bool pressed = ButtonBehavior(arrow_rect, id, &hovered, &held);

        const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
        bool popup_open = IsPopupOpen(popup_id, ImGuiPopupFlags_None);
        if (pressed && !popup_open)
        {
            OpenPopupEx(popup_id, ImGuiPopupFlags_None);
            popup_open = true;
        }

        // Render shape
        const ImU32 frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderNavHighlight(bb, id);
        if (!(flags & ImGuiComboFlags_NoPreview))
            ; //window->DrawList->AddRectFilled(bb.Min, ImVec2(value_x2, bb.Max.y), frame_col, style.FrameRounding, (flags & ImGuiComboFlags_NoArrowButton) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersLeft);

        if (!(flags & ImGuiComboFlags_NoArrowButton))
        {
            ImU32 bg_col = GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            ImU32 text_col = GetColorU32(ImGuiCol_Text);
            window->DrawList->AddRectFilled(arrow_rect.Min, arrow_rect.Max, bg_col, style.FrameRounding, (w <= arrow_size) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersRight);
            if (value_x2 + arrow_size - style.FramePadding.x <= bb.Max.x)
                RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, bb.Min.y + style.FramePadding.y), text_col, ImGuiDir_Down, 1.0f);
        }
        //RenderFrameBorder(bb.Min, bb.Max, style.FrameRounding);

        // Custom preview
        if (flags & ImGuiComboFlags_CustomPreview)
        {
            g.ComboPreviewData.PreviewRect = ImRect(bb.Min.x, bb.Min.y, value_x2, bb.Max.y);
            IM_ASSERT(preview_value == NULL || preview_value[0] == 0);
            preview_value = NULL;
        }

        // Render preview and label
        if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
        {
            if (g.LogEnabled)
                LogSetNextTextDecoration("{", "}");
            RenderTextClipped(bb.Min + style.FramePadding, ImVec2(value_x2, bb.Max.y), preview_value, NULL, NULL);
        }
        if (label_size.x > 0)
        {
            ImVec2 pos = ImVec2(bb.Max.x + style.ItemInnerSpacing.x, bb.Min.y + style.FramePadding.y);
            ImRect rect(pos, pos + ImGui::CalcTextSize(label));
            RenderText(pos, label);
        }

        if (!popup_open)
            return false;

        g.NextWindowData.Flags = backup_next_window_data_flags;
        return BeginComboPopup(popup_id, bb, flags);
    }

    void DrawDebug()
    {
        char tmp[4096] = {};
        ImGuiIO &io = ImGui::GetIO();
        ImDrawList *drawlist = ImGui::GetForegroundDrawList();
        snprintf(tmp, sizeof(tmp), "Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        ImVec2 BR = ImGui::CalcTextSize(tmp);
        ImVec2 TL = { 0, 0 };

        drawlist->AddRectFilled(TL, BR, 0xFFFFFFFF);
        drawlist->AddText(TL, 0xFF000000, tmp);

        snprintf(tmp, sizeof(tmp), "Application average %.3f ms/frame (%.1f FPS)",
                 1000.0f / ImGui::GetIO().Framerate, 
                 ImGui::GetIO().Framerate);

        TL.y = BR.y;
        BR = ImGui::CalcTextSize(tmp);
        BR.x += TL.x;
        BR.y += TL.y;
        drawlist->AddRectFilled(TL, BR, 0xFFFFFFFF);
        drawlist->AddText(TL, 0xFF000000, tmp);

        static bool pinned_point_toggled;
        static ImVec2 pinned_point;
        static ImVec2 pinned_window;

        ImVec2 mousepos = ImGui::GetMousePos();
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            pinned_point_toggled = !pinned_point_toggled;
            if (pinned_point_toggled)
            {
                // get relative window position
                ImGuiContext *ctx = ImGui::GetCurrentContext();

                // windows are stored back to front
                for (ImGuiWindow *iter : ctx->Windows)
                {
                    const ImVec2 WINDOW_BR = ImVec2(iter->Pos.x + iter->Size.x,
                                                    iter->Pos.y + iter->Size.y);
                    if ((!iter->Hidden && !iter->Collapsed) &&
                        (iter->Pos.x <= mousepos.x && iter->Pos.y <= mousepos.y) &&
                        (WINDOW_BR.x >= mousepos.x && WINDOW_BR.y >= mousepos.y) )
                    {
                        pinned_window = iter->Pos;
                    }
                }
                pinned_point = mousepos;
            }
        }

        if (pinned_point_toggled)
        {
            // draw a rect in the selected window
            const ImVec2 PIN_BR = ImVec2(mousepos.x - pinned_window.x,
                                         mousepos.y - pinned_window.y);
            const ImVec2 PIN_TL = ImVec2(pinned_point.x - pinned_window.x,
                                         pinned_point.y - pinned_window.y);

            unsigned int col = IM_COL32(0, 255, 0, 32);
            drawlist->AddRectFilled(pinned_point, mousepos, col);

            snprintf(tmp, sizeof(tmp), "window rect:\n  pos: (%d, %d)\n  size: (%d, %d)",
                     (int)PIN_TL.x, (int)PIN_TL.y,
                     (int)(PIN_BR.x - PIN_TL.x), (int)(PIN_BR.y - PIN_TL.y));
            BR = ImGui::CalcTextSize(tmp);
            TL = ImVec2(pinned_point.x, pinned_point.y - BR.y);
            BR.x += TL.x;
            BR.y += TL.y;

            drawlist->AddRectFilled(TL, BR, IM_COL32_WHITE);
            drawlist->AddText(TL, IM_COL32_BLACK, tmp);
        }
    }
}
