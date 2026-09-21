#include "Ui/Ui.h"
#include "Platform/Platform.h"
#include "Core/Log/Log.h"

#include <string>

namespace Ui {

// ---------- Layout ----------
static const int SCREEN_W      = 1280;
static const int SCREEN_H      = 720;

static const int TITLE_Y       = 40;
static const int TITLE_H       = 80;
static const int PANEL_X       = 60;
static const int PANEL_W       = SCREEN_W - 2 * PANEL_X;

static const int BODY_X        = 80;
static const int BODY_Y        = 180;
static const int LINE_HEIGHT   = 40;

// ---------- Xbox 360-ish palette ----------
static const unsigned COL_BG         = 0x000000;
static const unsigned COL_PANEL      = 0x101410;
static const unsigned COL_TITLEBAR   = 0x107C10;
static const unsigned COL_ACCENT     = 0x39FF14;
static const unsigned COL_TEXT       = 0xFFFFFF;
static const unsigned COL_HINT       = 0x808080;

static void DrawTitleBar(const std::string& title) {
    Platform::UiRect(0, 0, SCREEN_W, TITLE_H, COL_TITLEBAR);
    Platform::UiText(PANEL_X, TITLE_Y, title, COL_ACCENT);
}

static void DrawFooterHint(const std::string& text) {
    Platform::UiText(PANEL_X, SCREEN_H - 50, text, COL_HINT);
}

static void DrawMultilineBody(const std::string& body, int startY) {
    int y = startY;
    std::string line;

    for (size_t i = 0; i <= body.size(); ++i) {
        if (i == body.size() || body[i] == '\n') {
            Platform::UiText(BODY_X, y, line, COL_TEXT);
            y += LINE_HEIGHT;
            line.clear();
        } else {
            line += body[i];
        }
    }
}

void Message(const std::string& title,
             const std::string& body) {
    Platform::UiClear();
    Platform::UiRect(0, 0, SCREEN_W, SCREEN_H, COL_BG);
    DrawTitleBar(title);

    DrawMultilineBody(body, BODY_Y);

    DrawFooterHint("A = OK");
    Platform::UiPresent();

    while (true) {
        Platform::Button b = Platform::PollInput();
        if (b == Platform::BTN_A ||
            b == Platform::BTN_B ||
            b == Platform::BTN_BACK) {
            return;
        }
    }
}

bool Confirm(const std::string& title,
             const std::string& body) {
    Platform::UiClear();
    Platform::UiRect(0, 0, SCREEN_W, SCREEN_H, COL_BG);
    DrawTitleBar(title);

    DrawMultilineBody(body, BODY_Y);

    Platform::UiRect(SCREEN_W - 420, SCREEN_H - 120, 160, 60, COL_TITLEBAR);
    Platform::UiText(SCREEN_W - 400, SCREEN_H - 100, "A = Yes", COL_ACCENT);

    Platform::UiRect(SCREEN_W - 240, SCREEN_H - 120, 160, 60, COL_PANEL);
    Platform::UiText(SCREEN_W - 220, SCREEN_H - 100, "B = No", COL_TEXT);

    Platform::UiPresent();

    while (true) {
        Platform::Button b = Platform::PollInput();
        if (b == Platform::BTN_A)                            return true;
        if (b == Platform::BTN_B || b == Platform::BTN_BACK) return false;
    }
}

bool Prompt(const std::string& title,
            const std::string& initial,
            std::string& out) {
    Log::Info("Prompt: " + title);

    if (!Platform::ShowKeyboard(title, initial, out)) return false;
    if (out.empty()) return false;

    Log::Info("Prompt result: " + out);
    return true;
}

} // namespace Ui
