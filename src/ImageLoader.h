#pragma once
#include <cairo/cairo.h>

// Load a JPEG file, scale to fit within max_w/max_h, preserve aspect ratio
// Returns cairo_surface* (caller owns, destroy when done)
// Returns nullptr on failure
cairo_surface_t* image_load_jpeg(const char* path, int max_w, int max_h);

// Create a placeholder green square surface
cairo_surface_t* image_create_placeholder(int w, int h);
