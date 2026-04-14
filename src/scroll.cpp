#include "scroll.h"
#include "mainLoop.h"
#include "CairoOverlay.h"
#include "MpvIpc.h"

#include <cairo/cairo.h>
#include <unistd.h>

// ── Scrolling state machine ───────────────────────────────────────────────────
enum class ScrollState {
    Scrolling,   // Text is moving left to right
    Waiting,     // Solid green background, no text
    Initialized
};

static ScrollState g_scroll_state = ScrollState::Initialized;
static float       g_scroll_x     = (float)OVERLAY_W;  // Start off-screen right
static constexpr float g_scroll_speed = SCROLL_SPEED_PX;  // pixels per frame
static constexpr int   g_wait_duration_frames = 100;    // 100 frames = 10 seconds at 100ms
static int             g_wait_frames_remaining = 0;
static constexpr const char* g_scroll_text = "UPL TRAIN CAM - 24 HOURS A DAY";
static float           g_text_width = 0.0f;
static int             g_text_baseline = 0;

// ── Initialize scrolling text (call once) ─────────────────────────────────────
void scroll_init() {
    cairo_set_font_size(g_cr, TICKER_FONT_SZ);
    cairo_text_extents_t ext;
    cairo_text_extents(g_cr, g_scroll_text, &ext);
    g_text_width = (float)ext.width;
    g_text_baseline = (int)(OVERLAY_H - BOTTOM_BAR_H + TICKER_FONT_SZ * 1);
    g_scroll_state = ScrollState::Scrolling;
    g_scroll_x = (float)OVERLAY_W;  // Start off-screen to the right
}

// ── Update scroll state (call every frame) ────────────────────────────────────
bool scroll_update() {
    if (g_scroll_state == ScrollState::Scrolling) {
        g_scroll_x -= g_scroll_speed;
        if (g_scroll_x + g_text_width < 0) {
            // Scrolling complete, wait 10 seconds
            g_scroll_state = ScrollState::Waiting;
            g_wait_frames_remaining = g_wait_duration_frames;
        }
    } else if (g_scroll_state == ScrollState::Waiting) {
        g_wait_frames_remaining--;
        if (g_wait_frames_remaining <= 0) {
            // Wait complete, start scrolling again
            g_scroll_state = ScrollState::Scrolling;
            g_scroll_x = (float)OVERLAY_W;
        }
    }
    return (g_scroll_state == ScrollState::Scrolling);
}

// ── Draw scroll bar (call after bounce_draw) ──────────────────────────────────
void scroll_draw() {
    // Always draw green background (extends full width to cover under side borders)
    cairo_set_source_rgba(g_cr, 0.0f, 1.0f, 0.0f, 1.0f);  // Green
    cairo_rectangle(g_cr,
        0,
        OVERLAY_H - BOTTOM_BAR_H,
        OVERLAY_W,
        BOTTOM_BAR_H);
    cairo_fill(g_cr);

    if (g_scroll_state == ScrollState::Scrolling) {
        // Draw scrolling text on green background
        cairo_set_font_size(g_cr, TICKER_FONT_SZ);
        cairo_set_source_rgba(g_cr, 0.0f, 0.0f, 0.0f, 1.0f);  // Black text
        cairo_move_to(g_cr, g_scroll_x, (float)g_text_baseline);
        cairo_show_text(g_cr, g_scroll_text);
    }
}

// ── Shutdown scrolling ────────────────────────────────────────────────────────
void scroll_shutdown() {
    g_scroll_state = ScrollState::Initialized;
    g_scroll_x = 0.0f;
    g_wait_frames_remaining = 0;
    g_text_width = 0.0f;
}
