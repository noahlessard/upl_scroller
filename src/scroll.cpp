#include "scroll.h"
#include "mainLoop.h"
#include "CairoOverlay.h"
#include "MpvIpc.h"

#include <cairo/cairo.h>
#include <unistd.h>
#include <chrono>

// ============================================================================
// SCROLLING STATE MACHINE
// ============================================================================
enum class ScrollState {
    Initialized,
    Scrolling,      // Text is moving left to right
    Waiting         // Solid green background, no text
};

// Timing configuration (in frames at 100ms = 10fps)
static constexpr int WAIT_DURATION_FRAMES = 150;  // 15 seconds

// ============================================================================
// SCROLL STATE
// ============================================================================
static ScrollState g_scroll_state = ScrollState::Initialized;
static float       g_scroll_x     = 0.0f;
static int         g_wait_frames_remaining = 0;

// Current events being scrolled
static size_t                   g_current_event_index = 0;

// Text rendering state
static float       g_text_width = 0.0f;
static int         g_text_baseline = 0;
static int         g_letter_spacing = 2;  // pixels between events

// Accessors for state that avoids non-POD static warnings
static std::vector<ScrollEvent>& get_events() {
    static std::vector<ScrollEvent> events;
    return events;
}

// ============================================================================
// INITIALIZATION
// ============================================================================
void scroll_init() {
    // Initialize Cairo for text measurements
    cairo_set_font_size(g_cr, TICKER_FONT_SZ);

    // Load initial random events
    get_events() = get_random_events(10);

    if (!get_events().empty()) {
        cairo_text_extents_t ext;
        cairo_text_extents(g_cr, get_events()[0].c_str(), &ext);
        g_text_width = (float)ext.width;
        g_text_baseline = (int)(OVERLAY_H - BOTTOM_BAR_H + TICKER_FONT_SZ * 1);

        g_scroll_state = ScrollState::Scrolling;
        g_scroll_x = (float)OVERLAY_W;  // Start off-screen to the right
    }
}

// ============================================================================
// SCROLL LOGIC
// ============================================================================
static void next_event() {
    if (get_events().empty()) {
        g_scroll_state = ScrollState::Waiting;
        g_wait_frames_remaining = WAIT_DURATION_FRAMES;
        return;
    }

    // Move to the next event (don't reset to 0)
    if (g_current_event_index < get_events().size()) {
        cairo_text_extents_t ext;
        cairo_text_extents(g_cr, get_events()[g_current_event_index].c_str(), &ext);
        g_text_width = (float)ext.width;
        g_scroll_x = (float)OVERLAY_W;
        g_current_event_index++;
    }
}

// ============================================================================
// UPDATE - Called every frame (~10fps)
// ============================================================================
bool scroll_update() {
    if (g_scroll_state == ScrollState::Scrolling) {
        // Move scroll position
        g_scroll_x -= SCROLL_SPEED_PX;

        // Check if event is fully off-screen
        if (g_scroll_x + g_text_width < 0) {
            next_event();
        }

        // Check if all events completed
        if (g_current_event_index >= get_events().size()) {
            g_scroll_state = ScrollState::Waiting;
            g_wait_frames_remaining = WAIT_DURATION_FRAMES;
        }
    }
    else if (g_scroll_state == ScrollState::Waiting) {
        g_wait_frames_remaining--;

        if (g_wait_frames_remaining <= 0) {
            // Wait complete, start scrolling with new random events
            scroll_refresh_events();
            g_scroll_state = ScrollState::Scrolling;
            g_scroll_x = (float)OVERLAY_W;
        }
    }

    // Return true if there's text currently being displayed
    return (g_scroll_state == ScrollState::Scrolling) || !get_events().empty();
}

// ============================================================================
// DRAW - Called every frame
// ============================================================================
void scroll_draw() {
    // Draw green background bar
    cairo_set_source_rgba(g_cr, 0.0f, 1.0f, 0.0f, 1.0f);  // Green
    cairo_rectangle(g_cr,
        0,
        OVERLAY_H - BOTTOM_BAR_H,
        OVERLAY_W,
        BOTTOM_BAR_H);
    cairo_fill(g_cr);

    // Don't draw text while waiting
    if (g_scroll_state == ScrollState::Waiting || get_events().empty()) {
        return;
    }

    // Draw scrolling text
    cairo_set_font_size(g_cr, TICKER_FONT_SZ);
    cairo_set_source_rgba(g_cr, 0.0f, 0.0f, 0.0f, 1.0f);  // Black text
    cairo_move_to(g_cr, g_scroll_x, (float)g_text_baseline);
    cairo_show_text(g_cr, get_events()[g_current_event_index > 0 ? g_current_event_index - 1 : 0].c_str());
}

// ============================================================================
// EVENT REFRESH - Called every minute
// ============================================================================
void scroll_refresh_events() {
    get_events() = get_random_events(10);

    if (!get_events().empty()) {
        g_current_event_index = 0;

        cairo_text_extents_t ext;
        cairo_text_extents(g_cr, get_events()[0].c_str(), &ext);
        g_text_width = (float)ext.width;

        g_text_baseline = (int)(OVERLAY_H - BOTTOM_BAR_H + TICKER_FONT_SZ * 1);

        g_scroll_x = (float)OVERLAY_W;
    }
}

// ============================================================================
// SHUTDOWN
// ============================================================================
void scroll_shutdown() {
    g_scroll_state = ScrollState::Initialized;
    g_scroll_x = 0.0f;
    g_wait_frames_remaining = 0;
    get_events().clear();
    g_current_event_index = 0;
    g_text_width = 0.0f;
}
