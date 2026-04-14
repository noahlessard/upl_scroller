#pragma once
#include <string_view>

// Clear the overlay surface to fully transparent
void clear_to_transparent();

// Redraw border without bottom label (e.g., for ticker)
void cairo_draw_border(bool show_bottom_label);

// Create alert box with title and body (blocks for duration_s)
void cairo_create_alert(std::string_view title, std::string_view body, float duration_s);
