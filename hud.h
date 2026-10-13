#ifndef HUD_H
#define HUD_H

// the ui panel (imgui)

#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>

#include "benchmark.h"
#include "gpu_monitor.h"
#include "scenes.h"

namespace hud {

const ImVec4 Text   = ImVec4(0.92f, 0.92f, 0.93f, 1.00f);
const ImVec4 Muted  = ImVec4(0.54f, 0.54f, 0.58f, 1.00f);
const ImVec4 Accent = ImVec4(0.96f, 0.56f, 0.22f, 1.00f);
const ImVec4 Warm   = ImVec4(0.98f, 0.74f, 0.30f, 1.00f);
const ImVec4 Hot    = ImVec4(0.98f, 0.36f, 0.32f, 1.00f);

inline ImFont* regular = nullptr;
inline ImFont* semibold = nullptr;

// use segoe ui if its there, otherwise the default imgui font
inline void loadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    const char* regularPath = "C:\\Windows\\Fonts\\segoeui.ttf";
    const char* semiboldPath = "C:\\Windows\\Fonts\\seguisb.ttf";
    if (std::ifstream(regularPath).good()) regular = io.Fonts->AddFontFromFileTTF(regularPath, 17.0f);
    if (std::ifstream(semiboldPath).good()) semibold = io.Fonts->AddFontFromFileTTF(semiboldPath, 17.0f);
    if (!regular) regular = io.Fonts->AddFontDefault();
    if (!semibold) semibold = regular;
}

inline void applyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowPadding    = ImVec2(20.0f, 18.0f);
    s.WindowRounding   = 10.0f;
    s.WindowBorderSize = 0.0f;
    s.FramePadding     = ImVec2(10.0f, 7.0f);
    s.FrameRounding    = 6.0f;
    s.ItemSpacing      = ImVec2(8.0f, 8.0f);
    s.PopupRounding    = 6.0f;
    s.PopupBorderSize  = 0.0f;
    s.GrabRounding     = 6.0f;
    s.DisabledAlpha    = 0.45f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text]           = Text;
    c[ImGuiCol_TextDisabled]   = Muted;
    c[ImGuiCol_WindowBg]       = ImVec4(0.07f, 0.07f, 0.08f, 0.90f);
    c[ImGuiCol_PopupBg]        = ImVec4(0.11f, 0.11f, 0.12f, 0.98f);
    c[ImGuiCol_Border]         = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
    c[ImGuiCol_FrameBg]        = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.09f);
    c[ImGuiCol_FrameBgActive]  = ImVec4(1.0f, 1.0f, 1.0f, 0.12f);
    c[ImGuiCol_Button]         = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
    c[ImGuiCol_ButtonHovered]  = ImVec4(1.0f, 1.0f, 1.0f, 0.10f);
    c[ImGuiCol_ButtonActive]   = ImVec4(1.0f, 1.0f, 1.0f, 0.14f);
    c[ImGuiCol_Header]         = ImVec4(1.0f, 1.0f, 1.0f, 0.08f);
    c[ImGuiCol_HeaderHovered]  = ImVec4(1.0f, 1.0f, 1.0f, 0.11f);
    c[ImGuiCol_HeaderActive]   = ImVec4(1.0f, 1.0f, 1.0f, 0.14f);
    c[ImGuiCol_Separator]      = ImVec4(1.0f, 1.0f, 1.0f, 0.07f);
    c[ImGuiCol_NavCursor]      = ImVec4(1.0f, 1.0f, 1.0f, 0.30f);
}

inline std::string withCommas(int n) {
    std::string s = std::to_string(n);
    for (int i = (int)s.size() - 3; i > 0; i -= 3) s.insert(i, ",");
    return s;
}

inline void label(const char* text) {
    ImGui::TextColored(Muted, "%s", text);
}

// big number with a label under it
inline void stat(const char* value, const char* caption, ImVec4 color) {
    ImGui::BeginGroup();
    ImGui::PushFont(semibold, 32.0f);
    ImGui::TextColored(color, "%s", value);
    ImGui::PopFont();
    ImGui::PushFont(nullptr, 14.0f);
    ImGui::TextColored(Muted, "%s", caption);
    ImGui::PopFont();
    ImGui::EndGroup();
}

// thin progress bar, fraction is 0 to 1
inline void bar(float fraction, ImVec4 color, float height = 4.0f) {
    fraction = std::clamp(fraction, 0.0f, 1.0f);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + height), ImGui::GetColorU32(ImVec4(1, 1, 1, 0.08f)), height * 0.5f);
    if (fraction > 0.0f) {
        dl->AddRectFilled(p, ImVec2(p.x + w * fraction, p.y + height), ImGui::GetColorU32(color), height * 0.5f);
    }
    ImGui::Dummy(ImVec2(w, height));
}

// row of buttons where one is selected (scene picker)
inline void segmented(const char* const labels[], int count, int* current) {
    const float gap = 4.0f;
    float w = (ImGui::GetContentRegionAvail().x - gap * (count - 1)) / count;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap, 0.0f));
    for (int i = 0; i < count; ++i) {
        if (i > 0) ImGui::SameLine();
        bool selected = (*current == i);
        ImVec4 bg = selected ? ImVec4(1, 1, 1, 0.14f) : ImVec4(1, 1, 1, 0.04f);
        ImGui::PushStyleColor(ImGuiCol_Button, bg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected ? bg : ImVec4(1, 1, 1, 0.08f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.16f));
        ImGui::PushStyleColor(ImGuiCol_Text, selected ? Text : Muted);
        if (ImGui::Button(labels[i], ImVec2(w, 32.0f))) *current = i;
        ImGui::PopStyleColor(4);
    }
    ImGui::PopStyleVar();
}

