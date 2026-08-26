# Progress Log

## Current State

- Date: 2026-08-26
- Stage: Day 0 complete, Day 1 starting
- Active goal: Understand the smallest Qt Widgets program before adding more controls
- Next action: Explain the event loop and summarize the roles of the three Qt objects

## Completed

- Installed Qt 6.9.1 MSVC 2022 64-bit under `E:/Dev/Qt`.
- Confirmed Visual Studio 2022, MSVC, CMake, and the integrated camera.
- Created `E:/Projects/QtFaceAttendance`.
- Created client and server Qt Widgets application shells.
- Configured and built both targets successfully.
- Verified both executables start.
- Created and pushed the public GitHub repository.
- Configured the project in Qt Creator 17 with the MSVC Qt kit.
- Verified ordinary run works from Qt Creator.
- Changed the client label text, rebuilt the target, and verified the new text at runtime.
- Completed the first Git inspect, stage, commit, and push workflow.
- Published learner-authored commit `c022b06` to GitHub.
- Replaced inferred `auto*` with explicit `QLabel*` and verified a fresh build.
- Explained object versus pointer member access and Qt parent ownership correctly.
- Explained why a source-only change requires rebuilding but not reconfiguring.

## Known Environment Notes

- Use the `Qt 6.9.1 (msvc2022_64)` kit for this project.
- Use `Ctrl+B` to build and `Ctrl+R` to run.
- Debugging is not configured because CDB is not installed yet.
- The existing `Desktop Qt 6.9.1 MinGW 64-bit` kit is not used by this project.
- OpenCV and face-recognition models are not installed yet.

## Day 1 Checklist

- [x] Change the client label text and verify the change at runtime.
- [ ] Explain the roles of `QApplication`, `QMainWindow`, and `QLabel` in plain language.
- [x] Explain configuration, build, and run as three different steps.
- [x] Learn what a pointer stores and what Qt parent ownership does.
- [x] Make the first learner-authored commit.

## Questions To Revisit

- What does the Qt event loop do?
- Why must `QApplication` be created before widgets?
