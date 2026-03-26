<p align="center">
<img src="https://mcppp.vercel.app/icon.png" alt="mcpp-logo" width="15%"/>
</p>

<div align="center">

  # MC++ 

A lightweight command line Minecraft launcher for Windows, written purely in C++.

  <img src="https://img.shields.io/github/v/release/TryFast/MCPP-Launcher" alt="Latest Release"/>
  <img src="https://img.shields.io/github/downloads/TryFast/MCPP-Launcher/total" alt="Total Downloads"/>
  <img src="https://img.shields.io/github/stars/TryFast/MCPP-Launcher" alt="Stars"/>
  <img src="https://img.shields.io/github/languages/code-size/TryFast/MCPP-Launcher" alt="GitHub code size in bytes"/>
  <img src="https://img.shields.io/badge/platform-Windows-blue?logo=windows" alt="Platform"/>
  <img src="https://img.shields.io/badge/language-C%2B%2B-blue?logo=cplusplus" alt="C++"/>
</div>

**No Bloat. No Java launcher overhead. Just a single lightweight portable `.exe` that downloads, manages, and launches Minecraft straight out of the box.**

Download the latest release from the [Releases](../../releases/latest) page.
or
- [Build Entirely From Source](#building-from-source)
---

## Features

- Launch Vanilla and Fabric versions of Minecraft
- Downloads Minecraft versions, libraries, assets, and natives automatically
- Automatically installs the correct Mojang JRE for each Minecraft version
- Parallel file downloads for fast installs
- Offline mode with username-based UUID generation
- Configurable RAM, Java path, and extra JVM arguments
- Console color themes cuz why not
- Single portable `.exe`, no installation required

---

## Requirements

- Windows 10 or later (64-bit)
- [MinGW-w64](https://www.mingw-w64.org/) or MSVC to build from source
- A Minecraft account is not required (offline mode supported)

---

## Building from Source

Clone the repo by clicking on the green `Code` button and click on `Download ZIP` then extract the folder,
Make sure you have MinGW-w64 installed and `g++` available in your PATH, then run Open CMD and run:

```batch
cd path\to\your\folder\where\the\cpp\file\is\located
g++ -std=c++17 -O2 -o MCPP.exe MCPP.cpp -lwinhttp
```

---

## Usage

Run `MCPP.exe` and use the menu to navigate:

```
[1] Launch      // Launch an installed version
[2] Download    // Download Vanilla or Fabric versions
[3] Settings    // Configure Username, RAM, Java path, etc etc
[4] Themes      // Change the console color theme (personally I like blue)
[5] Exit        // Exit this shi
```

On first launch you will be asked to enter a username. Everything else is configured through the Settings menu.

---

## Configuration

Settings are saved to `config.json` in the same folder as the exe.

---

## Supported Versions

- Vanilla: any version available in the Mojang manifest, from classic to latest
- Fabric: any Minecraft version supported by the Fabric API
- Bundled Java support included

---

## TryFast Comments

- Windows only. The launcher uses WinHTTP and the Window API throughout and will not build or run on Linux or macOS, ill add cross-platform later.
- All Minecraft files are stored on root folder adjacent to exe, so make sure to not put the exe file into any important folder as it will make things a little messy
- Anyways talk to me on Discord cuz I am bored: `bs.pk`, Adios!
 
---

## License

GPL v3.0
