#include "Logging.h"

#include <cstdarg>
#include <chrono>
#include <cstdio>

static constexpr const char* LOG_PATH = "/tmp/upl_scroller.log";

static FILE* g_log = nullptr;
static auto  g_start_time = std::chrono::steady_clock::now();

void logging_init() {
#if ENABLE_LOGGING
    g_log = fopen(LOG_PATH, "w");
    if (g_log) setvbuf(g_log, nullptr, _IOLBF, 0);  // line-buffered
    if (g_log) {
        fprintf(g_log, "[upl_scroller log started]\n");
        fflush(g_log);
    }
#endif
}

void logging_shutdown() {
#if ENABLE_LOGGING
    if (g_log) {
        fprintf(g_log, "[upl_scroller log closed]\n");
        fclose(g_log);
        g_log = nullptr;
    }
#endif
}

void logging_msg(const char* fmt, ...) {
#if ENABLE_LOGGING
    if (!g_log) return;

    va_list ap;
    va_start(ap, fmt);

    auto now = std::chrono::steady_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_start_time).count();
    auto secs = (int)(total_ms / 1000);
    auto ms = (int)(total_ms % 1000);

    fprintf(g_log, "[%05d.%03d] ", secs, ms);
    vfprintf(g_log, fmt, ap);
    fputc('\n', g_log);
    fflush(g_log);

    va_end(ap);
#else
    (void)fmt;
#endif
}

