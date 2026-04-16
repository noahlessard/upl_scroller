#pragma once
#include <vector>
#include <cairo/cairo.h>

// Bounce Image constants
constexpr int MAX_BOUNCE_WIDTH = 120;
constexpr int MAX_BOUNCE_HEIGHT = 120;

// Initialize bouncing animation (can be disabled)
void bounce_init(bool enabled);

// Load image for bouncing (optional)
void bounce_load_image(const char* path);

// Scan /static folder for images and return full paths
void bounce_scan_images(const char* folder);

// Get list of scanned image paths
const std::vector<const char*>& bounce_get_images();

// Load a random image from the scanned list
void bounce_load_random_image();

// Get current loaded surface (for use in mainLoop test phase)
cairo_surface_t* bounce_get_current_surface();

// Get number of scanned images
size_t bounce_get_image_count();

// Clear all scanned image paths
void bounce_clear_image_paths();

// Update bouncing position, returns true if active
bool bounce_update();

// Get current position (caller must draw)
void bounce_get_position(int* x, int* y, int* w, int* h);

// Draw the bouncing image onto the overlay surface
void bounce_draw();

// Free image surface and reset state
void bounce_shutdown();
