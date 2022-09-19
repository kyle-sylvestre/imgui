#include "imgui.h"
#include <stdio.h>


void ImGui_MarkMissingCodepointDefault(unsigned int)
{
}

void ImGui_DrawDebug()
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
