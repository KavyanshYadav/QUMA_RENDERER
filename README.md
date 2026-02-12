# QumaRenderer

QumaRenderer is a modular C++20 engine workspace organized into core subsystems under `engine/` (core, platform, render, modules, devtools), with optional tests and sample bundle targets.

## Prerequisites

- CMake 3.23+
- A C++20 compiler (GCC/Clang/MSVC)
- Ninja (recommended; used by the provided CMake presets)

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

This repository currently builds engine libraries (and optional bundle/interface targets), not a standalone runtime executable yet.

You can still run the project validation flow after building:

```bash
ctest --test-dir build
```

If/when runnable app/sample executables are added, run them from the build output directory (typically under `build/<preset>/bin/`).

## Install

```bash
cmake --install build
```
