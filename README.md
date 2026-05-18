# Reverb

A stereo reverb audio plugin built with [JUCE](https://juce.com/). Five algorithm profiles shape the same core controls into distinct spaces—from tight rooms to expansive halls—with optional pre-delay, a dedicated spring modulation path, and an ADSR envelope that gates the wet signal from input level.

**Formats:** VST3 · Standalone

---

## Features

|                 |                                                                                                                                   |
| --------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| **Algorithms**  | Room, Hall, Chamber, Plate, Spring — each remaps room size, damping, width, and pre-delay for a distinct character                |
| **Pre-delay**   | Algorithm-specific early reflection delay (up to ~70 ms on Hall)                                                                  |
| **Spring mode** | Modulated feedback delay layered on the wet path to mimic coil resonance                                                          |
| **Wet ADSR**    | Attack, decay, sustain, and release applied only to the reverb tail; triggered automatically when input crosses a level threshold |
| **Mix**         | Independent dry and wet level controls with manual summing (envelope does not affect the dry path)                                |

---

## Requirements

All platforms:

- **CMake** 3.22 or newer
- **Git** (JUCE is fetched automatically at configure time)

<details>
<summary><strong>Linux</strong></summary>

- **Compiler:** GCC or Clang with C++17
- **Packages** (Debian/Ubuntu example):

  ```bash
  sudo apt install build-essential cmake git \
    libasound2-dev libfreetype6-dev libx11-dev libxinerama-dev \
    libxrandr-dev libxcursor-dev libwebkit2gtk-4.1-dev
  ```

  Use your distro’s equivalent package names if you are not on Debian/Ubuntu.

</details>

<details>
<summary><strong>macOS</strong></summary>

- **Xcode** (from the App Store) or **Xcode Command Line Tools**:

  ```bash
  xcode-select --install
  ```

- **CMake** — [cmake.org](https://cmake.org/download/) or Homebrew: `brew install cmake`

</details>

<details>
<summary><strong>Windows</strong></summary>

- **Visual Studio 2022** (or 2019) with the **Desktop development with C++** workload
- **CMake** — installer from [cmake.org](https://cmake.org/download/) or via Visual Studio’s CMake component
- **Git** for Windows — [git-scm.com](https://git-scm.com/download/win)

</details>

---

## Build

After a successful build, the plugin bundle is always named **`Reverb.vst3`**. Paths below use `Release`; use `Debug` if you built a debug configuration.

### Linux

```bash
./build.sh              # Release (default)
./build.sh Debug        # Debug
```

Or manually:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

| Target     | Path |
| ---------- | ---- |
| VST3       | `build/Reverb_artefacts/Release/VST3/Reverb.vst3` |
| Standalone | `build/Reverb_artefacts/Release/Standalone/Reverb` |

### macOS

From the project root in Terminal:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

| Target     | Path |
| ---------- | ---- |
| VST3       | `build/Reverb_artefacts/Release/VST3/Reverb.vst3` |
| Standalone | `build/Reverb_artefacts/Release/Standalone/Reverb.app` |

### Windows

Open **x64 Native Tools Command Prompt for VS 2022** (or a terminal where `cmake` and MSVC are on your `PATH`):

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```

| Target     | Path |
| ---------- | ---- |
| VST3       | `build\Reverb_artefacts\Release\VST3\Reverb.vst3` |
| Standalone | `build\Reverb_artefacts\Release\Standalone\Reverb.exe` |

---

## Installing the plugin

`Reverb.vst3` is a **folder** (a bundle), not a single file. Copy the **entire** `Reverb.vst3` directory into a VST3 scan path, then rescan plugins in your DAW.

### Linux

**User install** (recommended — no root):

```bash
mkdir -p ~/.vst3
cp -r build/Reverb_artefacts/Release/VST3/Reverb.vst3 ~/.vst3/
```

**System-wide** (all users; requires root):

```bash
sudo cp -r build/Reverb_artefacts/Release/VST3/Reverb.vst3 /usr/lib/vst3/
```

Some hosts also check `/usr/local/lib/vst3/`. After copying, open your DAW’s plugin manager and run a **rescan**.

**Standalone:** run `build/Reverb_artefacts/Release/Standalone/Reverb` directly, or add it to your launcher.

### macOS

**User install** (recommended):

```bash
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R build/Reverb_artefacts/Release/VST3/Reverb.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

**System-wide** (all users; prompts for password):

```bash
sudo cp -R build/Reverb_artefacts/Release/VST3/Reverb.vst3 /Library/Audio/Plug-Ins/VST3/
```

Rescan plugins in your DAW (e.g. **Logic Pro → Settings → Plug-in Manager**, **Ableton → Preferences → Plug-ins**).

**Standalone:** open `Reverb.app` from Finder, or:

```bash
open build/Reverb_artefacts/Release/Standalone/Reverb.app
```

If macOS blocks an unsigned build, open **System Settings → Privacy & Security** and allow the app, or right-click the app → **Open** once to confirm.

### Windows

**Per-user** (no administrator; works with many hosts):

```bat
mkdir "%LOCALAPPDATA%\Programs\Common\VST3" 2>nul
xcopy /E /I /Y "build\Reverb_artefacts\Release\VST3\Reverb.vst3" "%LOCALAPPDATA%\Programs\Common\VST3\Reverb.vst3"
```

**All users** (common default scan path; requires Administrator):

1. Copy the folder `build\Reverb_artefacts\Release\VST3\Reverb.vst3`
2. Paste into:

   ```
   C:\Program Files\Common Files\VST3\
   ```

   Approve the UAC prompt if Windows asks for permission.

Rescan in your DAW (**FL Studio → Options → Manage plugins**, **Reaper → Preferences → Plug-ins → VST**, etc.).

**Standalone:** run `build\Reverb_artefacts\Release\Standalone\Reverb.exe`.

> **Tip:** If the host does not see the plugin, confirm you copied the whole `Reverb.vst3` folder (it should contain platform binaries inside), that you used a **64-bit** build for 64-bit hosts, and that the path matches one of the locations your DAW documents for VST3.

---

## Parameters

| Control       | Range                                  | Description                                      |
| ------------- | -------------------------------------- | ------------------------------------------------ |
| **Algorithm** | Room · Hall · Chamber · Plate · Spring | Selects the reverb profile and parameter mapping |
| **Room Size** | 0 – 1                                  | Space size (scaled per algorithm)                |
| **Damping**   | 0 – 1                                  | High-frequency absorption                        |
| **Wet**       | 0 – 1                                  | Reverb send level (shaped by ADSR)               |
| **Dry**       | 0 – 1                                  | Unprocessed signal level                         |
| **Width**     | 0 – 1                                  | Stereo spread                                    |
| **Attack**    | 1 ms – 2 s                             | Wet envelope attack                              |
| **Decay**     | 1 ms – 2 s                             | Wet envelope decay                               |
| **Sustain**   | 0 – 1                                  | Wet envelope sustain level                       |
| **Release**   | 10 ms – 10 s                           | Wet envelope release                             |

---

## Project layout

```
reverb/
├── CMakeLists.txt          # Plugin target and JUCE dependency
├── build.sh                # Configure and build helper
└── Source/
    ├── PluginProcessor.*   # DSP, parameters, state
    └── PluginEditor.*      # UI and control bindings
```

---

## Tech stack

- [JUCE](https://github.com/juce-framework/JUCE) 8.0.4 (fetched via CMake `FetchContent`)
- `juce::dsp::Reverb` for the main reverb engine
- `juce::AudioProcessorValueTreeState` for parameters and preset persistence

---
