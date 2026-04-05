# UNIT: a CLI program for modding NSUNI and NSLAR

## Description

Ultimate Ninja Impact Toolbox is a CLI program dedicated to the modding of two PlayStation Portable titles:

*   **Naruto Shippuden: Ultimate Ninja Impact**
*   **Naruto Shippuden: Legends: Akatsuki Rising**

## Features 

-   **Archive Management**:
Unpack and repack various game data archives.
-   **Format Conversion**:
Export assets to and import from standard exchange formats, such as **WAV**, **glTF** and **PNG**.

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

