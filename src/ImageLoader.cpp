#include "ImageLoader.h"
#include "Logging.h"
#include <cairo/cairo.h>
#include <stddef.h>
#include <cstdio>
#include <cstdlib>
#include <jpeglib.h>

cairo_surface_t* image_load_jpeg(const char* path, int max_w, int max_h) {
    LOG("loading JPEG from %s", path);
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        LOG("failed to open %s", path);
        return nullptr;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    int src_w = (int)cinfo.output_width;
    int src_h = (int)cinfo.output_height;

    // Scale down to fit within max_w/max_h, preserving aspect ratio
    int width = max_w;
    int height = max_h;
    if (src_w > max_w || src_h > max_h) {
        float scale = (float)max_w / src_w;
        if (scale * src_h > max_h)
            scale = (float)max_h / src_h;
        width  = (int)(src_w  * scale);
        height = (int)(src_h * scale);
    }

    LOG("loaded JPEG %dx%d -> scaled to %dx%d", src_w, src_h, width, height);

    // Create cairo surface with the scaled size
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    int surf_stride = cairo_image_surface_get_stride(surf);
    unsigned char* surf_pixels = cairo_image_surface_get_data(surf);

    // Row buffer must hold a full-width source scanline
    unsigned char* row = (unsigned char*)malloc(src_w * 3);
    while (cinfo.output_scanline < (unsigned int)src_h) {
        jpeg_read_scanlines(&cinfo, &row, 1);
        int src_y = (int)cinfo.output_scanline - 1;
        // Cairo ARGB32 is top-down like JPEG — no vertical flip needed
        int dst_y = (int)((float)src_y / src_h * height);
        if (dst_y >= 0 && dst_y < height) {
            for (int x = 0; x < width; x++) {
                int src_x = (int)((float)x / width * src_w);
                unsigned char* src_pixel = row + src_x * 3;
                unsigned char* dst_pixel = surf_pixels + dst_y * surf_stride + x * 4;
                // RGB to BGRA (ARGB32 LE = B G R A)
                dst_pixel[0] = src_pixel[2];  // B
                dst_pixel[1] = src_pixel[1];  // G
                dst_pixel[2] = src_pixel[0];  // R
                dst_pixel[3] = 0xFF;          // A
            }
        }
    }

    free(row);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    cairo_surface_mark_dirty(surf);
    return surf;
}

cairo_surface_t* image_create_placeholder(int w, int h) {
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cairo_t* cr = cairo_create(surf);
    cairo_set_source_rgba(cr, 0, 1, 0, 1);  // green
    cairo_paint(cr);
    cairo_destroy(cr);
    return surf;
}
