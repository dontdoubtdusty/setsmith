# UniVersus (UVS) template for Setsmith: spec

Status (2026-11-08): the Foundation frame is built (bleed and exact). Other types are next. Progress is in `ROADMAP.md`, Phase UVS.

Corrections found while building (they take precedence over the text below):
- **Foundation rules text is white**, not dark. FT01 also gives it a thin dark outline; TK802 has none, and the base frame has none.
- **Difficulty and check have no badge box.** The number itself has a black outline plus an outer outline in the type color. The boxes on TK802 are a Tekken set style.
- **The name has the same double outline** (black, then type color), not a drop shadow.
- **Rules text is vertically centered** in the text box, below the keyword strip when there is one.
- **The keyword strip** (a darker band of the type color) appears only when the card has keywords or traits.
- **Fonts in use** (closest free matches): Barlow Condensed SemiBold 25.5 pt for rules text; Figtree Black Italic for the name and numbers; Figtree ExtraBold Italic for the keyword line; Figtree SemiBold / Black for the footer.
- **Footer, measured on FT01 2x scans (card units):** logo ring 41.5 across, centered at 68,988; white divider 2.3 wide at x 102, y 974-1002.5; resource symbols centered at y 988, 47.5 apart, the first at x 135.75. Circle symbols are 40 across with a thin white halo; attuned symbols are 41-unit rounded squares (white rim, black border, colored fill, same inner design).
- **IP logo:** a white logo, about 28 units, centered in the ring. Inside the ring is black, which shows on full-art cards. Users supply a square white-on-transparent PNG in Set info.
- **Resource symbol art** is vector art in the How to Play guide, page 21 (`tools/extract-uvs-symbols.ps1`).
- **Full-size scans:** uvsultra serves `<card>.jpg` without `-preview`. FT01 cards are 1488 x 2079 (2x); others are 744 x 1039.
Goal: custom UVS proxy cards made in Setsmith that look 1:1 like real 2026 cards.
This is for personal use only and is never redistributed, so we can use the official fonts and extracted symbols.
UVS Games allows and encourages custom proxies.

## Decisions (2026-11-08)

| Topic | Decision |
|---|---|
| Where | A Setsmith (MSE) template: one game package and its style packages. This replaces the earlier "Setsmith stays Magic-only" decision. |
| Frame era | 2026 frame only. It has boxed ability labels, the Action type and green Assets. Older frames (2025 Solo Leveling with colored-word labels and blue Assets; 2021-2023 Jasco frames with the name on top and a symbol column) are out of scope. |
| Borders | One clean base frame first, modeled on TK802 Tekken (plain black edge). Per-set border themes come later as a style option: inked manga (FT01), halftone comic (GDZ01), glitch (SL04). |
| Card types | Attack, Foundation, Action, Asset, Backup, Character, Token, Arena (landscape), plus transformed B sides. |
| Double-sided | Each face is a separate card and exports as a separate image (`005.png`, `005B.png`). |
| Export size | Picked by stylesheet. Default: print-shop bleed, 819 x 1114 px at 300 dpi. Options: exact size, 744 x 1039 px, same as the official PNGs; MPC, 822 x 1122 px (MakePlayingCards poker template). Add a printer by copying `uvs-2026-mpc.mse-style` and setting `card width/height`, `bleed_x` and `bleed_y`. |
| Footer | Each set sets its own IP logo image (the left circle) and its own copyright/credit line. |
| Fonts | Use the official typefaces. See "Fonts" below. |

## Sources

- `D:\My Documents\rpret\GitHub\uvsrefs.txt`: about 37 uvsultra.online previews (358 x 500) plus 4 larger images.
- Official 744 x 1039 PNGs from uvsgames.com news posts (e.g. `BL01-003-CH-RafatheExo-Soldier-1.png`).
- TCGplayer product images, 1000 px tall: `https://tcgplayer-cdn.tcgplayer.com/product/<id>_in_1000x1000.jpg`. Use these for measuring.
- How to Play guide: `https://uvsgames.com/wp-content/uploads/2025/09/How_to_Play-2025.pdf`. It names the card parts (pages 5-8).
- Rules Reference v1.2.12: `https://uvsgames.com/wp-content/uploads/2026/08/UniVersus-Rules-Reference-v1.2.12-8-14-2026.pdf`

## Card anatomy (2026 portrait frame)

Positions are approximate fractions of the card (x from left, y from top), read from the 358 x 500 previews. Measure them properly on 1000 px scans before drawing.

### Size and edge

