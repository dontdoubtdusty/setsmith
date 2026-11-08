# Session log

Newest first. Keep each entry short: what changed, what was verified, what's next.

## 2026-11-08

- **Phase 2 removals:** removed the Keywords, Statistics and Random Pack tabs, the package manager, the update checker, HTML export, the profiler and the symbol editor. Replaced the Console with popups.
- **Set symbol upload (`0d366599`):**
  - Added an upload dialog with a four-rarity preview and best-practice tips.
  - PNG symbols render on cards through the normal rarity filters.
- **Verified:**
  - `check.ps1` passes.
  - Rendered cards show the image symbol in all rarities.
  - The dialog preview works.
- **Not yet verified:** the final "Use this symbol" click, because the desktop was locked.
- **Framework:** copied from birb-box: CLAUDE.md, `claude/` docs and the Jello Board.
- **Next:**
  - Hand-check the upload.
  - Pick a new name for the fork.
  - Phase 3: packaging.
