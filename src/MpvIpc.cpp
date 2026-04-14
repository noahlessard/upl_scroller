#include "MpvIpc.h"
#include "Logging.h"
#include "mainLoop.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <chrono>

static constexpr const char* MPV_SOCK = "/tmp/mpvsock";

bool mpv_connect() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG("socket() failed: %s", strerror(errno));
        return false;
    }

    struct sockaddr_un addr = {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, MPV_SOCK, sizeof(addr.sun_path) - 1);

    // Retry for up to 15 seconds while MPV is starting
    auto start = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds(15);
    int attempt = 0;

    while (std::chrono::steady_clock::now() - start < timeout) {
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
            // Make non-blocking so drain_mpv_replies() can poll without stalling
            int fl = fcntl(sock, F_GETFL, 0);
            if (fl >= 0) fcntl(sock, F_SETFL, fl | O_NONBLOCK);
            LOG("connected to mpv at %s after %d attempts", MPV_SOCK, attempt + 1);
            g_mpv_sock = sock;
            return true;
        }
        if (attempt == 0 || attempt == 29) {
            LOG("connect(%s) attempt %d failed: %s", MPV_SOCK, attempt + 1, strerror(errno));
        }
        ++attempt;
        usleep(500000);  // 500ms
    }

    close(sock);
    return false;
}

void mpv_present_overlay() {
    if (g_mpv_sock < 0) {
        LOG("present_overlay: no mpv socket, skipping");
        return;
    }

    int stride = OVERLAY_W * 4;
    char cmd[512];
    int n = snprintf(cmd, sizeof(cmd),
        "{\"command\":[\"overlay-add\",1,%d,%d,\"%s\",0,\"bgra\",%d,%d,%d]}\n",
        OVERLAY_X, OVERLAY_Y, "/dev/shm/overlay.bgra", OVERLAY_W, OVERLAY_H, stride);

    if (write(g_mpv_sock, cmd, (size_t)n) < 0) {
        LOG("write(mpv) failed: %s - reconnecting", strerror(errno));
        close(g_mpv_sock);
        g_mpv_sock = -1;
        mpv_connect();
        return;
    }

    drain_mpv_replies();
}

void mpv_query_props() {
    if (g_mpv_sock < 0) return;
    const char* cmds[] = {
        "{\"command\":[\"get_property\",\"mpv-version\"],\"request_id\":100}\n",
        "{\"command\":[\"get_property\",\"width\"],\"request_id\":101}\n",
        "{\"command\":[\"get_property\",\"height\"],\"request_id\":102}\n",
        "{\"command\":[\"get_property\",\"vo-configured\"],\"request_id\":103}\n",
        "{\"command\":[\"get_property\",\"video-out-params\"],\"request_id\":104}\n",
        "{\"command\":[\"get_property\",\"osd-width\"],\"request_id\":105}\n",
        "{\"command\":[\"get_property\",\"osd-height\"],\"request_id\":106}\n",
    };
    for (auto c : cmds) write(g_mpv_sock, c, strlen(c));
    usleep(400000);   // 400 ms - give mpv time to reply
    drain_mpv_replies();
}

int mpv_get_socket() {
    return g_mpv_sock;
}

void drain_mpv_replies() {
    if (g_mpv_sock < 0) return;
    char buf[1024];
    for (;;) {
        ssize_t n = read(g_mpv_sock, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r')) {
                buf[--n] = '\0';
            }
            LOG("mpv -> %s", buf);
        } else if (n == 0) {
            LOG("mpv socket closed by peer");
            close(g_mpv_sock);
            g_mpv_sock = -1;
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            LOG("read(mpv) error: %s", strerror(errno));
            return;
        }
    }
}

void mpv_shutdown() {
    if (g_mpv_sock >= 0) {
        close(g_mpv_sock);
        g_mpv_sock = -1;
    }
}
