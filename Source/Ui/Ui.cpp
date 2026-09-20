#include "Ui.h"
#include "Platform/Platform.h"
#include "Core/Log/Log.h"

#include <string>
#include <vector>

namespace Ui {

// ------------------------------------------------------------
// Layout constants
// ------------------------------------------------------------

static const int SCREEN_W      = 1280;
static const int SCREEN_H      = 720;

static const int MARGIN_X      = 60;
static const int MARGIN_Y      = 60;

static const int TITLE_Y       = 40;
static const int LIST_START_Y  = 140;
static const int LINE_HEIGHT   = 40;
static const int MAX_VISIBLE   = 12;

// Colors are RGB hex, format 0xRRGGBB (drawn by Platform layer).
static const unsigned COL_BG         = 0x0F1410;
static const unsigned COL_TITLE      = 0x39FF14;
static const unsigned COL_TEXT       = 0xE0E0E0;
static const unsigned COL_SELECTED   = 0x107C10;
static const unsigned COL_HINT       = 0x808080;

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

static int ClampScroll(int selected, int scrollTop, int total) {
    if (selected < scrollTop) {
        scrollTop = selected;
    } else if (selected >= scrollTop + MAX_VISIBLE) {
        scrollTop = selected - MAX_VISIBLE + 1;
    }
    if (scrollTop < 0) scrollTop = 0;
    if (scrollTop > total - MAX_VISIBLE && total > MAX_VISIBLE) {
        scrollTop = total - MAX_VISIBLE;
    }
    return scrollTop;
}

static void DrawFooterHint(const std::string& text) {
    Platform::UiText(MARGIN_X, SCREEN_H - 50, text, COL_HINT);
}

// ------------------------------------------------------------
// Menu
// ------------------------------------------------------------

int Menu(const std::string& title,
         const std::vector<std::string>& items) {

    if (items.empty()) {
        Message(title, "Nothing to show here.");
        return -1;
    }

    const int total = (int)items.size();
    int selected  = 0;
    int scrollTop = 0;

    while (true) {
        Platform::UiClear();

        // Background
        Platform::UiRect(0, 0, SCREEN_W, SCREEN_H, COL_BG);

        // Title
        Platform::UiText(MARGIN_X, TITLE_Y, title, COL_TITLE);

        // Divider
        Platform::UiRect(MARGIN_X, TITLE_Y + 50,
                         SCREEN_W - 2 * MARGIN_X, 2, COL_TITLE);

        // Items
        scrollTop = ClampScroll(selected, scrollTop, total);

        for (int row = 0; row < MAX_VISIBLE; ++row) {
            int idx = scrollTop + row;
            if (idx >= total) break;

            int y = LIST_START_Y + row * LINE_HEIGHT;

            if (idx == selected) {
                Platform::UiRect(MARGIN_X - 10, y - 6,
                                 SCREEN_W - 2 * MARGIN_X + 20,
                                 LINE_HEIGHT, COL_SELECTED);
                Platform::UiText(MARGIN_X, y, "> " + items[idx], COL_TITLE);
            } else {
                Platform::UiText(MARGIN_X, y, "  " + items[idx], COL_TEXT);
            }
        }

        // Scroll indicator
        if (total > MAX_VISIBLE) {
            std::string pos = std::to_string(selected + 1)
                            + " / "
                            + std::to_string(total);
            Platform::UiText(SCREEN_W - MARGIN_X - 120, TITLE_Y, pos, COL_HINT);
        }

        DrawFooterHint("A = Select   B = Back");

        Platform::UiPresent();

        // Input
        Platform::Button b = Platform::PollInput();

        if (b == Platform::BTN_UP) {
            selected = (selected - 1 + total) % total;
        } else if (b == Platform::BTN_DOWN) {
            selected = (selected + 1) % total;
        } else if (b == Platform::BTN_A) {
            Log::Info("Menu selection: " + items[selected]);
            return selected;
        } else if (b == Platform::BTN_B || b == Platform::BTN_BACK) {
            return -1;
        }
    }
}

// ------------------------------------------------------------
// Message
// ------------------------------------------------------------

void Message(const std::string& title,
             const std::string& body) {
    Platform::UiClear();
    Platform::UiRect(0, 0, SCREEN_W, SCREEN_H, COL_BG);

    // Title bar
    Platform::UiRect(0, 0, SCREEN_W, 80, COL_SELECTED);
    Platform::UiText(MARGIN_X, 30, title, COL_TITLE);

    // Body — split on '\n'
    int y = 140;
    std::string line;
    for (size_t i = 0; i <= body.size(); ++i) {
        if (i == body.size() || body[i] == '\n') {
            Platform::UiText(MARGIN_X, y, line, COL_TEXT);
            y += LINE_HEIGHT;
            line.clear();
        } else {
            line += body[i];
        }
    }

    DrawFooterHint("A = OK");
    Platform::UiPresent();

    while (true) {
        Platform::Button b = Platform::PollInput();
        if (b == Platform::BTN_A || b == Platform::BTN_B ||
            b == Platform::BTN_BACK) {
            return;
        }
    }
}

// ------------------------------------------------------------
// Confirm
// ------------------------------------------------------------

bool Confirm(const std::string& title,
             const std::string& body) {
    Platform::UiClear();
    Platform::UiRect(0, 0, SCREEN_W, SCREEN_H, COL_BG);

    // Title bar
    Platform::UiRect(0, 0, SCREEN_W, 80, COL_SELECTED);
    Platform::UiText(MARGIN_X, 30, title, COL_TITLE);

    // Body
    int y = 140;
    std::string line;
    for (size_t i = 0; i <= body.size(); ++i) {
        if (i == body.size() || body[i] == '\n') {
            Platform::UiText(MARGIN_X, y, line, COL_TEXT);
            y += LINE_HEIGHT;
            line.clear();
        } else {
            line += body[i];
        }
    }

    // Buttons
    Platform::UiRect(SCREEN_W - 420, SCREEN_H - 120, 160, 60, COL_SELECTED);
    Platform::UiText(SCREEN_W - 400, SCREEN_H - 100, "A = Yes", COL_TITLE);

    Platform::UiRect(SCREEN_W - 240, SCREEN_H - 120, 160, 60, COL_BG);
    Platform::UiText(SCREEN_W - 220, SCREEN_H - 100, "B = No", COL_TEXT);

    Platform::UiPresent();

    while (true) {
        Platform::Button b = Platform::PollInput();
        if (b == Platform::BTN_A)                        return true;
        if (b == Platform::BTN_B || b == Platform::BTN_BACK) return false;
    }
}

// ------------------------------------------------------------
// Prompt
// ------------------------------------------------------------

bool Prompt(const std::string& title,
            const std::string& initial,
            std::string& out) {
    // The on-screen keyboard is a Platform primitive. This
    // wrapper just gives it a title and hands back the result.
    Log::Info("Prompt: " + title);

    if (!Platform::ShowKeyboard(title, initial, out)) {
        return false;
    }

    // Reject empty results silently
    if (out.empty()) {
        return false;
    }

    Log::Info("Prompt result: " + out);
    return true;
}

} // namespace Ui
