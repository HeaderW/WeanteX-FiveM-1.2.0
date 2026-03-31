
#define byte win32_byte

#include "main.h"
#include <dwmapi.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <unordered_map>

#undef byte

typedef unsigned char byte;

#pragma comment(lib, "psapi.lib")


#include "audio/audio_data.h"


bool exitLoop = false;
RECT ScreenSize;
int maxPlayerCount = 256;
int loopDelay = 5;

#define XorString(s) s


static std::string DecryptString(const unsigned char* encrypted, size_t len, unsigned char key) {
    std::string result;
    result.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        result += static_cast<char>(encrypted[i] ^ key);
    }
    return result;
}


struct WatermarkElements {
    float height = 60.0f;
    float rounding = 6.0f;
    float padding = 20.0f;
    ImVec2 spacing = ImVec2(40, 40);
    ImVec2 display_padding = ImVec2(20, 20);
} watermark_elements;

struct WatermarkOptions {
    bool watermark = true;
} watermark_options;


void RenderWatermark()
{
    if (!watermark_options.watermark) return;

    static bool watermark_first_time = true;


    if (watermark_first_time) {
        float screenWidth = ImGui::GetIO().DisplaySize.x;
        ImGui::SetNextWindowPos(ImVec2(screenWidth - 200, 20), ImGuiCond_FirstUseEver);
        watermark_first_time = false;
    }


    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);


    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(25, 25, 25, 200));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));


    if (ImGui::Begin("##watermark", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing))
    {
        ImGui::PushFont(font::inter_semibold);


        ImGui::Text("WeanteX v1.2.0");


        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(192, 7, 62, 255));
        ImGui::PopStyleColor();

        ImGui::PopFont();
    }
    ImGui::End();


    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(2);
}


static void ClearClipboard() {
    if (OpenClipboard(nullptr)) { EmptyClipboard(); CloseClipboard(); }
}

static void PlayBeepSound() { Beep(800, 300); }
static void PlayBoopSound() { Beep(400, 500); }

static std::string GetHWID() {
    DWORD serialNumber = 0;
    GetVolumeInformationA("C:\\", NULL, 0, &serialNumber, NULL, NULL, NULL, 0);
    return std::to_string(serialNumber);
}


namespace mjLib {
    namespace Console {
        void ExitError() { exit(1); }
    }
    namespace Logger {
        enum LogLevel { LOG_ERROR };
        void WriteLog(const char* msg, LogLevel level) {}
    }
}


#define IMPORT(x) x


struct GameStruct;
struct OffsetsStruct;
extern GameStruct Game;
extern OffsetsStruct Offsets;


