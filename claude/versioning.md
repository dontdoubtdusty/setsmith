# Version numbers

Every project carries a three-part version: `major.minor.patch`, for example `1.2.2`.

- **patch** (last number): small tweaks only. A fixed typo, a nudged value, a color change, a one-line bug fix, a resized widget. `1.2.2` -> `1.2.3`.
- **minor** (middle number): anything bigger. A new feature, a reworked screen, a new file format, a real behavior change. Reset patch to 0. `1.2.2` -> `1.3.0`.
- **major** (first number): only when the user says so. Reset minor and patch to 0.

Rules:
- Bump the version in the same change that earns it, in the one place the project keeps it (see the project's `CLAUDE.md` or `claude/file-map.md`).
- Several tweaks in one session can share one patch bump; a session that finishes a feature gets a minor bump instead.
- Mention the new version in the change summary and in the session-log row.
- Projects that used two-part versions (for example `5.43`) read them as `5.43.0`. Their old "+0.01 per finished feature" rule is the same thing as a minor bump: `5.43.0` -> `5.44.0`. Tweaks now get a patch: `5.43.0` -> `5.43.1`.
- Big Juicy Suite opens the other Big apps from their own sites, so an app update does not bump the Suite. Bump the Suite only when its own files change, for example when an app's change also needs a change in the Suite's copy of that app's rules for its card (such as `shinyProg` for Big Shiny). Whenever an app changes how it counts or shows things, check the Suite's card code for that app.
- Formats inside a project (save files, export files) keep their own separate version numbers; this rule is for the app or project version.

## Where each version lives
Bump it here and nowhere else. Two-part numbers (`v0.45`) are read as `0.45.0` and written in full at their next bump.

| Project | Place |
|---|---|
| Big apps (budget, cooking, diet, reminders, cleaning, habits, suite) | `<span class="ver">` in the app header, `index.html` |
| birb-box | `Version` in `GameEngine.cs` (`"Birb Cube <n> Alpha"`) |
| maury-simulator | `Version` in `GameEngine.cs` (`"Maury Sim 0.05 Alpha"`) |
| dotburg-app | `Version` in `Program.cs` |
| sleepytime_ambience | `Version` in `Program.cs` |
| srd-helper | `@version` in `srd-helper.user.js` and the panel constant below it (keep in step) |
| srd-desk | `DeskApp.Version` |
| custom-uvs | `version` in `package.json` |
| loudness-analyzer | none yet; add `__version__ = "0.1.0"` at the first bump |
| Setsmith | not recorded here yet; check its `CLAUDE.md` and add it |

The number itself lives only in the code, never in these notes, so it cannot go stale here. A new project adds its row here the day it gets a version.

## Copies
The master copy is `GitHub/claude/versioning.md`. Copies live in each project's `claude/` folder (so a session opened inside one repo, or a cloud session that only clones that repo, still has the rules); change the master first, then copy it over every copy. Copies are whole-file copies, never edited in place: anything only one project needs goes in that project's `CLAUDE.md` instead.
