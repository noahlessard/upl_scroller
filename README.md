# UPL Scroller (Train Cam)

This is the codebase that plays the UPL train cam, a real life TV that plays train footage and audio 24/7 in the UPL at UW Madison. The bash script controls startup and shutdown, as well as audio, while the C++ executable controls overlay graphics. This is done with IPC between MPV and a shared memory Cairo bitmap. It also uses cross compilation docker containers to test compiling on my 64 bit machine before deploying to the ARM raspberry pi 3 board, and run static analysis as well.

Some features:
* UPL train logo and uptime counter in the top left
* Newswire events scrolls across the bottom
* Randomly chosen cat images bounce around like a DVD screensaver
* A background thread polls claud's status api, if its not operational, an alert will show
* Train audio will play in the background, ramping up from silent to a quiet background ambience, then ramp back down

All of this combines for a rich, train based multimedia experience in the UPL, which is needed of course since the new UPL is further from the train tracks that run through Madison.


## How It Works

The rendering pipeline works as follows:
1. `font_init()` loads the TTF file using FreeType and creates a Cairo font face
2. We load the font for text that gets drawn onto a Cairo surface, along with other graphics like images
3. This is stored as a shared memory bitmap, an 800x600 BGRA pixel buffer in tmpfs
4. MPV plays the train video, exposing a JSON-RPC protocol over a unix domain socket (/tmp/mpvsock)
5. We run the overlay-add command to add our bitmap on top of each frame, before it is composited and rendered

This approach allows dynamic graphics to be drawn on top of video without re-encoding, making it ideal for low end hardware, like the older Raspberry Pi SBC.

The audio feature works by grepping the wpctl output, grabbing the node and sink for HDMI audio output, then setting that as the default and controlling the volume through it. Using Pipewire allows for easy control of audio levels and consistent output, and it is installed by default on Raspberry Pi's Debian OS.

## Project Structure

This is a C++ project that uses CMake for building. The project structure is organized as follows:

```
upl_scroller/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # Project documentation
├── build/                  # Build output directory (created during compilation)
├── pix.ttf                 # Font file used by the application
├── faucet.jpg              # Test image for bouncing animation
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
│   ├── claude_status_monitor.cpp  # Polls claude's status page
│   ├── claude_status_monitor.h    # Polls claude's status page
│   ├── ScrollEvent.cpp            # Scroll event types
│   └── ScrollEvent.h              # Scroll event types
├── startvlc.sh                      # Shell script to start mpv and the overlay app
└── bing.mp3                         # Audio file used by the application
```

## Build System

### Setup on Raspberry Pi (Raspbian)

```bash
sudo apt-get update
sudo apt-get install cmake g++ libcairo2-dev libfreetype6-dev libjpeg-dev libcurl4-openssl-dev

cmake -B build
make -C build
./upl_scroller
```

### Docker Cross-Compilation (ARM)

Build an ARM binary inside a container:

```bash
docker-compose run --build --rm --remove-orphans cross-compile 2>&1
```

Build progress (stdout) is suppressed. Compiler warnings and errors (stderr) are captured and printed between markers:

```
=== BUILD OUTPUT START ===
/work/src/foo.cpp:42:10: error: 'libcurl' not found
=== BUILD OUTPUT END ===
Build FAILED (exit code 1)
```

If the build succeeds and there are no warnings, the section between the markers will be empty. The exit code is non-zero on failure.

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