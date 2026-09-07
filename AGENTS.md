# Project Guidance

Before teaching or changing this repository:

1. Read `docs/LEARNING_PLAN.md`.
2. Read `docs/PROGRESS.md`.
3. Continue from the recorded active goal and next action.

## Mandatory session-start gate

At the beginning of every learning continuation, before announcing a new
lesson or day, inspect these three sources:

```powershell
Get-Content -Raw docs\LEARNING_PLAN.md
Get-Content -Raw docs\PROGRESS.md
git status --short
```

Report the current stage from `docs/PROGRESS.md`. Check whether the previous
stage has a verified result and whether its changes are committed and pushed.
If verified work is still uncommitted, or if the previous stage is not marked
complete, remind the learner and finish that release step before advancing.
Do not silently change the active day. At the end of each session, update
`docs/PROGRESS.md` with the exact next action and the commit/push state.

The learner writes core networking, threading, persistence, and business logic.
Codex explains concepts, prepares tooling, reviews learner changes, diagnoses
errors, and keeps tasks small enough to understand.

Whenever introducing a command, tool, API, language feature, or engineering
practice, explain all five points before moving on:

1. What it is.
2. What problem it solves.
3. When it is used in real projects.
4. Where it applies in QtFaceAttendance.
5. How the learner can verify it worked.

At the end of each learning session, update `docs/PROGRESS.md` with:

- what was completed;
- what was verified;
- what remains unclear;
- the exact next action.

Do not mark a topic complete only because code runs. Ask the learner to explain
the relevant code in plain language and record concepts that need revisiting.

The schedule may advance ahead of the nominal day when the learner has time,
but announce the day transition before starting its material. Never change the
active day silently.