inline bool primaryButton(const char* text) {
    ImGui::PushStyleColor(ImGuiCol_Button, Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.00f, 0.63f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.88f, 0.48f, 0.16f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.08f, 0.06f, 0.05f, 1.0f));
    ImGui::PushFont(semibold, 0.0f);
    bool pressed = ImGui::Button(text, ImVec2(-FLT_MIN, 38.0f));
    ImGui::PopFont();
    ImGui::PopStyleColor(4);
    return pressed;
}

inline ImVec4 temperatureColor(unsigned int c) {
    if (c >= 85) return Hot;
    if (c >= 75) return Warm;
    return Text;
}

// gpu name on the left, fps on the right
inline void header(const std::string& gpuName, double fps) {
    char fpsText[32];
    snprintf(fpsText, sizeof(fpsText), "%.0f fps", fps);
    float fpsWidth = ImGui::CalcTextSize(fpsText).x;
    float left = ImGui::GetCursorPosX();
    float avail = ImGui::GetContentRegionAvail().x;
    ImVec2 p = ImGui::GetCursorScreenPos();

    ImGui::PushFont(semibold, 0.0f);
    ImGui::PushClipRect(p, ImVec2(p.x + avail - fpsWidth - 12.0f, p.y + 100.0f), true); // clip long gpu names
    ImGui::TextUnformatted(gpuName.c_str());
    ImGui::PopClipRect();
    ImGui::PopFont();
    ImGui::SameLine(left + avail - fpsWidth);
    ImGui::TextColored(Muted, "%s", fpsText);
}

inline void telemetry(const GPUMonitor& gpu, float shownUtil) {
    if (!gpu.isAvailable) {
        ImGui::TextColored(Muted, "Temperature and utilization need\nan NVIDIA GPU (NVML).");
        return;
    }
    char tempText[16], utilText[16];
    snprintf(tempText, sizeof(tempText), "%u°C", gpu.temperatureC);
    snprintf(utilText, sizeof(utilText), "%u%%", gpu.utilizationGpu);

    float half = ImGui::GetContentRegionAvail().x * 0.5f;
    stat(tempText, "Temperature", temperatureColor(gpu.temperatureC));
    ImGui::SameLine(half + ImGui::GetStyle().WindowPadding.x);
    stat(utilText, "GPU utilization", Text);

    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    bar(shownUtil / 100.0f, Accent);
}

// resolution dropdown
inline void resolutionDropdown(int& resIndex) {
    ImGui::SetNextItemWidth(-FLT_MIN);
    char preview[48];
    snprintf(preview, sizeof(preview), "%s  ·  %s", resolutions[resIndex].name, resolutions[resIndex].size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));
    bool open = ImGui::BeginCombo("##resolution", preview);
    ImGui::PopStyleVar();
    if (!open) return;

    for (int i = 0; i < RESOLUTION_COUNT; ++i) {
        bool selected = (i == resIndex);
        ImGui::PushID(i);
        float rowStart = ImGui::GetCursorPosX();
        float rowWidth = ImGui::GetContentRegionAvail().x;
        if (ImGui::Selectable("##res", selected)) resIndex = i;
        if (selected) ImGui::SetItemDefaultFocus();
        ImGui::SameLine(rowStart);
        ImGui::TextUnformatted(resolutions[i].name);
        float sizeWidth = ImGui::CalcTextSize(resolutions[i].size).x;
        ImGui::SameLine(rowStart + rowWidth - sizeWidth);
        ImGui::TextColored(Muted, "%s", resolutions[i].size);
        ImGui::PopID();
    }
    ImGui::EndCombo();
}

// run button / progress bar / score
inline void benchmarkSection(Benchmark& bench, double now) {
    if (!bench.running) {
        if (primaryButton("Run benchmark")) bench.start(now);
    } else {
        double elapsed = bench.elapsed(now);
        int remaining = std::max(0, (int)std::ceil(Benchmark::DURATION - elapsed));

        ImGui::Text("Running");
        ImGui::SameLine();
        ImGui::TextColored(Muted, "%ds left", remaining);
        bar((float)(elapsed / Benchmark::DURATION), Accent, 6.0f);
        ImGui::Dummy(ImVec2(0.0f, 2.0f));
        if (ImGui::Button("Cancel", ImVec2(-FLT_MIN, 32.0f))) bench.cancel();
    }

    if (bench.hasResult && !bench.running) {
        const BenchmarkResult& r = bench.result;
        ImGui::Dummy(ImVec2(0.0f, 8.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        label("Score");
        ImGui::PushFont(semibold, 44.0f);
        ImGui::TextUnformatted(withCommas(r.score).c_str());
        ImGui::PopFont();
        ImGui::TextColored(Muted, "%s  ·  %s  ·  %.0f fps avg",
            sceneNames[r.scene], resolutions[r.resIndex].name, r.avgFps);
    }
}

inline void draw(const std::string& gpuName, double fps, const GPUMonitor& gpu, float shownUtil,
                 int& scene, int& resIndex, Benchmark& bench, double now) {
    ImGui::SetNextWindowPos(ImVec2(16.0f, 16.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_Always); // 0 height = auto fit
    ImGui::Begin("##hud", NULL,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);

    header(gpuName, fps);
    ImGui::Dummy(ImVec2(0.0f, 6.0f));

    telemetry(gpu, shownUtil);
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    // cant change settings during a run
    ImGui::BeginDisabled(bench.running);
    label("Scene");
    segmented(sceneNames, 3, &scene);
    ImGui::Dummy(ImVec2(0.0f, 4.0f));
    label("Resolution");
    resolutionDropdown(resIndex);
    ImGui::EndDisabled();
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    benchmarkSection(bench, now);

    ImGui::End();
}

} // namespace hud

#endif // HUD_H
