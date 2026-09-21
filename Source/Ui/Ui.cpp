
#include "Ui/Ui.h"
#include "Platform/Platform.h"
#include "Core/Log/Log.h"

#include <string>

namespace Ui {

static const int SCREEN_W      = 1280;
static const int SCREEN_H      = 720;

static const int MARGIN_X      = 60;
static const int TITLE_Y       = 40;
static const int LINE_HEIGHT   = 40;

static const unsigned COL_BG       = 0x0F1410;
static const unsigned COL_TITLE    = 0x39FF14;
static const unsigned COL_TEXT     = 0xE0E0E0;
static const unsigned COL_SELECTED = 0x107C10;
static const unsigned COL_HINT     = 0x808080;

static void DrawMultilineBody(const std::string& body,
                              int startY,
                              unsigned color) {
    int y = startY;
    std::string line;

    for (size_t i = 0; i <= body.size(); ++i) {
        if (i == body.size() || body[i] == '\n') {
            Platform::UiText(MARGIN_X, y, line, color);
            y += LINE_HEIGHT;
            line.clear();
        } else {
            line += body[i];
        }
    }
}

static void DrawFooterHint(const std::string& text) {
    Platform::UiText(MARGIN_X, SCREEN_H - 50, text, COL_HINT);
}

void Message(const std::string& title,
             const std::string& body) {
    Platform::UiClear();
    Platform::UiRect(0, 0, SCREEN_W, SCREEN_H, COL_BG);

    Platform::UiRect(0, 0, SCREEN_W, 80, COL_SELECTED);
    Platform::UiText(MARGIN_X, TITLE_Y, title, COL_TITLE);

    DrawMultilineBody(body, 140, COL_TEXT);

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

    Platform::UiRect(0, 0, SCREEN_W, 80, COL_SELECTED);
    Platform::UiText(MARGIN_X, TITLE_Y, title, COL_TITLE);

    DrawMultilineBody(body, 140, COL_TEXT);

    Platform::UiRect(SCREEN_W - 420, SCREEN_H - 120, 160, 60, COL_SELECTED);
    Platform::UiText(SCREEN_W - 400, SCREEN_H - 100, "A = Yes", COL_TITLE);

    Platform::UiRect(SCREEN_W - 240, SCREEN_H - 120, 160, 60, COL_BG);
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

    if (!Platform::ShowKeyboard(title, initial, out)) {
        return false;
    }

    if (out.empty()) {
        return false;
    }

    Log::Info("Prompt result: " + out);
    return true;
}

} // namespace Ui
