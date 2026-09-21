#include "Ui/Ui.h"
#include "Platform/Platform.h"
#include "Core/Log/Log.h"

#include <string>
#include <vector>

namespace Ui {

static const int SCREEN_W      = 1280;
static const int SCREEN_H      = 720;

static const int TITLE_Y       = 40;
static const int TITLE_H       = 80;
static const int PANEL_X       = 60;

static const int LIST_Y        = 160;
static const int ITEM_H        = 60;
static const int ITEM_GAP      = 4;
static const int MAX_VISIBLE   = 8;

static const unsigned COL_BG       = 0x000000;
static const unsigned COL_TITLEBAR = 0x107C10;
static const unsigned COL_ACCENT   = 0x39FF14;
static const unsigned COL_ITEM     = 0x101410;
static const unsigned COL_SEL      = 0x1B3D1B;
static const unsigned COL_TEXT     = 0xFFFFFF;
static const unsigned COL_HINT     = 0x808080;

static int ClampScroll(int sel, int top, int total) {
    if (sel < top) top = sel;
    else if (sel >= top + MAX_VISIBLE) top = sel - MAX_VISIBLE + 1;

    if (top < 0) top = 0;
    if (total > MAX_VISIBLE && top > total - MAX_VISIBLE)
        top = total - MAX_VISIBLE;
    return top;
}

int Menu(const std::string& title,
         const std::vector<std::string>& items) {

    if (items.empty()) {
        Message(title, "Nothing to show here.");
        return -1;
    }

    const int total = (int)items.size();
    int sel  = 0;
    int top  = 0;

    while (true) {
        Platform::UiClear();
        Platform::UiRect(0, 0, SCREEN_W, SCREEN_H, COL_BG);

        // Title bar (green Xbox dash style)
        Platform::UiRect(0, 0, SCREEN_W, TITLE_H, COL_TITLEBAR);
        Platform::UiText(PANEL_X, TITLE_Y, title, COL_ACCENT);

        // List
        top = ClampScroll(sel, top, total);

        for (int row = 0; row < MAX_VISIBLE; ++row) {
            int idx = top + row;
            if (idx >= total) break;

            int y = LIST_Y + row * (ITEM_H + ITEM_GAP);
            int w = SCREEN_W - 2 * PANEL_X;

            if (idx == sel) {
                Platform::UiRect(PANEL_X, y, w, ITEM_H, COL_SEL);
                Platform::UiRect(PANEL_X, y, 6, ITEM_H, COL_ACCENT);
                Platform::UiText(PANEL_X + 30, y + 18, items[idx], COL_ACCENT);
            } else {
                Platform::UiRect(PANEL_X, y, w, ITEM_H, COL_ITEM);
                Platform::UiText(PANEL_X + 30, y + 18, items[idx], COL_TEXT);
            }
        }

        // Scroll indicator
        if (total > MAX_VISIBLE) {
            std::string pos = std::to_string(sel + 1)
                            + " / " + std::to_string(total);
            Platform::UiText(SCREEN_W - PANEL_X - 120, TITLE_Y, pos, COL_HINT);
        }

        // Footer hint
        Platform::UiText(PANEL_X, SCREEN_H - 50,
                         "A = Select    B = Back", COL_HINT);

        Platform::UiPresent();

        Platform::Button b = Platform::PollInput();

        if (b == Platform::BTN_UP) {
            sel = (sel - 1 + total) % total;
        } else if (b == Platform::BTN_DOWN) {
            sel = (sel + 1) % total;
        } else if (b == Platform::BTN_A) {
            return sel;
        } else if (b == Platform::BTN_B || b == Platform::BTN_BACK) {
            return -1;
        }
    }
}

} // namespace Ui