- 2.5 x 3.5 in, 744 x 1039 px at 300 dpi, rounded corners.
- The art covers the whole card. A thick black edge runs around it and gets wider at the bottom, where the footer sits.
- Badges have a thin rim in the **type color**, and the name's drop shadow uses it too.

### Type colors

The text box tint shows the card type.

| Type | Text box | Text color | Stats |
|---|---|---|---|
| Attack | orange | white | difficulty, block, speed + attack zone, damage, check |
| Foundation | light grey | dark | difficulty, block, check |
| Action | blue | white | difficulty, block, check |
| Asset | green | white | difficulty, block, check |
| Backup | magenta | white | difficulty, block, check, stamina |
| Character | purple panel, or dark shading over the art | white | hand size, health |
| Token | no box; dark shading over the art | white | none |
| Arena | landscape layout (see below) | white | difficulty, block, check |

### Stats

- **Difficulty** (top-left, about x 2-17%, y 1-13%):
  - A big italic number with a yellow-to-orange-red gradient and a dark outline.
  - It sits in a rounded square badge with a rim in the type color.
- **Block** (top-right, about x 80-98%, y 2-12%):
  - A shield whose fill shows the **block zone**: red for high, orange for mid, yellow for low.
  - The number is white with a black outline.
  - To the right is a tab with three slots. A `+` sits in the slot for the zone (top = high, middle = mid, bottom = low); the other two slots are grey.
  - The outer rim is the type color.
- **Check** (bottom-right, about x 80-98%, y 80-92%): a big italic number with a light-blue-to-blue gradient and a dark outline, in a rim of the type color.
- **Attack only:** these two stack on the right edge of the text box, above the check.
  - **Speed** is in a circle filled with the attack zone color, with an arrow notch pointing out.
  - **Damage** is in a yellow starburst.
- **Backup:** **stamina** in a green heart on the right edge of the text box.
- **Character:**
  - **Hand size** is in a green card-stack badge.
  - **Health** is in a red heart.
  - Both stack on the right, above the text.
  - There is no difficulty, block or check.
  - Rarity prints as `CH`, `CH-C` or `CH-R`.
  - Full art: the image covers the whole card and the bleed (819 x 1114 px for print). No text box, as on the official cards: name, traits and rules text have a black outline and a hard drop shadow, over a light scrim (bottom half, strongest on the left: `shade-character.png`). No watermark.
  - Optional `cutout` field: the character cut out of the art (same framing as the image), drawn above the scrim and below the text so the body stays bright. Not editable in the GUI: an empty editable image field would show a checkerboard over the card. Set by a builder script.
  - Set info `character theme`:
    - **standard:** just the above.
    - **bonus stage:** a torn salmon brush swatch behind the name (`bonus-brush.png`, sized from the name length; two lines past 15 letters), like the ink swatches behind FT01 / SF6 Character names. The name's outer outline and the badge rims turn salmon.
- **Token:** no stats. The name is black on a white tag. The footer reads `Token T <SET> 1/1`.

### Name and the line under it

- **Name** (left, just above the text box, up to 2 lines):
  - Heavy italic caps.
  - White, with a black outline and a drop shadow in the type color.
- **Keyword/trait line** (below the name): small bold italic caps.
  - Keywords come first. Keywords with a value (`QUEST: 1`, `BREAKER: 1`, `EX: 3`, `POWERFUL: 3`) are printed in **yellow**.
  - Then a separator, `|` or `/`.
  - Then traits in white, joined with ` • `.
  - Example: `BREAKER: 1 • UNIQUE | WEAPON`.
  - Characters show their traits only, e.g. `MAGE • OCCULTIST`.

### Rules text

- Bold sans text, left aligned.
- **Ability labels** are colored boxes with black text, followed by the cost, a colon, then the effect:
  - `ENHANCE`: yellow
  - `RESPONSE`: green
  - `FORM`: light blue
  - `BLITZ`: pink/lilac
- Keywords used in the text (`QUEST: 1`, `ECHO`, `STUN: 2`) get their own yellow boxes.
- Bracketed conditions like `[Once per turn]`, `[Card Pool]` and `[Tenacious]` are plain text.
- **Reminder text** is italic, in parentheses, and starts with a small label, e.g. `(QUEST: 1 – RESPONSE: ...)`.
- **Inline icons:**
  - The commit/flip card icon.
  - Zone icons, used in phrases like "your [high] attacks".
  - Resource symbols.
- Some sets put a faint watermark in the text box (FT01 shows its logo). This belongs to the border themes, not the base frame.

### Footer

About y 93-99%, on the black edge.

