#pragma once
#include <string>
#include <string_view>
#include <vector>

enum class Dir { Left, Right };

struct ScrollText {
    std::string text;
    float       x;       // current left-edge pixel position within the overlay
    int         y;       // text baseline pixel position within the overlay
    float       speed;   // pixels per frame
    Dir         dir;
    bool        done;
    float       text_w;  // cached rendered width (pixels)

    // y_px   : baseline Y in the overlay (e.g. OVERLAY_H - 12 for the bottom bar)
    // speed_px: pixels to advance per frame
    ScrollText(std::string_view text, int y_px, float speed_px, Dir dir);
};

// Run scrolling text with given scrolls (blocks until done)
void scroll_run(std::vector<ScrollText>& scrolls);
