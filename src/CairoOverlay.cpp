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


