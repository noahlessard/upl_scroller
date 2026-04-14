#pragma once

// Initialize Cairo font from FreeType
// font_path: path to TTF file
// Returns true on success
bool font_init(const char* font_path);

// Set font size
void font_set_size(double pt_size);

// Shutdown font (cleanup FreeType resources)
void font_shutdown();
