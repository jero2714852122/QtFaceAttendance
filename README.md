# Qt Face Attendance

A Qt 6 desktop face-attendance system rebuilt from authorized educational source material.

The project is being reimplemented with C++20, CMake, Qt Widgets, Qt Network, Qt SQL, OpenCV, and public face-recognition models. The original source is used as a learning reference; the architecture and implementation in this repository are being rewritten.

## Current status

- Client application shell
- Server application shell
- Face recognition, networking, and persistence are not implemented yet

## Prerequisites

- Windows 10 or 11
- Visual Studio 2022 with Desktop development with C++
- Qt 6.9.1 for MSVC 2022 64-bit
- CMake 3.24 or newer

## Configure and build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH=E:/Dev/Qt/6.9.1/msvc2022_64
cmake --build build --config Debug --parallel
```

The local Qt path is an example. Replace it if Qt is installed elsewhere.

## License and attribution

The project is based on educational source code shared with permission. Third-party libraries and model licenses will be documented before their integration.
