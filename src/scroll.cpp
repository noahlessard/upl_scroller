#include "scroll.h"
#include "mainLoop.h"
#include "CairoOverlay.h"
#include "MpvIpc.h"

#include <cairo/cairo.h>
#include <unistd.h>

// ── Constructor ───────────────────────────────────────────────────────────────
ScrollText::ScrollText(std::string_view text_, int y_px, float speed_px, Dir dir_)
    : text(text_), y(y_px), speed(speed_px), dir(dir_), done(false)
{
    cairo_set_font_size(g_cr, TICKER_FONT_SZ);
    cairo_text_extents_t ext;
    cairo_text_extents(g_cr, text.c_str(), &ext);
    text_w = (float)ext.width;
    x = (dir == Dir::Left) ? (float)OVERLAY_W : -text_w;
}

void scroll_run(std::vector<ScrollText>& scrolls) {
    while (true) {
        cairo_draw_border(false);

        cairo_save(g_cr);
        cairo_rectangle(g_cr,
            SIDE_BORDER_W,
            OVERLAY_H - BOTTOM_BAR_H,
            OVERLAY_W - 2 * SIDE_BORDER_W,
            BOTTOM_BAR_H);
        cairo_clip(g_cr);

        cairo_set_font_size(g_cr, TICKER_FONT_SZ);
        cairo_set_source_rgba(g_cr, 0, 0, 0, 1);

        for (auto& s : scrolls) {
            if (!s.done) {
                cairo_move_to(g_cr, s.x, s.y);
                cairo_show_text(g_cr, s.text.c_str());
            }
        }

        cairo_restore(g_cr);
        mpv_present_overlay();

        bool all_done = true;
        for (auto& s : scrolls) {
            if (s.done) continue;
            s.x += (s.dir == Dir::Left) ? -s.speed : s.speed;
            s.done = (s.dir == Dir::Left)
                ? (s.x + s.text_w < 0)
                : (s.x > OVERLAY_W);
            if (!s.done) all_done = false;
        }

        if (all_done) break;
        usleep(FRAME_MS * 1000);
    }
}
