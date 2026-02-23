#pragma once
#include <notcurses/notcurses.h>
#include <vector>
#include <cstdio>
#include <string>
#include <string_view>

constexpr unsigned BIG_FONT_ROW_SIZE = 20;
constexpr unsigned BIG_FONT_COLS_SIZE = 22;
constexpr int BIG_FONT_SPACING = 10;
constexpr double SPEED = 0.2;

enum ScrollDirection { SCROLL_LEFT, SCROLL_RIGHT };

struct ScrollEntry {
    std::string text;
    int y;
    float speed;
    ScrollDirection dir;
    std::vector<int> positions;
    float accumulator;
    bool done;
    std::vector<ncplane*> planes;
    uint32_t fg_color;
    uint32_t bg_color;
};
