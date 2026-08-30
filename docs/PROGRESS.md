# Progress Log

## Current State

- Date: 2026-08-30
- Stage: Day 5 verified, release pending
- Active goal: Commit and push the verified face-detection increment
- Next action: Upload Day 5, then announce the Day 6 TCP lesson

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
- Published learner-authored Day 3 commit `d4a669a` to GitHub.
- Installed the official OpenCV 4.12.0 Windows package under `E:/Dev/OpenCV`.
- Configured the client CMake target for OpenCV `core`, `imgproc`, and `videoio`.
- Verified CMake finds OpenCV, the MSVC Debug build succeeds, and the required OpenCV DLL is copied beside the client executable.
- Replaced simulated preview text with real `cv::VideoCapture` frame reads.
- Converted OpenCV BGR frames to Qt RGB images and displayed them in the preview label.
- Corrected the front-camera mirror effect with a horizontal `cv::flip`.
- Verified camera open failure handling, timer-driven frame updates, camera release, and a fresh Debug build.
- Published learner-authored Day 4 commit `789a7b9` to GitHub.
- Added the Haar cascade face-detection model to the project assets.
- Configured the build to copy the model beside the executable and verified the copied model file.
- Loaded the Haar cascade from the executable directory so the runtime working directory does not matter.
- Added live face detection and rectangle drawing to the camera preview.
- Throttled detection to roughly 100 ms while keeping preview updates at roughly 33 ms to reduce GUI-thread stalls.
- Verified a fresh Debug build after the face-detection changes.

## Known Environment Notes

- Use the `Qt 6.9.1 (msvc2022_64)` kit for this project.
- Use `Ctrl+B` to build and `Ctrl+R` to run.
- Use `F5` to build and debug; CDB is configured and verified.
- Qt Creator 20.0.1 is installed under `D:/tools/QT/Tools/QtCreator`.
- Windows SDK CDB is installed at `C:/Program Files (x86)/Windows Kits/10/Debuggers/x64/cdb.exe`.
- The existing `Desktop Qt 6.9.1 MinGW 64-bit` kit is not used by this project.
- OpenCV 4.12.0 is installed under `E:/Dev/OpenCV/opencv`; face-recognition models are not installed yet.

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

## Day 4 Checklist

- [x] Install and configure OpenCV 4.12.0 for the MSVC Qt client.
- [x] Open the default camera and report failure visibly.
- [x] Read real frames with `cv::VideoCapture` and `cv::Mat`.
- [x] Convert BGR frames to RGB and display them in `QLabel`.
- [x] Correct the mirror orientation and release the camera on stop.

## Day 5 Checklist

- [x] Load the Haar cascade model from the application directory.
- [x] Detect faces in each captured frame.
- [x] Draw a rectangle around each detected face.
- [x] Report model or detection failures visibly.
- [x] Prepare a first resume bullet describing the camera and detection work.

## Resume Draft

- 使用 Qt 6/C++、OpenCV 构建人脸考勤客户端：通过 QTimer 驱动摄像头采集，完成 BGR/RGB 图像转换、镜像校正、Haar 级联人脸检测与实时矩形标注，并处理模型加载失败、摄像头打开失败和资源释放。

## Day 6 Checklist

- [ ] Explain TCP client/server roles and connection lifecycle.
- [ ] Add a server listening endpoint.
- [ ] Connect the Qt client and exchange a test message.
