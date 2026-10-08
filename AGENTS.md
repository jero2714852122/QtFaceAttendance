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

## Code style

Code that lands in this repository follows normal work standards, not tutorial
narration. Annotations in chat may be richer than the comments that end up in
the file.

- Comment the reason, not the mechanics. The code already shows what it does.
- Comment these explicitly: wire formats and byte layouts, thresholds and other
  magic numbers, non-obvious library behaviour, invariants, and workarounds that
  would otherwise look wrong.
- Use a short file-level comment only when the file's purpose is not obvious
  from its name and contents.
- Do not annotate every line. A block that needs a comment per line usually
  needs better names instead.
- Chinese comments are fine in this repository; match the surrounding file.

## Git workflow

The learner runs every stage, commit, and push from now on, to build fluency
with the tools.

- Codex finishes and verifies the change, then reports the changed files and a
  suggested commit message.
- Codex does not run `git add`, `git commit`, or `git push` in this repository.
- Keeping the working tree clean, splitting commits sensibly, and pushing is the
  learner's job. Remind them if verified work sits uncommitted or unpushed.
