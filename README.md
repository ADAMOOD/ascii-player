# ARTSCII 🎬

> Real-time terminal ASCII art video player with multiple rendering strategies, live parameter editing, and optional true color output.

![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![OpenCV](https://img.shields.io/badge/OpenCV-4.x-green)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-lightgrey)
![FTXUI](https://img.shields.io/badge/TUI-FTXUI-orange)

---

## Overview

ARTSCII converts video files or a live webcam feed into ASCII art and plays them directly in your terminal — in real time. It uses a multi-threaded producer/consumer pipeline to decode frames in the background while the main thread renders them, ensuring smooth playback. All rendering parameters can be adjusted live with the keyboard while the video is playing.

---

## Features

- Real-time ASCII video playback in terminal (video files and webcam)
- Multi-threaded frame decoding (producer/consumer queue)
- 7 rendering strategies selectable at runtime
- Live parameter tuning while video is playing (no restart needed)
- Optional per-character ANSI true color (24-bit RGB) or 8-bit (256-color) mode
- Color tolerance parameter to batch nearby colors and reduce escape sequence spam
- Aspect ratio correction for terminal font proportions
- Dynamic terminal resize detection
- Interactive TUI for settings and file browsing (FTXUI)
- Cross-platform: Linux, WSL2, and Windows (Windows Terminal required)

---

## Requirements

| Dependency | Version | Notes |
|------------|---------|-------|
| C++ standard | C++17 | |
| [OpenCV](https://opencv.org/) | 4.x | |
| [FTXUI](https://github.com/ArthurSonzogni/FTXUI) | any recent | Auto-downloaded on Linux via CMake FetchContent; install via vcpkg on Windows |
| CMake | 3.14+ | |

### Windows — additional requirements

- **Windows Terminal** (not classic CMD or PowerShell) — required for ANSI escape code support
- **vcpkg** — for OpenCV and FTXUI installation:

```powershell
vcpkg install opencv4:x64-windows
vcpkg install ftxui:x64-windows
```

### Linux / WSL2 — additional requirements

```bash
sudo apt install libopencv-dev cmake build-essential
```

> **WSL2 + Webcam:** The default WSL2 kernel does not include the `uvcvideo` driver. You need to attach the camera via `usbipd` and build a custom WSL2 kernel with UVC support enabled. See [Microsoft WSL USB docs](https://learn.microsoft.com/en-us/windows/wsl/connect-usb) for `usbipd` setup, and [this guide](https://github.com/PINTO0309/wsl2_linux_kernel_usbcam_enable_conf) for the custom kernel build. Video file playback works without any extra setup.

---

## Build

### Linux / WSL2

```bash
git clone https://github.com/yourusername/artscii.git
cd artscii
mkdir build && cd build
cmake ..
make
```

### Windows (MSVC + vcpkg)

```powershell
git clone https://github.com/yourusername/artscii.git
cd artscii
mkdir build; cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release
```

---

## Usage

```bash
./artscii          # Linux / WSL2
.\ASCII-PLAYER.exe # Windows (run inside Windows Terminal)
```

On first run you will be taken to a file browser to select a video file. The path is saved to `settings.conf` for subsequent runs. From the Settings screen you can switch to webcam mode, choose a rendering strategy, and set other options.

### Supported video formats

`.mp4`, `.mov`, `.avi`

### Playback controls

| Key | Action |
|-----|--------|
| `A` / `D` | Navigate between parameters in the HUD |
| `W` / `S` | Increase / decrease the selected parameter |
| `Q` | Quit playback |

---

## Rendering Strategies

All strategies share a common `IRenderStrategy` interface. They receive a raw `cv::Mat` frame and write into a `std::vector<Pixel>` buffer (symbol + foreground RGB color + background RGB color per character).

Every strategy inherits the **color properties** from `AbstractRenderStrategy`:

| Property | Type | Description |
|----------|------|-------------|
| **Use Color** | Toggle | Enables per-character ANSI color output |
| **8-bit Colors** | Toggle | Switches from 24-bit RGB to 256-color mode (wider terminal compatibility) |
| **Color Tolerance** | Float | Groups nearby colors into one escape sequence to reduce output size |

---

### Grayscale strategies

All grayscale strategies inherit from `BaseGrayscaleStrategy`, which handles resizing, pixel iteration, color extraction, and ASCII character lookup. Subclasses only define the brightness formula.

The brightness value is mapped onto the character ramp:
```
 .:-=+*#%@
```

| Strategy | Brightness formula |
|----------|--------------------|
| **Naive Grayscale** | `(R + G + B) / 3` — simple average |
| **Perceptual Grayscale** | `0.299·R + 0.587·G + 0.114·B` — weighted for human eye sensitivity (default) |
| **Lightness Grayscale** | `(min(R,G,B) + max(R,G,B)) / 2` — HSL lightness |
| **Value Grayscale** | `max(R, G, B)` — HSV value channel |

---

### Edge detection strategies

Edge strategies inherit from `BaseEdgeDetectionStrategy`, which provides a hand-written Gaussian blur (separable 1D convolution) and a hand-written Sobel gradient computation. All edge strategies share these parameters:

| Property | Description |
|----------|-------------|
| **Kernel Size** | Gaussian blur kernel size (odd, 3–11). Larger = smoother edges |
| **Sobel Boost** | Multiplier applied to raw gradient magnitude |
| **Edge Threshold** | Minimum magnitude to draw a character instead of the fill character |

#### Sobel Edge Detection

Computes gradient magnitude and direction using the 3×3 Sobel operator. Pixels above the threshold are drawn as directional ASCII characters (`|`, `-`, `/`, `\`) based on gradient angle. Non-edge pixels use the fill character configured in Settings. Color from the source pixel is carried through.

#### Canny Edge Detection

Extends Sobel with **Non-Maximum Suppression (NMS)**, which thins edges to a single pixel width by suppressing pixels that are not the local maximum along the gradient direction. Optionally adds **Hysteresis thresholding**, which distinguishes strong edges from weak ones — weak edges are only kept if they are connected to a strong edge.

Additional properties when Hysteresis is enabled:

| Property | Description |
|----------|-------------|
| **Hysteresis** | Toggle ON/OFF |
| **Hysteresis Low** | Lower threshold — candidates below this are discarded |
| **Hysteresis High** | Upper threshold — pixels above this are strong edges |

#### Comic / Edge+Grayscale

Combines edge detection with grayscale shading. Edge pixels use the directional character set. For each edge pixel, the **Smart Edge** algorithm scans neighbors along the gradient direction: if enough of them share the same orientation (above the sensitivity threshold), the directional character is drawn; otherwise a `+` is used for corners and noise. Non-edge areas are filled with brightness-mapped shading characters from the grayscale ramp.

Additional properties:

| Property | Description |
|----------|-------------|
| **Search Radius** | How many pixels to scan in each direction for line continuity |
| **Sensitivity** | Minimum ratio of matching neighbors to draw a clean directional line |

---

## Architecture

```
main()
 └── Tui                           — FTXUI menus, file browser, settings screen
      └── AsciiEngine
           ├── frameProducerTask() — background thread: decodes frames → bounded queue
           ├── fetchFrameFromQueue()— main thread: pops frame (wait_for with timeout)
           ├── processFrameToBuffer()— calls active strategy's render()
           ├── renderBuffer()      — writes Pixel buffer to stdout (optional ANSI color)
           ├── renderHUD()         — live parameter bar at bottom of screen
           └── checkUserInput()    — non-blocking raw keyboard polling

IRenderStrategy           (pure interface)
 └── AbstractRenderStrategy        (adds: Use Color, 8-bit Colors, Color Tolerance)
      ├── BaseGrayscaleStrategy    (adds: resize + ASCII ramp + color extraction)
      │    ├── NaiveGrayscaleStrategy
      │    ├── PerceptualGrayscaleStrategy
      │    ├── LightnessGrayscaleStrategy
      │    └── ValueGrayscaleStrategy
      └── BaseEdgeDetectionStrategy(adds: Gaussian kernel + Sobel computation)
           ├── SobelEdgeDetectionStrategy
           └── AdvancedEdgeDetectionStrategy (adds: NMS + Hysteresis)
                ├── CannyEdgeDetectionStrategy
                └── ComicEdgeDetectionStrategy
```

---

### Design patterns used

**Strategy pattern** — `IRenderStrategy` defines a single `render()` method. `StrategiesFactory` instantiates the correct subclass at runtime based on a name string. `AsciiEngine` talks only to the interface — it has no knowledge of which strategy is active.

**Template Method pattern** — `BaseGrayscaleStrategy::render()` defines the full pixel loop and color extraction. Subclasses only override `calculateBrightness()`. Similarly, `AdvancedEdgeDetectionStrategy::render()` defines the NMS + Hysteresis pipeline and delegates only the final character decision to `determinePixelChar()` in `CannyEdgeDetectionStrategy` and `ComicEdgeDetectionStrategy`.

**Factory pattern** — `StrategiesFactory::createStrategy(name)` centralizes object creation. Adding a new strategy requires only registering it in the factory and the name list — no changes to `AsciiEngine`.

**Producer/Consumer with bounded queue** — A background thread continuously decodes video frames into a `std::queue<cv::Mat>` (max 30 frames). The main render loop pops from the queue. Synchronization uses `std::mutex` + two `std::condition_variable`s (`m_frameReady`, `m_queueNotFull`). `fetchFrameFromQueue()` uses `wait_for` with a 50 ms timeout so the main loop can still process keyboard input when the queue is empty.

**Dynamic property system** — Each strategy exposes a `std::vector<Property>` via `getProperties()`. The HUD renders these at the bottom of the screen. `setProperty()` applies changes mid-playback. Properties can appear and disappear dynamically (e.g., Hysteresis Low/High only appear when Hysteresis is ON). The system is chained: each level calls its parent's `getProperties()` first and appends its own.

---

### Frame buffer

The buffer is a `std::vector<ImageUtils::Pixel>` of size `width × height`, stored in row-major order (`index = y * width + x`). Each `Pixel` holds:

```cpp
struct Pixel {
    char     symbol;   // ASCII character
    cv::Vec3b fgColor; // Foreground text color (BGR)
    cv::Vec3b bgColor; // Background cell color (BGR, reserved for future use)
};
```

`renderBuffer()` assembles the entire frame into a single `std::string` before writing it to stdout in one `flush`, minimizing system call overhead. Color escape sequences are only emitted when the color differs from the previous pixel (controlled by `Color Tolerance`).

---

### Cross-platform notes

| Feature | Linux / WSL2 | Windows |
|---------|-------------|---------|
| Terminal size | `ioctl(TIOCGWINSZ)` | `GetConsoleScreenBufferInfo()` |
| Raw keyboard input | `termios` + `read()` | `_kbhit()` + `_getch()` |
| Webcam backend | `cv::CAP_V4L2` | `cv::CAP_MSMF` |
| ANSI escape codes | Any modern terminal | **Windows Terminal only** |
| FTXUI | Auto via CMake FetchContent | vcpkg |

---

## Configuration

Settings are persisted in `settings.conf` (key=value format) in the working directory:

| Key | Description |
|-----|-------------|
| `video_path` | Last used video file path |
| `use_webcam` | `true` / `false` — whether to use live webcam feed |
| `target_fps` | Target playback FPS (UI only — sync not yet implemented) |
| `render_strategy` | Name of the last used strategy |
| `fill_char` | Fill character for non-edge areas in edge detection strategies |

---

## Planned / TODO

- [ ] Actual FPS-based frame synchronization (currently a fixed 33 ms sleep)
- [ ] Background color rendering (ANSI `48;2;R;G;B` sequences — `bgColor` field already present in `Pixel`)
- [ ] Audio playback sync
- [ ] Enable ANSI Virtual Terminal Processing automatically on Windows (currently requires Windows Terminal)

---

## Documentation

The codebase follows Doxygen documentation conventions. To generate HTML docs:

```bash
doxygen Doxyfile
```

Key documented components: `AsciiEngine`, `IRenderStrategy`, `AbstractRenderStrategy`, `BaseEdgeDetectionStrategy`, `AdvancedEdgeDetectionStrategy`, `ImageUtils`, `Property`, `ConfigManager`.
