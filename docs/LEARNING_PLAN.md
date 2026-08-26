# QtFaceAttendance 15-Day Learning Plan

## Outcome

By 2026-09-09, deliver a Qt 6 face-attendance demo that can be built, run,
explained, and shown during internship interviews.

Required evidence:

- A public GitHub repository with meaningful commit history.
- A working client and server.
- Camera capture and face detection/recognition.
- A framed TCP protocol with disconnect handling.
- SQLite employee and attendance records.
- Clean shutdown and visible error states.
- Build instructions, screenshots, architecture notes, and a short demo video.
- A one-page resume and at least 30 targeted applications.

Not required in this 15-day scope:

- Training a face model.
- Liveness detection.
- Production-grade security.
- Multiple simultaneous clients.
- Full reproduction of NotepadNext.

## Daily Roadmap

| Day | Main topic | Verifiable result |
| --- | --- | --- |
| 0 | Environment and repository | Qt 6/MSVC/CMake builds and runs both application shells |
| 1 | C++ and Qt application basics | Learner can edit, build, run, and explain the client entry point |
| 2 | Widgets and layouts | Client window has camera, status, identity, and action areas |
| 3 | Signals, slots, and timers | UI reacts to simulated status changes without blocking |
| 4 | OpenCV setup and camera | Camera frames are displayed in the client |
| 5 | Face detection | Detected faces are marked; first resume draft is prepared |
| 6 | TCP fundamentals | Minimal client/server text exchange works |
| 7 | Framed image protocol | JPEG frames survive partial and combined TCP reads; applications begin |
| 8 | Server architecture | Network, recognition, and UI responsibilities are separated |
| 9 | SQLite schema | Employee registration and queries work |
| 10 | Face recognition | Registered user is recognized with documented threshold behavior |
| 11 | Attendance workflow | Successful recognition writes a rate-limited attendance record |
| 12 | Failure handling | Camera, model, database, network, and shutdown failures are visible and safe |
| 13 | Tests and packaging | Protocol/database tests pass and a deployable Windows folder is produced |
| 14 | Documentation and demo | README, architecture diagram, screenshots, and demo video are complete |
| 15 | Interview rehearsal | Project can be explained at 30-second, 3-minute, and 10-minute depth |

## Learning Rule

The learner writes the core networking, threading, persistence, and business
logic. Codex may prepare the environment, explain concepts, review code, and
help diagnose errors. Each day ends with a runnable increment, a meaningful
commit, and short questions about the code that was written.

## Secondary Project

NotepadNext is a secondary source-reading exercise. It is limited to building
the project, tracing its main flow, and attempting one small fix after the face
attendance project has a stable end-to-end path.
