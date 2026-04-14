#pragma once

// Connect to mpv IPC server, returns true on success
bool mpv_connect();

// Send overlay-add command with current frame
void mpv_present_overlay();

// Query mpv properties (writes responses to log)
void mpv_query_props();

// Read and discard all pending replies from mpv socket
void drain_mpv_replies();

// Get current mpv socket fd (-1 if not connected)
int mpv_get_socket();

// Shutdown mpv connection
void mpv_shutdown();
