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
- Formats inside a project (save files, export files) keep their own separate version numbers; this rule is for the app or project version.

The master copy is `GitHub/claude/versioning.md`. Copies live in each project's `claude/` folder; change the master first, then the copies.
