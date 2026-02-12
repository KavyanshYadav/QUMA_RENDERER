# QumaRenderer

QumaRenderer is a modular C++20 engine workspace organized into core subsystems under `engine/` (core, platform, render, modules, devtools), with optional tests and sample bundle targets.

## Prerequisites

- CMake 3.23+
- A C++20 compiler (GCC/Clang/MSVC)
- Ninja (recommended; used by the provided CMake presets)
- Preferred: place GLAD under `third_party/glad` and it will be linked automatically.
- Optional: Windows can auto-fetch GLAD when enabled (`ENGINE_AUTO_FETCH_GLAD=ON`; default `OFF`).

## Build (manual CMake commands)

```bash
cmake -S . -B build
cmake --build build
```

## Build (recommended preset flow)

Linux GCC debug:

```bash
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
```

Linux Clang debug:

```bash
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
```

## Run

This repository now includes a runnable sample executable:

- `engine_sample_opengl_triangle`: validates platform + module + OpenGL render path by opening a window and drawing a triangle.

Run it from the build output directory:

```bash
./build/linux-gcc-debug/bin/engine_sample_opengl_triangle --frames=600
```

For headless/CI smoke checks, run only a few frames:

```bash
./build/linux-gcc-debug/bin/engine_sample_opengl_triangle --frames=1
```

You can still run the project validation flow after building:

```bash
ctest --test-dir build
```

If/when runnable app/sample executables are added, run them from the build output directory (typically under `build/<preset>/bin/`).


## Feature flags

- `ENGINE_ENABLE_DEVTOOLS` toggles the `engine/devtools/imgui_tools` package and can be set `OFF` for production builds.
- `ENGINE_ENABLE_IMGUI` keeps ImGui integration compile definitions available to downstream consumers.
- `ENGINE_AUTO_FETCH_SDL2` auto-downloads/builds SDL2 from source when SDL2 is missing locally.
- `ENGINE_AUTO_FETCH_IMGUI` auto-downloads/builds ImGui from source when devtools/ImGui support is enabled.

## Install

```bash
cmake --install build
```
