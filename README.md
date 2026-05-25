# ARTSCII 🎬

> Real-time terminal ASCII art video player with multiple rendering strategies and live parameter editing.

![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![OpenCV](https://img.shields.io/badge/OpenCV-4.x-green)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20WSL-lightgrey)

---

## Overview

ARTSCII converts video files into ASCII art and plays them directly in your terminal — in real time. It uses a multi-threaded producer/consumer pipeline to decode frames in the background while the main thread renders them, ensuring smooth playback. All rendering parameters can be adjusted live with the keyboard while the video is playing.

---

## Features

- Real-time ASCII video playback in terminal
- Multi-threaded frame decoding (producer/consumer queue)
- Multiple rendering strategies selectable at runtime
- Live parameter tuning while video is playing (no restart needed)
- Optional per-character ANSI true color (24-bit RGB)
- Aspect ratio correction for terminal font proportions
- Dynamic terminal resize detection
- Interactive TUI for settings and file browsing (FTXUI)

---

## Requirements

| Dependency | Version  |
|------------|----------|
| C++ standard | C++17 |
| [OpenCV](https://opencv.org/) | 4.x |
| [FTXUI](https://github.com/ArthurSonzogni/FTXUI) | any recent |
| CMake | 3.15+ |

---

## Build

```bash
git clone https://github.com/yourusername/artscii.git
cd artscii
mkdir build && cd build
cmake ..
make
```

---

## Usage

```bash
./artscii
```

On first run you will be taken to a file browser to select a video file. The path is saved to `settings.conf` for subsequent runs. From the Settings screen you can also choose the rendering strategy and target FPS.

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

All strategies share a common `IRenderStrategy` interface. They receive a raw `cv::Mat` frame and write into a `std::vector<Pixel>` buffer (symbol + RGB color per character).

### Grayscale strategies

All grayscale strategies inherit from `BaseGrayscaleStrategy`, which handles resizing, pixel iteration and ASCII character lookup. They differ only in how they compute a single brightness value from R, G, B channels.

| Strategy | Method |
|----------|--------|
| **Naive Grayscale** | `(R + G + B) / 3` — simple average |
| **Perceptual Grayscale** | `0.299·R + 0.587·G + 0.114·B` — weighted for human eye sensitivity |
| **Lightness Grayscale** | `(min(R,G,B) + max(R,G,B)) / 2` — HSL lightness |
| **Value Grayscale** | `max(R, G, B)` — HSV value channel |

The brightness value is mapped onto the character ramp:
```
 .:-=+*#%@
```

### Edge detection strategies

Edge strategies inherit from `BaseEdgeDetectionStrategy`, which provides Gaussian blur and Sobel gradient computation. They all use these shared parameters:

| Parameter | Description |
|-----------|-------------|
| **Kernel Size** | Gaussian blur kernel (odd, 3–11) |
| **Sobel Boost** | Multiplier applied to gradient magnitude |

#### Sobel Edge Detection

Computes gradient magnitude and angle using a 3×3 Sobel operator. Pixels above a magnitude threshold are drawn as directional ASCII characters (`|`, `-`, `/`, `\`), the rest as spaces.

#### Canny Edge Detection (`AdvancedEdgeDetectionStrategy`)

Extends Sobel with Non-Maximum Suppression (NMS) to thin edges to single-pixel width. Optionally adds **Hysteresis thresholding** (enabled as a live toggle), which distinguishes strong edges from weak ones and only keeps weak edges that connect to strong ones.

Additional parameters when Hysteresis is ON:

| Parameter | Description |
|-----------|-------------|
| **Hysteresis Low** | Lower threshold for weak edge candidates |
| **Hysteresis High** | Upper threshold for strong edges |

#### Comic / Edge+Grayscale

Combines edge detection with grayscale shading. Edges are drawn with directional characters. For each edge pixel, neighboring pixels in the edge direction are checked — if enough of them share the same direction (configurable sensitivity), the character is kept; otherwise a `+` is used for corners and noise. Non-edge areas are filled with brightness-mapped shading characters.

Additional parameters:

| Parameter | Description |
|-----------|-------------|
| **Search Radius** | How many neighboring pixels to check for edge continuity |
| **Sensitivity** | Minimum ratio of matching neighbors to draw a clean line |

---

## Architecture

```
main()
 └── Tui                        — FTXUI menus, file browser, settings screen
      └── AsciiEngine
           ├── frameProducerTask()   — background thread: reads frames → queue
           ├── fetchFrameFromQueue() — main thread: pops frame
           ├── processFrameToBuffer()— calls active strategy's render()
           ├── renderBuffer()        — writes Pixel buffer to stdout (with optional ANSI color)
           ├── renderHUD()           — draws live parameter bar at bottom
           └── checkUserInput()      — non-blocking raw keyboard input

IRenderStrategy  (interface)
 ├── BaseGrayscaleStrategy
 │    ├── NaiveGrayscaleStrategy
 │    ├── PerceptualGrayscaleStrategy
 │    ├── LightnessGrayscaleStrategy
 │    └── ValueGrayscaleStrategy
 └── BaseEdgeDetectionStrategy
      └── AdvancedEdgeDetectionStrategy
           ├── CannyEdgeDetectionStrategy
           └── ComicEdgeDetectionStrategy
```

### Key design patterns

**Strategy pattern** — `IRenderStrategy` defines a single `render()` method. `StrategiesFactory` instantiates the correct subclass at runtime. `AsciiEngine` only ever talks to the interface, with no knowledge of which strategy is active.

**Producer/Consumer with bounded queue** — A background thread continuously decodes video frames into a `std::queue<cv::Mat>` (max size 30). The main render loop pops from the queue. Synchronization uses `std::mutex` + two `std::condition_variable`s (`m_frameReady`, `m_queueNotFull`).

**Template Method pattern** — `BaseGrayscaleStrategy::render()` defines the full pixel loop. Subclasses only override `calculateBrightness()`. Similarly, `BaseEdgeDetectionStrategy::generateBaseEdgeData()` provides blur + Sobel, and `AdvancedEdgeDetectionStrategy::render()` provides NMS + Hysteresis while delegating the character decision to `determinePixelChar()` in subclasses.

**Dynamic property system** — Each strategy exposes a `std::vector<Property>` via `getProperties()`. The HUD displays these live and `setProperty()` applies changes mid-playback. Properties can appear/disappear dynamically (e.g. Hysteresis Low/High only show when Hysteresis is ON).

### Frame buffer

The buffer is a `std::vector<ImageUtils::Pixel>` of size `width × height`. Each `Pixel` holds a `char symbol` and a `cv::Vec3b color`. `renderBuffer()` writes the buffer to stdout row by row, optionally wrapping each character in ANSI 24-bit color escape sequences (`\x1b[38;2;R;G;Bm`).

---

## Configuration

Settings are persisted in `settings.conf` (key=value format) in the working directory:

| Key | Description |
|-----|-------------|
| `video_path` | Last used video file path |
| `target_fps` | Target playback FPS |
| `render_strategy` | Name of the last used strategy |

---

## Planned / TODO

- [ ] Actual FPS-based frame sync (currently fixed 33 ms sleep)
- [ ] Color support for Edge Detection strategies (currently always black)
- [ ] Background color rendering (ANSI 48;2 sequences)
- [ ] Audio playback sync
- [ ] Windows native terminal support

