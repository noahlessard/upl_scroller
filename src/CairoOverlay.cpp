#include "CairoOverlay.h"
#include "mainLoop.h"
#include "MpvIpc.h"

#include <cairo/cairo.h>
#include <vector>
#include <thread>
#include <chrono>

// ── Drawing helpers ───────────────────────────────────────────────────────────
void clear_to_transparent() {
    cairo_save(g_cr);
    cairo_set_operator(g_cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(g_cr);
    cairo_restore(g_cr);
}

static void set_green() { cairo_set_source_rgba(g_cr, 0, 1, 0, 1); }
static void set_black() { cairo_set_source_rgba(g_cr, 0, 0, 0, 1); }
static void set_white() { cairo_set_source_rgba(g_cr, 1, 1, 1, 1); }

// ── Alert window ──────────────────────────────────────────────────────────────
static std::vector<std::string> wrap_pixels(std::string_view text, double max_w) {
    std::vector<std::string> lines;
    std::string current;

    auto measure = [&](const std::string& s) -> double {
        cairo_text_extents_t e;
        cairo_text_extents(g_cr, s.c_str(), &e);
        return e.width;
    };

    size_t pos = 0;
    while (pos < text.size()) {
        size_t sp = text.find(' ', pos);
        if (sp == std::string_view::npos) { sp = text.size(); }
        std::string word(text.substr(pos, sp - pos));
        std::string trial = current;
        if (!current.empty()) { trial += ' '; }
        trial += word;
        if (!current.empty() && measure(trial) > max_w) {
            lines.push_back(current);
            current = word;
        } else {
            current = trial;
        }
        pos = sp + 1;
    }
    if (!current.empty()) { lines.push_back(current); }
    return lines;
}

void cairo_create_alert(std::string_view title, std::string_view body, float duration_s) {
    clear_to_transparent();

    cairo_set_font_size(g_cr, ALERT_BODY_SZ);
    double max_content_w = 380.0;
    auto lines = wrap_pixels(body, max_content_w);

    double line_h = ALERT_BODY_SZ + 6;
    double box_w  = max_content_w + 40;
    double box_h  = ALERT_TITLE_SZ + (double)lines.size() * line_h + 30;
    double bx     = (OVERLAY_W - box_w) / 2.0;
    double by     = (OVERLAY_H - box_h) / 2.0;

    // Red fill
    cairo_set_source_rgba(g_cr, 0.78, 0, 0, 0.97);
    cairo_rectangle(g_cr, bx, by, box_w, box_h);
    cairo_fill(g_cr);

    // White border
    set_white();
    cairo_set_line_width(g_cr, 2);
    cairo_rectangle(g_cr, bx, by, box_w, box_h);
    cairo_stroke(g_cr);

    // Title
    cairo_set_font_size(g_cr, ALERT_TITLE_SZ);
    set_white();
    cairo_move_to(g_cr, bx + 10, by + ALERT_TITLE_SZ + 6);
    cairo_show_text(g_cr, title.data());

    // Body
    cairo_set_font_size(g_cr, ALERT_BODY_SZ);
    for (size_t i = 0; i < lines.size(); i++) {
        cairo_move_to(g_cr, bx + 10,
                      by + ALERT_TITLE_SZ + 6 + (double)(i + 1) * line_h + 8);
        cairo_show_text(g_cr, lines[i].c_str());
    }

    mpv_present_overlay();
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(duration_s * 1000.0f)));
}
