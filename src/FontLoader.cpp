#include "FontLoader.h"
#include "mainLoop.h"
#include "Logging.h"

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library          g_ft_lib   = nullptr;
static FT_Face             g_ft_face  = nullptr;
static cairo_font_face_t*  g_font     = nullptr;

bool font_init(const char* font_path) {
    if (FT_Init_FreeType(&g_ft_lib)) {
        LOG("FT_Init_FreeType failed");
        return false;
    }
    if (FT_New_Face(g_ft_lib, font_path, 0, &g_ft_face)) {
        LOG("FT_New_Face failed for %s", font_path);
        FT_Done_FreeType(g_ft_lib);
        return false;
    }

    g_font = cairo_ft_font_face_create_for_ft_face(g_ft_face, 0);
    cairo_set_font_face(g_cr, g_font);

    // Pixel fonts look best without anti-aliasing
    cairo_font_options_t* opts = cairo_font_options_create();
    cairo_font_options_set_antialias(opts, CAIRO_ANTIALIAS_NONE);
    cairo_font_options_set_hint_style(opts, CAIRO_HINT_STYLE_FULL);
    cairo_set_font_options(g_cr, opts);
    cairo_font_options_destroy(opts);

    cairo_set_font_size(g_cr, LABEL_FONT_SZ);
    LOG("font init OK (%s)", font_path);
    return true;
}

void font_set_size(double pt_size) {
    cairo_set_font_size(g_cr, pt_size);
}

void font_shutdown() {
    if (g_font) {
        cairo_font_face_destroy(g_font);
        g_font = nullptr;
    }
    if (g_ft_face) {
        FT_Done_Face(g_ft_face);
        g_ft_face = nullptr;
    }
    if (g_ft_lib) {
        FT_Done_FreeType(g_ft_lib);
        g_ft_lib = nullptr;
    }
}
