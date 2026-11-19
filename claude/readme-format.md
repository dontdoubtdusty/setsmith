# README format

Every project's `README.md` follows this format unless the owner asks for something else. The model is the Drowse README (`sleepytime_ambience/README.md`). Apply it to new projects, and convert an old README the next time a session works in that project (no sweeps).

## Who it is for
Someone who only wants to run or use the thing. Nothing for developers: no build-from-source, dev setup, tests, architecture, changelog, badges, license or contributing sections. Those live in `CLAUDE.md` and `claude/`.

## Sections, in this order
1. `# Name`: the product name, not the repo name (Drowse, not sleepytime_ambience).
2. Pitch: two or three sentences under the title. What it is, the one thing that makes it worth having, and what it includes (for example "Fourteen scenes, plus a sleep timer, screen dimming and auto-rotate").
3. Screenshot: `![what it shows](docs/screenshot.png)`, for anything with a screen. Skip only when there is nothing to see.
4. `## Install`: numbered steps a non-technical person can follow, one action each, with the exact button names in **bold**. Name the traps (extract before running, the unsigned-app warning and how to get past it). Then, as short plain paragraphs: `Updates:` how a new version arrives, and `Uninstall:` what to delete, including where settings live.
5. `## Usage`: one short paragraph on how to get around (how to show the controls, the menu key, that settings save). Then a `| Key | Action |` table if there are shortcuts or commands; leave the table out if there are none.
6. `## Known limits`: honest bullets. Platform, unsigned, deliberate trade-offs (say "on purpose" when it is), what needs internet, side effects on the system.

## Adapting to other kinds of project
Keep the same sections; change what Install says.
- Windows app: download the zip from the latest GitHub release, extract, run the exe (the Drowse steps).
- Web app or site: the URL to open; "add to home screen" if it is a PWA. Uninstall only if there is local data to clear.
- Userscript: install a userscript manager, then the install link.
- Claude Code mod: where the folder goes, the setting that loads it, how to start it (the slash command).
- Kit or tool copied into other projects: what to copy where, and the one command to run.
- Local-only project with no release: say so, and how the owner starts it (the Desktop shortcut from the `Build <App>.bat`).

## Style
Plain language, short sentences, no emojis, no marketing words. Lines wrapped near 120 characters. Keep the whole file to about one screen; detail beyond that belongs in an in-app guide or a separate `GUIDE.md`.

## Copies
The master copy is `GitHub/claude/readme-format.md`. Whole-file copies live in each project's `claude/` folder, the same way as `versioning.md`: change the master first, then copy it over every copy. Anything only one project needs goes in that project's `CLAUDE.md`.
