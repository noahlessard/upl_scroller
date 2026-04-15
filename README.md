# UPL Scroller

## How It Works

This app generates a video overlay by combining MPV media playback with real-time text rendering. The overlay is created using:

- **MPV**: A media player that provides an IPC (inter-process communication) interface for controlling playback and rendering a video frame buffer
- **FreeType**: A font engine that loads the TrueType font (`pix.ttf`) and provides glyph rendering capabilities
- **Cairo**: A 2D graphics library that draws text onto an offscreen surface, which is then composited as an overlay on top of the MPV video

The rendering pipeline works as follows:
1. `font_init()` loads the TTF file using FreeType and creates a Cairo font face
2. Text is drawn onto a Cairo surface with anti-aliasing disabled for crisp, pixel-perfect characters
3. The Cairo surface (containing the rendered text) is uploaded as a texture overlay onto the MPV video frame
4. This composite (video + text overlay) is displayed in the MPV window

This approach allows dynamic text updates without re-encoding video, making it ideal for scrolling credits, live captions, or other overlay content.

## Project Structure

This is a C++ project that uses CMake for building. The project structure is organized as follows:

```
upl_scroller/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # Project documentation
├── build/                  # Build output directory (created during compilation)
├── pix.ttf                 # Font file used by the application
├── soggy.jpg               # Test image for bouncing animation
├── src/                    # Source code directory
│   ├── mainLoop.cpp        # Main loop implementation
│   ├── mainLoop.h          # Main loop header
│   ├── scroll.cpp          # Scrolling functionality implementation
│   ├── scroll.h            # Scrolling functionality header
│   ├── Bounce.cpp          # Bouncing image animation
│   ├── Bounce.h            # Bouncing image animation
│   ├── CairoOverlay.cpp    # Cairo rendering for overlay
│   ├── CairoOverlay.h      # Cairo rendering for overlay
│   ├── FontLoader.cpp      # Font loading and initialization
│   ├── FontLoader.h        # Font loading and initialization
│   ├── ImageLoader.cpp     # JPEG image loading
│   ├── ImageLoader.h                # JPEG image loading
│   ├── Logging.cpp                  # Logging subsystem
│   ├── Logging.h                    # Logging subsystem
│   ├── MpvIpc.cpp                   # MPV IPC communication
│   ├── MpvIpc.h                     # MPV IPC communication
|   ├── claude_status_monitor.cpp    # Polls claude's status page
|   └── claude_status_monitor.h      # Polls claude's status page
├── startvlc.sh                      # Shell script to start VLC media player
└── bing.mp3                         # Audio file used by the application
```

## Build System

### Setup on Raspberry Pi (Raspbian)

```bash
sudo apt-get update
sudo apt-get install cmake g++ libnotcurses-dev jpeglib-dev

cmake -B build
make -C build
./upl_scroller
```

### Docker Cross-Compilation (ARM)

Build an ARM binary inside a container:

```bash
docker-compose run --rm --remove-orphans cross-compile 2>&1
```

Build progress is suppressed. Output is bounded by markers:

```
=== BUILD OUTPUT START ===
=== BUILD OUTPUT END ===
Build succeeded. ARM binary: build-arm/upl_scroller
```

If the build fails, the exit code is non-zero and the failure is noted between the markers. Compiler warnings/errors appear on stderr and are visible between the markers via the `2>&1` redirect above.

### Static Analysis (clang-tidy)

Run clang-tidy with comprehensive checks across bugprone, modernize, performance, readability, cppcoreguidelines, and clang-analyzer categories:

```bash
docker-compose build static-analysis
docker-compose run --rm static-analysis 2>&1
```

Build the image first (once) to avoid mixing docker build output with analysis results. Output is bounded by markers — everything between them is the analysis:

```
=== CLANG-TIDY RESULTS START ===
/work/src/foo.cpp:42:5: warning: ... [bugprone-some-check]
=== CLANG-TIDY RESULTS END ===
```

No output between the markers means no issues found. To adjust which check categories run, edit the `-checks=` flag in `docker-compose.yml`. The `*` wildcard enables all checks in a category; prefix a category with `-` to disable it (e.g., `-modernize-use-trailing-return-type`).

## Logging

 To enable logging when needed, compile with:

 ```bash
   cmake -DCMAKE_CXX_FLAGS="-DENABLE_LOGGING=1" .
   make
 ```