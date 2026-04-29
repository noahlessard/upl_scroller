#pragma once
#include "ScrollEvent.h"

// Initialize scrolling subsystem (call once at startup)
void scroll_init();

// Update scroll animation state (call every frame)
// Returns true if text is currently scrolling
bool scroll_update();

// Draw the scrolling text at the bottom (call after bounce_draw)
void scroll_draw();

// Shutdown scrolling subsystem (cleanup at program exit)
void scroll_shutdown();

// Refresh scroll events to new random set
void scroll_refresh_events();
