---
name: card
description: Make a Magic or UniVersus (UVS) card with Setsmith from an idea or full card text, render it to PNG and add it to a running set. Use when the user types /card or asks to make, design, redo or tweak a Magic/MTG or UniVersus/UVS card.
argument-hint: "[magic|uvs] <idea or card text> [art: <image path>]"
---

# /card: make a card with Setsmith

The user's request: $ARGUMENTS

## 1. Read the request

- **Game.** `magic`/`mtg` or `uvs`/`universus` in the request decides it. Without one, infer it from the content (mana costs, creature types, power/toughness mean Magic; difficulty, check, block, speed, damage, foundations, attacks mean UVS). Ask only if it is truly unclear.
- **Idea or text.** Fields the user spells out are used exactly as written. Design everything else: a fitting name, costs and stats that are fair for the effect, clean rules text in the game's own wording, and a short flavor text for Magic when there is room. Don't ask about details you can decide; the user can say "redo" or change a field afterwards.
- **Art.** If the request names an image file (`art: <path>` or just a path), pass it as `-Art`. Otherwise there is no art: the art box stays empty, and the user adds art later in Setsmith.
- **Redo or tweak.** "make it cost 3", "redo" or "change the name" means the last card made in this conversation. Write the whole card again with the change. The script replaces a card with the same name; if the name changes, the old card stays in the set, so mention it.
- **Not supported yet:** UVS Token and Arena frames, double-faced Magic cards, sagas, classes, and any Magic frame other than M15, M15 planeswalker and M15 tokens. Say so instead of faking it.

## 2. Write the card file

Write a UTF-8 text file into the scratchpad directory (one file per card, e.g. `card-ember-scout.txt`). Format: one `key: value` per line, no `card:` line, no indent. A value with several lines goes on the following lines, each indented with exactly one tab:

```
name: Ember Scout
rule text:
	Haste
	When Ember Scout enters, draw a card.
```

Leave out fields that don't apply. Never write an `image:` line; the script handles art.

### Magic fields (game `magic`, default frame M15)

| Field | Values and notes |
|---|---|
| `name` | |
| `casting cost` | Mana letters: `2UR`, `X1G`, `WUBRG`. Hybrid `W/U`, Phyrexian `H/R`, colorless `C`. |
| `super type` | `Legendary Creature`, `Instant`, `Artifact`, `Basic Land`, ... |
| `sub type` | `Human Wizard`, `Equipment`, ... (empty for none) |
| `rarity` | `common`, `uncommon`, `rare`, `mythic rare` |
| `rule text` | Plain text. Mana and tap symbols are plain letters, never braces: `T: Add B or R.`, `2R: This gets +1/+0.` Keywords and the card's own name are plain words. |
| `flavor text` | Wrap in `<i-flavor>...</i-flavor>`. |
| `power`, `toughness` | Creatures only. |
| `illustrator` | Artist credit when the user gives one. |
| `stylesheet` | Only for other frames: `m15-mainframe-planeswalker` or `m15-mainframe-tokens`. |

Planeswalkers: `stylesheet: m15-mainframe-planeswalker`, `loyalty: 4`, then each ability as `loyalty cost 1: +1` with `level 1 text: ...`, `loyalty cost 2: -2` with `level 2 text: ...`, and so on (up to 4 abilities). A static ability goes in a `level N text` with no `loyalty cost N`.

Tokens: `stylesheet: m15-mainframe-tokens`, `super type: Token Creature`, `sub type`, `power`, `toughness`, `rule text` if it has abilities. No casting cost or rarity.

### UVS fields (game `uvs`, frame 2026, exact size 744 x 1039)

| Field | Values and notes |
|---|---|
| `card type` | `attack`, `foundation`, `action`, `asset`, `backup`, `character` |
| `name` | |
| `keywords` | Keyword abilities, comma separated: `Stun: 1, Breaker: 2`, `Unique`, `Ally` |
| `traits` | Comma separated: `Weapon, Armor`, `Slam` |
| `rule text` | Labels `ENHANCE`, `RESPONSE`, `FORM`, `BLITZ` in capitals become label boxes. `{C}` is the commit icon. `{fire}` etc. are inline resource symbols. A paragraph that starts with `(` is reminder text. Usual shape: `ENHANCE Flip 1 foundation: This attack gets +2 damage.` |
| `difficulty`, `check` | Numbers. Every type except character. |
| `block`, `block zone` | Block modifier and `high` / `mid` / `low`. Every type except character. |
| `speed`, `damage`, `attack zone` | Attacks only; zone `high` / `mid` / `low`. |
| `stamina` | Backups only. |
| `hand size`, `health` | Characters only (characters have no difficulty, check or block). |
| `symbol 1` ... `symbol 6` | Resources: `air`, `all`, `chaos`, `death`, `earth`, `evil`, `fire`, `good`, `infinity`, `life`, `order`, `void`, `water`; add ` attuned` for the attuned version (`fire attuned`). Most cards have 1 to 3. |
| `rarity` | `C`, `UC`, `R`, `UR`, `SR`, `CH` (characters) |
| `face` | `B side` for the transformed face of a double-sided card. Make the front and the B side as two cards (two runs). |
| `credit` | Artist credit, e.g. `Illustration by Jane Doe`. |

Typical UVS numbers: difficulty 0-6, check 1-6, block modifier 0-4; attack speed 2-6 and damage 2-6 (faster or harder hitting means higher difficulty); character hand size 5-7 and health 20-35.

## 3. Render

Run in PowerShell from the Setsmith repo root:

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\card.ps1 -Game <magic|uvs> -CardFile "<card file>" [-Art "<image path>"]
```

It adds the card to `<Documents>\Setsmith Cards\Claude Magic.mse-set` or `Claude UVS.mse-set` (made on first use, with the previous copy kept as `.bak`), and saves the PNG in the `Claude Magic` / `Claude UVS` folder next to it. It prints `png:` and `set:` lines. If it prints `setsmith printed:` messages, a field value is probably wrong: fix the card file and run again. If `build\setsmith.com` is missing, run `tools\check.ps1` first.

## 4. Check the image, then report

Read the PNG and look at it before answering. Fix and render again if text overflows or is cut off, the text shrank too much to read (shorten the rules text), a symbol shows up as plain letters, or a stat is missing. Don't show the user a broken card.

Then give the user, briefly:
- the card as text (name, cost or stats, type line, rules text), in a few lines;
- the PNG path and the set path as full paths;
- any choices made for them (stats, name), so they can ask for a change.

If the user has the set open in Setsmith, tell them to reopen it to see the new card, and that saving from Setsmith while it's open with the old contents would drop cards added here.
