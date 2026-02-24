#pragma once
#include <string_view>
constexpr unsigned BIG_FONT_ROW_SIZE  = 20;
constexpr unsigned BIG_FONT_COLS_SIZE = 22;
constexpr int      BIG_FONT_SPACING   = 10;
constexpr double   SPEED              = 0.1;

static void run_reel(std::string_view text, float speed);
static void returnToNormalBoarder(bool textAtBottom);