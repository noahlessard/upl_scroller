#pragma once
#include <vector>
#include <cairo/cairo.h>

// Bounce Image constants
constexpr int MAX_BOUNCE_WIDTH = 120;
constexpr int MAX_BOUNCE_HEIGHT = 120;
constexpr int LOGO_SIZE = 300;

// Initialize bouncing animation (can be disabled)
void bounce_init(bool enabled);

// Scan /static folder for images and return full paths
void bounce_scan_images(const char* folder);

// Load a random image from the scanned list
void bounce_load_random_image();

// Update bouncing position, returns true if active
bool bounce_update();

// Get current position (caller must draw)
void bounce_get_position(int* x, int* y, int* w, int* h);

// Draw the bouncing image onto the overlay surface
void bounce_draw();

// Free image surface and reset state
void bounce_shutdown();

// Load the static UPL logo (call once after bounce_init)
void logo_init(const char* path);

// Draw the UPL logo in the top-left corner (call each frame after bounce_draw)
void logo_draw();
