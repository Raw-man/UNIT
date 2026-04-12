# UNIT: a CLI program for modding NSUNI and NSLAR

## Description

Ultimate Ninja Impact Toolbox is a CLI program dedicated to the modding of two PlayStation Portable titles:

*   **Naruto Shippuden: Ultimate Ninja Impact**
*   **Naruto Shippuden: Legends: Akatsuki Rising**

## Features 

-   **Archive Management**:
Unpack and repack various game data archives (e.g *.BIN files).
-   **Format Conversion**:
Export assets to and import from standard exchange formats, such as glTF and PNG. Supports:
    - 3D & Environment: models, lights, cameras, fog configs.
    - 2D: textures, text archives, minimap configs.
    - Audio: sound banks.
  
![naruto](https://github.com/user-attachments/assets/453d3dd1-3d77-4dc9-9e1c-f227ad6de514)

## Usage

Run the program with the --help option to get more info.

```bash
unit --help
unit imp --help
unit imp mdl --help
```

## Building

Ensure you have the following installed:

- Git
- Ninja or Make
- C++17 compiler (GCC 13+, Clang 16+, or MSVC 19.28+)
- CMake 3.20+

Run the following commands:

```bash
git clone https://github.com/Raw-man/UNIT.git
cd UNIT
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Related Projects

*   **[NNL](https://github.com/Raw-man/NNL)**: The underlying library responsible for asset processing.
*   **[Web Viewer](https://rcjn.itch.io/view)**: A browser-based tool for inspection of game assets.
