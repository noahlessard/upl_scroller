#pragma once
#include <vector>

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

// Update bouncing position, returns true if active
bool bounce_update();

// Get current position (caller must draw)
void bounce_get_position(int* x, int* y, int* w, int* h);

// Draw the bouncing image onto the overlay surface
void bounce_draw();

// Free image surface and reset state
void bounce_shutdown();
