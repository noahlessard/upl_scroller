# UPL Scroller

## Project Structure

This is a C++ project that uses CMake for building. The project structure is organized as follows:

```
upl_scroller/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # Project documentation
├── build/                  # Build output directory (created during compilation)
├── fonts/                  # Font files directory
│   └── pix.ttf             # Font file used by the application
├── src/                    # Source code directory
│   ├── mainLoop.cpp        # Main loop implementation
│   ├── mainLoop.h          # Main loop header
│   ├── scroll.cpp          # Scrolling functionality implementation
│   └── scroll.h            # Scrolling functionality header
├── startvlc.sh             # Shell script to start VLC media player
├── fonts.sh                # Shell script to handle font setup
└── bing.mp3                # Audio file used by the application
```

## Build System

The project uses CMake as its build system. The `CMakeLists.txt` file defines the build configuration, including compiler settings, dependencies, and the target executable.

## Setup on Raspberry Pi (Raspbian)

```bash
sudo apt-get update
sudo apt-get install cmake g++ libnotcurses-dev

cmake -B build
make -C build
./upl_scroller
```

## Logging

 To enable logging when needed, compile with:

 ```bash
   cmake -DCMAKE_CXX_FLAGS="-DENABLE_LOGGING=1" .
   make
 ```