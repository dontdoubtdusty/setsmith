# QA plan

## Automated (every change)

Run `tools\check.ps1`. It must end with `ALL CHECKS PASSED`. It checks:
- The build.
- The render test: 7 cards compared byte for byte against the reference hashes.
- The GUI: it opens the test set for 6 seconds and fails on any popup or error output.

## Manual GUI checks (UI changes)

1. **Startup:** the welcome window says Setsmith and opens with no updates button. New set with the M15 style works.
2. **Tabs:** only Cards, Style and Set info appear.
3. **Set symbol, Set info tab:**
   - Click Upload. The dialog opens, and "Use this symbol" is disabled.
   - Choose a transparent PNG. The preview shows four rarities, and the button enables.
   - "Use this symbol": the field and the card rarity symbols update. Undo goes back to the previous symbol.
   - Save, reopen: the symbol is still there.
   - A JPG without transparency shows the "dark pixels" note. A tiny image shows the "small" note. A blank image is refused.
4. **Messages:** a template warning shows one popup, not one per message.
5. **Printing:** File > Print preview renders cards.
6. **Spell check:** a misspelled word in rules text is underlined.

## Open items

- Step 3, from "Use this symbol" on, has not been run by hand yet. The automated session was locked during the last test.
