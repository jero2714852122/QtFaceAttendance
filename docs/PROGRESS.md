# Progress Log

## Current State

- Date: 2026-08-28
- Stage: Day 3 complete
- Active goal: Preserve the completed signal-slot and timer simulation as a learner-authored commit
- Next action: Review, commit, and push the Day 3 client interaction changes

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
- Verified that removing `app.exec()` ends the process immediately, then restored it.
- Explained the roles of `QApplication`, `QMainWindow`, and `QLabel` in plain language.
- Completed the Day 1 entry-point explanation and review.
- Published learner-authored commit `fddd3e4` using an explicit `QLabel*` type.
- Added a central widget, vertical layout, and stable camera-preview placeholder.
- Verified the Day 2 interface increment builds and displays correctly.
- Upgraded Qt Creator to 20.0.1 and installed the matching CDB extension.
- Configured the Windows SDK CDB debugger for the MSVC Qt kit.
- Verified `F5` automatically builds, starts CDB, stops at the `window.show()` breakpoint, exposes local variables, and continues to the application window.
- Added compact running-status and recognized-identity labels below the camera placeholder.
- Used a layout stretch factor so the camera area receives the window's extra vertical space.
- Added a nested horizontal action layout with enabled start and disabled stop buttons.
- Explained vertical versus horizontal layout order, nested-layout ownership, and widget destruction through the Qt parent object tree.
- Published learner-authored Day 2 commit `4658ee6` to GitHub.
- Connected the camera action buttons to lambdas that update labels and button enabled states.
- Added a 500 ms `QTimer` that simulates recurring camera-preview updates.
- Verified the timer starts and stops with the camera controls while the window remains responsive.
- Explained that 500 ms equals 0.5 seconds and that timer events do not occupy the GUI thread with a blocking loop.

## Known Environment Notes

- Use the `Qt 6.9.1 (msvc2022_64)` kit for this project.
- Use `Ctrl+B` to build and `Ctrl+R` to run.
- Use `F5` to build and debug; CDB is configured and verified.
- Qt Creator 20.0.1 is installed under `D:/tools/QT/Tools/QtCreator`.
- Windows SDK CDB is installed at `C:/Program Files (x86)/Windows Kits/10/Debuggers/x64/cdb.exe`.
- The existing `Desktop Qt 6.9.1 MinGW 64-bit` kit is not used by this project.
- OpenCV and face-recognition models are not installed yet.

## Day 1 Checklist

- [x] Change the client label text and verify the change at runtime.
- [x] Explain the roles of `QApplication`, `QMainWindow`, and `QLabel` in plain language.
- [x] Explain configuration, build, and run as three different steps.
- [x] Learn what a pointer stores and what Qt parent ownership does.
- [x] Make the first learner-authored commit.

## Questions To Revisit

- Why are local C++ objects destroyed in reverse construction order?
- How a header declaration differs from a linked library implementation.
- How content alignment differs from assigning a main-window central widget.
- The exact destruction sequence after `app.exec()` returns.

## Day 2 Checklist

- [x] Create a central container and attach a vertical layout.
- [x] Add a stable camera-preview placeholder.
- [x] Add visible status and identity areas.
- [x] Add an action-button row.
- [x] Build, run, and explain layout ownership in plain language.

## Day 3 Checklist

- [x] Connect button `clicked` signals to small Lambda responses.
- [x] Keep start/stop button states consistent with the simulated camera state.
- [x] Use `QTimer::timeout` for repeated non-blocking preview updates.
- [x] Stop the timer and restore the preview when the simulated camera closes.
- [x] Explain why timer-driven updates leave the Qt interface responsive.