#include "Cheat/Game.hpp"
#include "Cheat/Memory.hpp"
#include "Cheat/Menu.hpp"
#include "Cheat/Cheat.hpp"
#include "audio/audio_data.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{


    while (!Game.pID) {
        Game.pID = FindGame();
        Sleep(100);
    }

    Sleep(5000);
    Game.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Game.pID);
    if (!Game.hProcess) {
        mjLib::Console::ExitError();
    }

    while (!Game.hWnd) {
        Game.hWnd = FindWindowA(Game.lpClassName, NULL);
        Offsets.GameBase = GetBaseAddress();
        Sleep(100);
    }

    ReadOffsets();


    Game.World = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);


    GetWindowRect(Game.hWnd, &Game.lpRect);
    GetCursorPos(&Game.lpPoint);


    FindServerInfo();

    if (!LanGame) {

        std::thread([=]() { UpdateNamesThread(); }).detach();
    }


    std::thread([&]() { UpdatePeds(); }).detach();
    std::thread([=]() { SetAim(); }).detach();
    std::thread([=]() { TriggerBot(); }).detach();
    std::thread([=]() { AddPlayerList(); }).detach();
    std::thread([=]() { Exploits(); }).detach();

    if (!IMPORT(GetWindowRect)(IMPORT(GetDesktopWindow)(), &ScreenSize))
        return 1;


    WNDCLASSEXA wc = {
       sizeof(WNDCLASSEXA),
       0,
       WndProc,
       0,
       0,
       nullptr,
       LoadIcon(nullptr, IDI_APPLICATION),
       LoadCursor(nullptr, IDC_ARROW),
       nullptr,
       nullptr,
       ("GTA V Overlay"),
       LoadIcon(nullptr, IDI_APPLICATION)
    };
    ::RegisterClassExA(&wc);

    RECT Rect;
    if (!IMPORT(GetWindowRect)(IMPORT(GetDesktopWindow)(), &Rect))
        return 1;


    HWND hwnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        "GTA V Overlay", "GTA V Overlay", WS_POPUP,
        Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wc.hInstance, NULL);

    if (!SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), BYTE(255), LWA_ALPHA))
        return 1;

    RECT client_area{};
    GetClientRect(hwnd, &client_area);

    RECT window_area{};
    GetWindowRect(hwnd, &window_area);

    POINT diff{};
    ClientToScreen(hwnd, &diff);

    const MARGINS margins{
        window_area.left + (diff.x - window_area.left),
        window_area.top + (diff.y - window_area.top),
        window_area.right,
        window_area.bottom
    };

    const auto dwm_lib = LoadLibraryA("Dwmapi.dll");
    if (FAILED(DwmExtendFrameIntoClientArea(hwnd, &margins))) {
        FreeLibrary(dwm_lib);
        return 1;
    }
    FreeLibrary(dwm_lib);

    if (!CreateDeviceD3D(hwnd))
        {
        CleanupDeviceD3D();
        ::UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);


    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;


    io.MouseDrawCursor = false;
    io.WantCaptureMouse = true;
    io.WantCaptureKeyboard = true;


    ImGuiStyle* style = &ImGui::GetStyle();
    style->AntiAliasedLines = true;
    style->AntiAliasedLinesUseTex = true;
    style->AntiAliasedFill = true;


    style->WindowPadding = ImVec2(0, 0);
    style->WindowBorderSize = 0;
    style->ItemSpacing = ImVec2(20, 20);
    style->ItemInnerSpacing = ImVec2(8, 8);
    style->FramePadding = ImVec2(8, 4);
    style->ScrollbarSize = 10.f;

    ImFontConfig cfg;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;

    font::inter_semibold = io.Fonts->AddFontFromMemoryTTF(inter_semibold, sizeof(inter_semibold), 15.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::icomoon_page = io.Fonts->AddFontFromMemoryTTF(icomoon_page, sizeof(icomoon_page), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::icomoon_logo = io.Fonts->AddFontFromMemoryTTF(icomoon_page, sizeof(icomoon_page), 30.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::icon_notify = io.Fonts->AddFontFromMemoryTTF(icon_notify, sizeof(icon_notify), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::menufont = io.Fonts->AddFontFromMemoryTTF(menufont, sizeof(menufont), 15.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());


    font::inter_medium = io.Fonts->AddFontFromMemoryCompressedTTF(InterMedium_compressed_data, InterMedium_compressed_size, 12.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());


    FrameWork::Assets::InterMedium12 = font::inter_medium;


    ImFont* breesh_font = io.Fonts->AddFontFromMemoryTTF(Breesh, sizeof(Breesh), 24.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    ImFont* menu_font = io.Fonts->AddFontFromMemoryTTF(menufont, sizeof(menufont), 24.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());


    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);


    InitializeMenu(g_pd3dDevice);


    Menu.MenuAlpha = 0.0f;
    Menu.MenuScale = 0.0f;
    Menu.MenuVisible = false;
    Menu.IsAnimating = false;
    io.MouseDrawCursor = false;

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(0, 0, 0, 0);

    D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
    if (texture::preview_slow == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, preview_slow, sizeof(preview_slow), &info, pump, &texture::preview_slow, 0);

    bool done = false;
    while (!done)
    {

        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        NewFrame();


        static bool firstTime = true;
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            if (!Menu.IsAnimating) {
                Menu.IsAnimating = true;
                Menu.IsOpening = !Menu.MenuVisible;
                Menu.MenuVisible = !Menu.MenuVisible;


                if (!Menu.IsOpening) {
                    firstTime = true;
                }
            }
        }


        if (Menu.IsAnimating) {
            float deltaTime = ImGui::GetIO().DeltaTime;
            float animationStep = Menu.AnimationSpeed * deltaTime;

            if (Menu.IsOpening) {
                Menu.MenuScale += animationStep;
                if (Menu.MenuScale >= 1.0f) {
                    Menu.MenuScale = 1.0f;
                    Menu.IsAnimating = false;
                }
            } else {
                Menu.MenuScale -= animationStep;
                if (Menu.MenuScale <= 0.0f) {
                    Menu.MenuScale = 0.0f;
                    Menu.IsAnimating = false;
                }
            }


            float easedScale = EaseOutCubic(Menu.MenuScale);
            Menu.MenuAlpha = easedScale;
            io.MouseDrawCursor = (Menu.MenuScale > 0.0f);


            if (Menu.MenuScale > 0.0f) {
                SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOOLWINDOW);
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            } else {
                SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
            }
            UpdateWindow(hwnd);
        }


        if (Cheats::MenuUtils::StreamProof) {
            SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
        }
        else {
            SetWindowDisplayAffinity(hwnd, WDA_NONE);
        }


        Game.hWnd = FindWindowA(Game.lpClassName, NULL);
        RECT currentRect{};
        POINT currentPoint{};

        if (Game.hWnd) {
            GetClientRect(Game.hWnd, &currentRect);
            ClientToScreen(Game.hWnd, &currentPoint);


            if (currentRect.left != Game.lpRect.left || currentRect.right != Game.lpRect.right ||
                currentRect.top != Game.lpRect.top || currentRect.bottom != Game.lpRect.bottom ||
                currentPoint.x != Game.lpPoint.x || currentPoint.y != Game.lpPoint.y) {
                Game.lpRect = currentRect;
                Game.lpPoint = currentPoint;
            }


            RECT GameRect;
            if (GetWindowRect(Game.hWnd, &GameRect)) {
                MoveWindow(hwnd, GameRect.left, GameRect.top, Rect.right, Rect.bottom, true);
            }
        }


        HWND hwnd_active = GetForegroundWindow();
        if (hwnd_active == Game.hWnd) {
            HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
            if (hwndtest) {
                SetWindowPos(hwnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
            POINT xy = {};
            if (ClientToScreen(Game.hWnd, &xy)) {
                io.ImeWindowHandle = Game.hWnd;
                io.DeltaTime = 1.0f / 144.0f;

                POINT p;
                if (GetCursorPos(&p)) {
                    io.MousePos.x = p.x - xy.x;
                    io.MousePos.y = p.y - xy.y;

                    bool mouseDown = (GetAsyncKeyState(0x1) != 0);
                    io.MouseDown[0] = mouseDown;
                    if (mouseDown) {
                        io.MouseClicked[0] = true;
                        io.MouseClickedPos[0].x = io.MousePos.x;
                        io.MouseClickedPos[0].y = io.MousePos.y;
                    }
                }
            }
        }


        HWND foregroundWindow = GetForegroundWindow();
        bool gameWindowActive = (foregroundWindow == Game.hWnd);

        if (gameWindowActive) {
            Draw::Always();
            Draw::Esp();


            static int vehicleDrawSkip = 0;
            vehicleDrawSkip++;
            if (vehicleDrawSkip % 2 == 0) {
                Draw::Vehicle();
            }
        }


        RenderWatermark();


        if (Menu.MenuScale > 0.0f) {

            ImVec2 originalSize = ImVec2(890, 700);
            ImVec2 scaledSize = ImVec2(originalSize.x * Menu.MenuScale, originalSize.y * Menu.MenuScale);


            if (firstTime && Menu.MenuScale > 0.8f) {
                ImVec2 screenCenter = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
                ImVec2 windowPos = ImVec2(screenCenter.x - scaledSize.x * 0.5f, screenCenter.y - scaledSize.y * 0.5f);
                ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
                firstTime = false;
            }


            ImGui::SetNextWindowSize(scaledSize, ImGuiCond_Always);
            ImGui::SetNextWindowSizeConstraints(ImVec2(scaledSize.x * 0.8f, scaledSize.y * 0.8f), ImGui::GetIO().DisplaySize);


            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Menu.MenuScale);

            Begin("M1LL3X", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
        {
            ImGuiStyle* style = &ImGui::GetStyle();

            const ImVec2& pos = ImGui::GetWindowPos();
            const ImVec2& region = ImGui::GetContentRegionMax();
            const ImVec2& spacing = style->ItemSpacing;

            style->WindowPadding = ImVec2(0, 0);
            style->ItemSpacing = ImVec2(20, 20);
            style->WindowBorderSize = 0;
            style->ScrollbarSize = 10.f;

            c::bg::background = ImLerp(c::bg::background, dark ? ImColor(15, 15, 15) : ImColor(255, 255, 255), ImGui::GetIO().DeltaTime * 12.f);
            c::separator = ImLerp(c::separator, dark ? ImColor(22, 23, 26) : ImColor(222, 228, 244), ImGui::GetIO().DeltaTime * 12.f);

            c::accent = ImLerp(c::accent, dark ? ImColor(192, 7, 62) : ImColor(192, 7, 62), ImGui::GetIO().DeltaTime * 12.f);

            c::elements::background_hovered = ImLerp(c::elements::background_hovered, dark ? ImColor(31, 33, 38) : ImColor(197, 207, 232), ImGui::GetIO().DeltaTime * 25.f);
            c::elements::background = ImLerp(c::elements::background, dark ? ImColor(22, 23, 25) : ImColor(222, 228, 244), ImGui::GetIO().DeltaTime * 25.f);

            c::checkbox::mark = ImLerp(c::checkbox::mark, dark ? ImColor(0, 0, 0) : ImColor(255, 255, 255), ImGui::GetIO().DeltaTime * 12.f);

            c::child::background = ImLerp(c::child::background, dark ? ImColor(17, 17, 18) : ImColor(241, 243, 249), ImGui::GetIO().DeltaTime * 12.f);
            c::child::cap = ImLerp(c::child::cap, dark ? ImColor(20, 21, 23) : ImColor(228, 235, 248), ImGui::GetIO().DeltaTime * 12.f);

            c::page::text_hov = ImLerp(c::page::text_hov, dark ? ImColor(68, 71, 85) : ImColor(136, 145, 176), ImGui::GetIO().DeltaTime * 12.f);
            c::page::text = ImLerp(c::page::text, dark ? ImColor(68, 71, 85) : ImColor(136, 145, 176), ImGui::GetIO().DeltaTime * 12.f);

            c::page::background_active = ImLerp(c::page::background_active, dark ? ImColor(31, 33, 38) : ImColor(196, 205, 228), ImGui::GetIO().DeltaTime * 25.f);
            c::page::background = ImLerp(c::page::background, dark ? ImColor(22, 23, 25) : ImColor(222, 228, 244), ImGui::GetIO().DeltaTime * 25.f);

            c::text::text_active = ImLerp(c::text::text_active, dark ? ImColor(255, 255, 255) : ImColor(0, 0, 0), ImGui::GetIO().DeltaTime * 12.f);
            c::text::text_hov = ImLerp(c::text::text_hov, dark ? ImColor(68, 71, 85) : ImColor(68, 71, 81), ImGui::GetIO().DeltaTime * 12.f);
            c::text::text = ImLerp(c::text::text, dark ? ImColor(68, 71, 85) : ImColor(68, 71, 81), ImGui::GetIO().DeltaTime * 12.f);

            GetWindowDrawList()->AddRectFilled(pos, pos + ImVec2(region), ImGui::GetColorU32(c::bg::background), c::bg::rounding);
            GetWindowDrawList()->AddRectFilled(pos + spacing, pos + ImVec2(region.x - spacing.x, 50 + spacing.y), ImGui::GetColorU32(c::child::background), c::child::rounding);


            ImGui::PushFont(menu_font);
            const char* text = "WeanteX Cheat 1.2.0";
            ImVec2 textSize = ImGui::CalcTextSize(text);


            float centerX = region.x / 2.0f;


            float centerY = spacing.y + 18;

            ImVec2 textPos = pos + ImVec2(centerX - textSize.x/2, centerY - textSize.y/2);

            ImU32 mainTextColor = dark ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255);
            GetWindowDrawList()->AddText(textPos, mainTextColor, text);
            ImGui::PopFont();


            ImGui::PushFont(font::inter_semibold);
            const char* subtext = "www.weantex.xyz & discord.gg/weantex";
            ImVec2 subtextSize = ImGui::CalcTextSize(subtext);


            float subtextY = centerY + textSize.y/2 + 2;
            ImVec2 subtextPos = pos + ImVec2(centerX - subtextSize.x/2, subtextY);

            ImU32 subTextColor = dark ? IM_COL32(180, 180, 180, 255) : IM_COL32(100, 100, 100, 255);
            GetWindowDrawList()->AddText(subtextPos, subTextColor, subtext);
            ImGui::PopFont();


            GetWindowDrawList()->AddImage(texture::preview_slow, pos + ImVec2(spacing.x + 111, spacing.y), pos + ImVec2(spacing.x + 200, 50 + spacing.y), ImVec2(1, 0), ImVec2(0, 1), ImGui::GetColorU32(c::accent, 0.6f));


            GetWindowDrawList()->AddImage(texture::preview_slow, pos + ImVec2(region.x - (spacing.x + 200), spacing.y), pos + ImVec2(region.x - (spacing.x + 111), 50 + spacing.y), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(c::accent, 0.6f));

            SetCursorPos(ImVec2(spacing.x, (50 + (spacing.y * 2))));

            tab_alpha = ImClamp(tab_alpha + (4.f * ImGui::GetIO().DeltaTime * (page == active_tab ? 1.f : -1.f)), 0.f, 1.f);
            if (tab_alpha == 0.f && tab_add == 0.f) active_tab = page;

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * style->Alpha);

            if (active_tab == 0)
            {

                custom::BeginGroup();
                {

                    custom::Child("GENERAL", ImVec2((GetContentRegionMax().x - spacing.x * 3) / 2, (GetContentRegionMax().y - (60 + spacing.y * 2) * 2) + 10), true);
                    {

                        static bool popup = false;

                        if (popup) {
                            ImGui::Begin("Popupbox", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
                            {
                                custom::Child("Popupbox", ImVec2(300, 170), ImGuiWindowFlags_NoBringToFrontOnFocus);
                                {
                                    custom::ColorEdit4("Color Palette", col, picker_flags);

                                    custom::Separator_line();

                                    custom::Combo("Combobox", &select1, items, IM_ARRAYSIZE(items), 3);
                                }
                                custom::EndChild();
                            }
                            ImGui::End();
                        }


                        custom::Checkbox("Silent", &Cheats::AimAssist::Silent::Enabled);
                        custom::Separator_line();

                        custom::Keybind("Silent Key", &Cheats::AimAssist::Silent::Key, &m);
                        custom::Separator_line();

                        custom::Checkbox("Show FOV", &Cheats::AimAssist::Silent::DrawFov);
                        custom::Separator_line();

                        custom::Checkbox("Dynamic FOV", &Cheats::MenuUtils::DynamicFOV);
                        custom::Separator_line();

                        custom::Checkbox("Draw Target", &Cheats::AimAssist::Silent::DrawTarget);
                        custom::Separator_line();

                        if (Cheats::AimAssist::Silent::DrawTarget) {
                            custom::ColorEdit4("Target Color", (float*)&Cheats::AimAssist::Silent::TargetColor, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Draw Silent Line", &Cheats::AimAssist::Silent::DrawSilentLine);
                        custom::Separator_line();

                        if (Cheats::AimAssist::Silent::DrawSilentLine) {
                            custom::ColorEdit4("Line Color", (float*)&Cheats::AimAssist::Silent::SilentLineColor, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Closest Bone", &Cheats::AimAssist::Silent::ClosestBone);
                        custom::Separator_line();


                        custom::Checkbox("Aimbot", &Cheats::AimAssist::Aimbot::Enabled);
                        custom::Separator_line();

                        custom::Keybind("Aimbot Key", &Cheats::AimAssist::Aimbot::Key, &m);
                        custom::Separator_line();

                        custom::Checkbox("Show FOV", &Cheats::AimAssist::Aimbot::DrawFov);
                        custom::Separator_line();

                        custom::Combo("Select Bones", &Cheats::AimAssist::Aimbot::SelectedType, Cheats::AimAssist::Aimbot::Type, IM_ARRAYSIZE(Cheats::AimAssist::Aimbot::Type), 3);
                        custom::Separator_line();

                        custom::SliderFloat("Aimbot Fov", &Cheats::AimAssist::Aimbot::Fov, 1, 400, "%0.1f");
                        custom::Separator_line();

                        custom::SliderFloat("Aimbot Distance", &Cheats::AimAssist::Aimbot::Distance, 1.0f, 1000.0f, "%.1f");
                        custom::Separator_line();

                        custom::SliderFloat("Aimbot Smooth", &Cheats::AimAssist::Aimbot::Smooth, 1.0f, 50.0f, "%.1f");
                        custom::Separator_line();


                    }
                    custom::EndChild();
                }
                custom::EndGroup();

                ImGui::SameLine();

                custom::BeginGroup();
                {
                    custom::Child("SETTINGS", ImVec2((GetContentRegionMax().x - spacing.x * 3) / 2, ((GetContentRegionMax().y - (60 + spacing.y * 2) * 2) + 10) / 2 - 10), true);
                    {

                        custom::SliderFloat("Silent Fov", &Cheats::AimAssist::Silent::Fov, 1.0f, 70.0f, "%.1f");
                        custom::Separator_line();

                        custom::SliderFloat("Silent Distance", &Cheats::AimAssist::Silent::Distance, 1.0f, 1000.0f, "%.1f");
                        custom::Separator_line();

                    }
                    custom::EndChild();

                    custom::Child("OTHER", ImVec2((GetContentRegionMax().x - spacing.x * 3) / 2, ((GetContentRegionMax().y - (60 + spacing.y * 2) * 2) + 10) / 2 - 10), true);
                    {


                        custom::Checkbox("Only Visible", &Cheats::AimAssist::OnlyVisible);
                        custom::Separator_line();

                        custom::Checkbox("Ignore Ped", &Cheats::AimAssist::IgnorePed);
                        custom::Separator_line();

                        custom::Checkbox("Ignore Death", &Cheats::AimAssist::IgnoreDeath);
                        custom::Separator_line();


                    }
                    custom::EndChild();

                }
                custom::EndGroup();

            }
            else if (active_tab == 1)
            {
                static bool checkboxes[10];

                custom::BeginGroup();
                {

                    custom::Child("ESP", ImVec2((GetContentRegionMax().x - spacing.x * 3) / 2, (GetContentRegionMax().y - (60 + spacing.y * 2) * 2) + 10), true);
                    {
                        custom::Checkbox("Draw Skeleton", &Cheats::Players::DrawSkeleton::Enabled);
                        custom::Separator_line();

                        if (Cheats::Players::DrawSkeleton::Enabled) {
                            custom::ColorEdit4("Skeleton Color", (float*)&Cheats::Players::DrawSkeleton::Color, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Draw Id", &Cheats::Players::DrawId::Enabled);
                        custom::Separator_line();

                        if (Cheats::Players::DrawId::Enabled) {
                            custom::ColorEdit4("Id Color", (float*)&Cheats::Players::DrawId::Color, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Draw Name", &Cheats::Players::DrawName::Enabled);
                        custom::Separator_line();

                        if (Cheats::Players::DrawName::Enabled) {
                            custom::ColorEdit4("Name Color", (float*)&Cheats::Players::DrawName::Color, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Draw Head", &Cheats::Players::DrawHeadDot::Enabled);
                        custom::Separator_line();

                        if (Cheats::Players::DrawHeadDot::Enabled) {
                            custom::ColorEdit4("Head Color", (float*)&Cheats::Players::DrawHeadDot::Color, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Draw Box", &Cheats::Players::DrawBox::Enabled);
                        custom::Separator_line();

                        if (Cheats::Players::DrawBox::Enabled) {
                            custom::ColorEdit4("Box Color", (float*)&Cheats::Players::DrawBox::Color, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Draw Line", &Cheats::Players::DrawLine::Enabled);
                        custom::Separator_line();

                        if (Cheats::Players::DrawLine::Enabled) {
                            custom::ColorEdit4("Line Color", (float*)&Cheats::Players::DrawLine::Color, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Draw Direction", &Cheats::Players::DirectionEsp::Enabled);
                        custom::Separator_line();

                        if (Cheats::Players::DirectionEsp::Enabled) {
                            custom::ColorEdit4("Direction Color", (float*)&Cheats::Players::DirectionEsp::Color, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Draw Distance", &Cheats::Players::DrawDistance::Enabled);
                        custom::Separator_line();

                        if (Cheats::Players::DrawDistance::Enabled) {
                            custom::ColorEdit4("Distance Color", (float*)&Cheats::Players::DrawDistance::Color, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Draw Health", &Cheats::Players::DrawHealth::Enabled);
                        custom::Separator_line();

                        custom::Checkbox("Draw Armor", &Cheats::Players::DrawArmor::Enabled);
                        custom::Separator_line();

                        custom::Checkbox("Draw Weapon Name", &Cheats::Players::DrawWeaponName::Enabled);
                        custom::Separator_line();

                        if (Cheats::Players::DrawWeaponName::Enabled) {
                            custom::ColorEdit4("Weapon Color", (float*)&Cheats::Players::DrawWeaponName::Color, Menu.ColorPickerFlags);
                            custom::Separator_line();
                        }

                        custom::Checkbox("Only Visible", &Cheats::Players::OnlyVisible);
                        custom::Separator_line();

                        custom::Checkbox("Ignore Ped", &Cheats::Players::IgnorePed);
                        custom::Separator_line();

                        custom::Checkbox("Ignore Death", &Cheats::Players::IgnoreDeath);
                        custom::Separator_line();

                        custom::SliderFloat("Players Distance", &Cheats::Players::Distance, 1.f, 1000.f, "%0.1f");

                    }
                    custom::EndChild();
                }
                custom::EndGroup();

                ImGui::SameLine();

                custom::BeginGroup();
                {
                    custom::Child("SETTINGS", ImVec2((GetContentRegionMax().x - spacing.x * 3) / 2, (GetContentRegionMax().y - (60 + spacing.y * 2) * 2) + 10), true);
                    {
                        custom::Combo("BoxType", &Cheats::Players::DrawBox::SelectedType, Cheats::Players::DrawBox::Type, IM_ARRAYSIZE(Cheats::Players::DrawBox::Type), 3);
                        custom::Separator_line();

                        custom::SliderFloat("Box Size", &Cheats::Players::DrawBox::Size, 0.75f, 1.5f, "%.2f");
                        custom::Separator_line();

                        custom::Checkbox("Enable Gradient", &Cheats::Players::DrawBox::GradientEnabled);
                        custom::Separator_line();

                        custom::Checkbox("Outline", &Cheats::MenuUtils::OutlineEnabled);
                        custom::Separator_line();

                        if (Cheats::Players::DrawBox::GradientEnabled) {
                            custom::Checkbox("Use Custom Gradient", &Cheats::Players::DrawBox::UseCustomGradient);
                            custom::Separator_line();

                            custom::SliderFloat("Gradient Intensity", &Cheats::Players::DrawBox::GradientIntensity, 0.0f, 5.0f, "%.2f");
                            custom::Separator_line();

                            if (Cheats::Players::DrawBox::UseCustomGradient) {
                                ImGui::ColorEdit4("Top Color", (float*)&Cheats::Players::DrawBox::BoxGradientTopColor, Menu.ColorPickerFlags);
                                ImGui::ColorEdit4("Bottom Color", (float*)&Cheats::Players::DrawBox::BoxGradientBottomColor, Menu.ColorPickerFlags);
                                custom::Separator_line();
                            }
                        }

                        custom::Combo("LineType", &Cheats::Players::DrawLine::SelectedType, Cheats::Players::DrawLine::Type, IM_ARRAYSIZE(Cheats::Players::DrawLine::Type), 3);
                        custom::Separator_line();

                        custom::Combo("Health Bar Position", &Cheats::Players::DrawHealth::SelectedPosition, Cheats::Players::DrawHealth::Position, IM_ARRAYSIZE(Cheats::Players::DrawHealth::Position), 4);
                        custom::Separator_line();

                        custom::Combo("Armor Bar Position", &Cheats::Players::DrawArmor::SelectedPosition, Cheats::Players::DrawArmor::Position, IM_ARRAYSIZE(Cheats::Players::DrawArmor::Position), 4);
                        custom::Separator_line();


                    }
                    custom::EndChild();
                }
                custom::EndGroup();
            }

            else if (active_tab == 2)
            {
                custom::BeginGroup();
                {
                    custom::Child("EXPLOITS", ImVec2((GetContentRegionMax().x - spacing.x * 3) / 2, (GetContentRegionMax().y - (60 + spacing.y * 2) * 2) + 10), true);
                    {
                        static bool popup = false;

                        if (popup) {
                            ImGui::Begin("Popupbox", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
                            {
                                custom::Child("Popupbox", ImVec2(300, 170), ImGuiWindowFlags_NoBringToFrontOnFocus);
                                {
                                    custom::ColorEdit4("Color Palette", col, picker_flags);

                                    custom::Separator_line();

                                    custom::Combo("Combobox", &select1, items, IM_ARRAYSIZE(items), 3);
                                }
                                custom::EndChild();
                            }
                            ImGui::End();
                        }


                        custom::Checkbox("Infinite Ammo", &Cheats::Exploit::InfiniteAmmo);
                        custom::Separator_line();

                        custom::Checkbox("No Recoil", &Cheats::Exploit::NoRecoil);
                        custom::Separator_line();

                        custom::Checkbox("No Spread", &Cheats::Exploit::NoSpread);
                        custom::Separator_line();

                        custom::Checkbox("No Reload", &Cheats::Exploit::NoReload);
                        custom::Separator_line();

                        custom::Checkbox("Reload Ammo", &Cheats::Exploit::ReloadAmmo);
                        custom::Separator_line();
                        custom::Keybind("Reload Ammo Key", &Cheats::Exploit::ReloadAmmoKey, &m);
                        custom::Separator_line();

                        if (Cheats::Exploit::ReloadAmmo) {
                            custom::SliderFloat("Ammo Value", &Cheats::Exploit::ReloadValue, 1.f, 100.f, "%0.1f");
                            custom::Separator_line();
                        }

                        custom::Checkbox("Player Health Boost", &Cheats::Exploit::HealthBoost);
                        custom::Separator_line();
                        custom::Keybind("Player Health Boost Key", &Cheats::Exploit::HealthBoostKey, &m);
                        custom::Separator_line();

                        if (Cheats::Exploit::HealthBoost) {
                            custom::SliderFloat("Health Boost Value", &Cheats::Exploit::HealthBoostValue, 1.f, 100.f, "%0.1f");
                            custom::Separator_line();
                        }

                        custom::Checkbox("Player Armor Boost", &Cheats::Exploit::ArmorBoost);
                        custom::Separator_line();
                        custom::Keybind("Player Armor Boost Key", &Cheats::Exploit::ArmorBoostKey, &m);
                        custom::Separator_line();

                        if (Cheats::Exploit::ArmorBoost) {
                            custom::SliderFloat("Armor Boost Value", &Cheats::Exploit::ArmorBoostValue, 1.f, 100.f, "%0.1f");
                            custom::Separator_line();
                        }

                        custom::Checkbox("Damage Boost", &Cheats::Exploit::DamageBoost);
                        if (Cheats::Exploit::DamageBoost) {
                            custom::SliderFloat("Damage Value", &Cheats::Exploit::DamageBoostValue, 1.f, 100.f, "%0.1f");
                        }
                        custom::Separator_line();

                        custom::Checkbox("GodMode", &Cheats::Exploit::godMode);
                        custom::Separator_line();
                        custom::Keybind("GodMode Key", &Cheats::Exploit::godModeKey, &m);
                        custom::Separator_line();

                        custom::Checkbox("Friend Add", &Cheats::AimAssist::Silent::AddFried);
                        custom::Separator_line();
                        if (Cheats::AimAssist::Silent::AddFried) {

                            custom::Keybind("Friend Key", &Cheats::AimAssist::Silent::AddFriendKey, &m);
                            custom::Separator_line();


                        }

                        custom::Checkbox("Peek Assist", &Cheats::PeekAssist::Enabled);
                        if (Cheats::PeekAssist::Enabled) {
                            custom::Separator_line();

                            custom::Keybind("Peek Key", &Cheats::PeekAssist::Key, &m);
                            custom::Separator_line();
                            custom::SliderFloat("Max Distance", &Cheats::PeekAssist::MaxDistance, 5.0f, 50.0f, "%.1fm");
                            custom::Separator_line();

                            custom::Checkbox("Show Marker", &Cheats::PeekAssist::ShowMarker);
                            if (Cheats::PeekAssist::ShowMarker) {
                                custom::Separator_line();
                                custom::ColorEdit4("Marker Color", (float*)&Cheats::PeekAssist::MarkerColor, ImGuiColorEditFlags_NoInputs);
                                custom::Separator_line();
                                custom::SliderFloat("Marker Size", &Cheats::PeekAssist::MarkerSize, 4.0f, 20.0f, "%.1f");
                                custom::Separator_line();

                            }

                        }
                        custom::Separator_line();
                        custom::Checkbox("FreeCam", &Cheats::FreeCam::Enabled);
                        custom::Separator_line();
                        custom::Keybind("FreeCam Key", &Cheats::FreeCam::Key, &m);

                        if (Cheats::FreeCam::Enabled) {
                            custom::SliderFloat("FreeCam Speed", &Cheats::FreeCam::Speed, 0.1f, 5.0f, "%.1f");
                        }
                        custom::Separator_line();
                        custom::Checkbox("Strafe Boost", &Cheats::Exploit::StrafeBoost);
                        custom::Separator_line();

                        if (Cheats::Exploit::StrafeBoost) {
                        static int keyMode = 0;
                        custom::Keybind("Strafe Key", &Cheats::Exploit::StrafeMacro::Key, &keyMode);
                        custom::Separator_line();
                        custom::Combo("Preset", &Cheats::Exploit::StrafeMacro::SelectedPreset, Cheats::Exploit::StrafeMacro::Presets, 4);
                            custom::Separator_line();
                            custom::SliderInt("Delay (ms)", &Cheats::Exploit::StrafeMacro::Delay, 10, 200, "%d ms");
                            custom::Separator_line();
                        }

                        custom::Checkbox("Shaking", &Cheats::Exploit::Shaking);
                        custom::Separator_line();

                        if (Cheats::Exploit::Shaking) {
                            static int keyMode = 0;
                            custom::Keybind("Shaking Key", &Cheats::Exploit::ShakingKey, &keyMode);
                            custom::Separator_line();
                            custom::SliderFloat("Shaking Speed", &Cheats::Exploit::ShakingSpeed, 1.f, 30.f, "%.1f");
                            custom::Separator_line();
                            custom::SliderFloat("Shaking Amount", &Cheats::Exploit::ShakingAmount, 0.1f, 5.f, "%.1f");
                            custom::Separator_line();
                        }


                    }
                    custom::EndChild();
                }
                custom::EndGroup();

                ImGui::SameLine();

                custom::BeginGroup();
                {
                    custom::Child("SETTINGS", ImVec2((GetContentRegionMax().x - spacing.x * 3) / 2, (GetContentRegionMax().y - (60 + spacing.y * 2) * 2) + 10), true);
                    {


                        custom::SliderInt("Max Player Count", &maxPlayerCount, 0, 5000, "%d");
                        custom::Separator_line();

                        custom::SliderInt("Overlay Delay", &loopDelay, 0, 2000, "%d");
                        custom::Separator_line();

                        custom::Checkbox("Watermark", &watermark_options.watermark);
                        custom::Separator_line();


                        ImVec2 buttonSize = ImVec2(280, 50);
                        ImVec2 windowSize = ImGui::GetWindowSize();
                        ImGui::SetCursorPosX((windowSize.x - buttonSize.x) * 0.5f);


                        static const unsigned char encrypted_safe_unhook[] = {
                            0x52 ^ 0xAB, 0x61 ^ 0xAB, 0x66 ^ 0xAB, 0x65 ^ 0xAB, 0x20 ^ 0xAB,
                            0x55 ^ 0xAB, 0x6E ^ 0xAB, 0x68 ^ 0xAB, 0x6F ^ 0xAB, 0x6F ^ 0xAB, 0x6B ^ 0xAB
                        };
                        std::string safe_unhook_text = DecryptString(encrypted_safe_unhook, sizeof(encrypted_safe_unhook), 0xAB);

                        if (custom::Button("Unhook Cheat", buttonSize)) {


                            SecureZeroMemory(&safe_unhook_text[0], safe_unhook_text.size());

                            std::thread([]() {

                                exit(0);


                                }
                        );}

                    }
                    custom::EndChild();
                }
                custom::EndGroup();
            }
            else if (active_tab == 3)
            {
                custom::BeginGroup();
                {
                    custom::Child("Soon", ImVec2(GetContentRegionMax().x - spacing.x * 2, (GetContentRegionMax().y - (60 + spacing.y * 2) * 2) + 10), true);
                    {
                        ImVec2 buttonSize = ImVec2(280, 50);
                        ImVec2 windowSize = ImGui::GetWindowSize();
                        ImGui::SetCursorPosX((windowSize.x - buttonSize.x) * 0.5f);

                        if (custom::Button("Legit Config", buttonSize)) {
                            ApplyLegitConfig();
                        }
                    }
                    custom::EndChild();
                }
                custom::EndGroup();
            }

            ImGui::PopStyleVar();

            SetCursorPos(ImVec2(spacing.x, region.y - (60 + spacing.y)));
            custom::BeginGroup();
            {

                custom::Child("Page One", ImVec2((GetContentRegionMax().x - (spacing.x * 6)) / 4 - 2, 60), false);
                {

                    if (custom::ThemeButton("0", dark, ImVec2(GetContentRegionMax().x - spacing.x, GetContentRegionMax().y - spacing.y)))
                    {
                        dark = !dark;
                    }

                }
                custom::EndChild();

                SameLine();

                custom::SeparatorEx(ImGuiSeparatorFlags_Vertical, 2.f);

                SameLine();

                custom::Child("Page Two", ImVec2((GetContentRegionMax().x - (spacing.x * 6)) / 2, 60), false);
                {

                    if (custom::Page(0 == page, "c", ImVec2((GetContentRegionMax().x - spacing.x * 4) / 4, GetContentRegionMax().y - spacing.y))) page = 0;
                    SameLine();
                    if (custom::Page(1 == page, "d", ImVec2((GetContentRegionMax().x - spacing.x * 4) / 4, GetContentRegionMax().y - spacing.y))) page = 1;
                    SameLine();
                    if (custom::Page(2 == page, "e", ImVec2((GetContentRegionMax().x - spacing.x * 4) / 4, GetContentRegionMax().y - spacing.y))) page = 2;
                    SameLine();
                    if (custom::Page(3 == page, "f", ImVec2((GetContentRegionMax().x - spacing.x * 4) / 4, GetContentRegionMax().y - spacing.y))) page = 3;

                }
                custom::EndChild();

                SameLine();

                custom::SeparatorEx(ImGuiSeparatorFlags_Vertical, 2.f);

                SameLine();

                custom::Child("Page Three", ImVec2((GetContentRegionMax().x - (spacing.x * 6)) / 4 - 2, 60), false);
                {

                    if (custom::Button("b", ImVec2(GetContentRegionMax().x - spacing.x, GetContentRegionMax().y - spacing.y)));
                }
                custom::EndChild();

            }
            custom::EndGroup();

        }


        End();


        ImGui::PopStyleVar(2);
        }

        ImGui::RenderNotifications();

        Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


        g_pSwapChain->Present(1, 0);

    }


    exitLoop = true;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassA(wc.lpszClassName, wc.hInstance);


    if (Game.hProcess) {
        CloseHandle(Game.hProcess);
    }

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;

    sd.SampleDesc.Count = 4;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

