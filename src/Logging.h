#pragma once

// ── Logging Toggle ────────────────────────────────────────────────────────────
// Set to 1 to enable logging, 0 to disable all log output
// Default: 0 (disabled)
#ifndef ENABLE_LOGGING
#define ENABLE_LOGGING 0
#endif

#if ENABLE_LOGGING
#define LOG(...) logging_msg(__VA_ARGS__)
#else
#define LOG(...) (void)0
#endif

void logging_init();
void logging_shutdown();
void logging_msg(const char* fmt, ...);