- **Left:** the set's IP logo in a circle, a thin `|`, then 1 to 4 resource symbol circles.
- **Right, top line (bold):** `<Type> <Rarity> <SETCODE> <n>/<total>`, e.g. `Asset UR MHA09-CS2 4/21`.
- **Right, bottom line (small):** the credit and copyright text, e.g. `Illustration by X © 2026 ...`.
- Rarities seen: C, UC, R, UR, SR, T, CH, CH-C, CH-R.

### Arena (landscape)

TK802-020 and TK802-038 are examples.

- The art covers the whole landscape card.
- The name runs vertically in a white tag along the left text column, and the rules text sits in that column.
- Difficulty is top-right, check is bottom-left, and the block shield is bottom-right, all rotated with the frame.
- The footer runs vertically on the left edge. Its type reads `Arena`.
- **Needs its own landscape style** (1039 x 744).

### B sides

- Seen on SL04-005B and SL04-006B: a separate card with the same layout and its own type. The number gets a suffix: `005` and `005B`.
- The card text refers to the other face with "build it transformed (opposite side up)".

## Fields (game package)

| Field | Notes |
|---|---|
| type | attack, foundation, action, asset, backup, character, token, arena |
| name | |
| keywords | keywords with values; printed yellow, before the separator |
| traits | joined with ` • ` |
| separator | `|` or `/`, default `|` |
| rules text | tags for ability labels, keyword boxes, reminder text and inline symbols |
| difficulty, check | text, so `X` and `*` work |
| block modifier, block zone | zone is high, mid or low |
| speed, damage, attack zone | Attack only |
| stamina | Backup only |
| hand size, health | Character only |
| resource symbols | 0-4, chosen from the 13 resources |
| rarity | C, UC, R, UR, SR, T, CH, CH-C, CH-R |
| number, face suffix | the suffix is blank or `B` |
| art, artist/credit | |
| set fields | set code, total cards, IP logo image, copyright line, border theme |

The 13 resource symbols: Air, All, Chaos, Death, Earth, Evil, Fire, Good, Infinity, Life, Order, Void, Water.

## Assets to make or extract

- **Fonts.** The How to Play PDF does **not** contain the card fonts: its card pictures are flat images.
  - The fonts it embeds belong to the guide layout: Figtree (Light, Italic, ExtraBold Italic). The playmat pages also use Avenir Next Bold Italic, Exo 2, Myriad Pro and Tiller Bold.
  - Figtree (free, Open Font License) is the top candidate for the rules text and footer. Check it by rendering it next to a 1000 px scan.
  - The name and number fonts are still unknown.
  - Another source: a Reddit thread points to fan-made templates, with the font files included, pinned in the "Fan Made Cards" channel of Tam's Discord (a UVS content creator). The user can fetch those, since Claude can't open Discord.
- **Resource symbols** (13), the commit/flip icon and the zone icons: extract from high-res scans or official web assets, then trace them as clean vectors or PNGs.
- **Frame pieces.** Rebuild these as layered PNGs at 2x resolution:
  - the difficulty badge
  - the check badge
  - the shield, plus its three-slot tab
  - the speed circle, with its arrow
  - the damage starburst
  - the stamina heart, health heart and hand-size badge
  - the text box tints
  - the black edge and footer
  - the ability label boxes
- **Number gradients.** Difficulty goes yellow to orange-red; check goes light blue to blue. Draw these in the style itself, or as pre-made glyph images.

## How to build it in MSE

- **Packages:**
  - `uvs.mse-game`
  - `uvs-2026.mse-style` (portrait)
  - `uvs-2026-arena.mse-style` (landscape)
  - `uvs-symbols.mse-symbol-font` (inline icons)
  - Each card can pick its own style, so Arena cards use the landscape one.
- **Bleed vs exact size:** MSE sets the card size per style, so this is probably two style variants that share the same include files. Check whether 2.5.6 can change the size from a style option before deciding.
- **Slim data:** `tools/make-slim-data.ps1` only copies Magic packages. Add the UVS packages to it.
- **Render test:** add UVS cards to the render test, one of each type plus a B side and an Arena.
- **Build order:**
  1. Foundation, the simplest frame.
  2. Action, Asset and Backup (tint and stamina changes).
  3. Attack (speed and damage).
  4. Character and Token.
  5. Arena.
  6. Border themes.

## Open questions

- The name font is not identified yet. Stat numbers can use the official digit images cut in the study (`claude/uvs-study.md`, section 3); they change style per set.
- Answered by the study (2026-11-12): the text box is a fixed size in every 2026 set (top at y 643-651), and the badge art now comes from the scans (`study/uvs-graphics`).
