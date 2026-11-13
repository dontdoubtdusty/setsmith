# UniVersus study (2026-11-12)

What 3,816 real cards (39 sets, MHA01 2021 through FT01 2026), their text, the How to Play guide (2025) and the Rules Reference v1.2.12 teach us, for the Setsmith UVS template (`claude/uvs-template.md`) and for designing custom cards that play fair.

Personal use only. All source material lives in `study/` (git-ignored); the scripts that made it are in `tools/uvs-study/` (run from the repo root with `D:\msys64\ucrt64\bin\python.exe`; Pillow and numpy are installed there).

| Where | What |
|---|---|
| `study/uvs-cards/<code>/<nnn>.jpg`, `manifest.tsv` | The scans (744 x 1039; FT01 is 1488 x 2079) |
| `study/uvs-cards/cards.json` | Name, type, rarity, rules text (`**bold**` = labels/keywords, `{good}` = symbols), symbols (`at_` = attuned), stats for every card |
| `study/uvs-graphics/<set>/` | Official badges cut from the scans (see Graphics), plus `frame/<type>-median.png` |
| `study/docs/` | How to Play PDF and Rules Reference PDF, with text dumps (`htp.txt`, `rr-raw.txt`) |
| `study/analysis.txt`, `analysis2.txt` | Full statistics tables this doc summarizes |

## 1. Which sets use the 2026 frame

Boxed ability labels (the frame we build): **FT01, SF62X, TK802, MHA09, GDZ01, KF0X, GGS01, TMNT01, SF60X**, plus BL01 and Promo 2026. That is 1,022 cards.
- SLEB, AOT03 and older Challenger decks: colored-word labels (2024-2025 frame).
- MHA05 and older, RHID01: Jasco frame (name on top, symbol column). Out of scope.

## 2. Layout (from per-set median images)

Taking the per-pixel median of every card of one type in one set erases the art and leaves the frame. `frame/<type>-median.png` shows it; `<type>-steadiness.png` is white where every card is identical.
- **The text box is a fixed size.** In every 2026 set the name bar sits at y 598-618 and the box top at y 643-651 (744-wide units), whatever the art. This closes the spec's open question. The template's 648 is right.
- Per set, the border is a set theme: TK802 has chamfered inner corners on the art window, GDZ01 torn edges, FT01 ink brush edges, TMNT01 no text box on Backups (text over art).
- Footer, ring, divider and symbol positions are identical across sets (already measured into the template).

## 3. Graphics (official, cut from the scans)

Method (`tools/uvs-study/extract_by_value.py`, then `clean.py`): group cards whose badge must look identical (same type and difficulty, same block zone and modifier, same speed and zone...) using `cards.json`, take the median for color and use "same on every card" as transparency. Then keep only the piece connected to the badge center. 351 PNGs in 10 sets.

| Folder | Contents | Best source |
|---|---|---|
| `difficulty/<type>-<n>` | Difficulty numbers: gradient fill with speed lines, black outline, outer rim in the type color | FT01 (2x) for the plain style |
| `check/<type>-<n>` | Check numbers, blue gradient | FT01, TK802 |
| `block/<type>-<zone>-<n>` | Shield with zone fill (red high, orange mid, yellow low), white number, three-slot tab | FT01, TK802 |
| `speed/<zone>-<n>` | Speed circle in the zone color, with the arrow **pointing up-right (high), right (mid) or down-right (low)** and two grey arrowheads | FT01, TK802 |
| `damage/<n>` | Yellow starburst, dark outline | TK802, TMNT01 |
| `hand-size/<n>`, `health/<n>`, `stamina/<n>` | Green card stack, red heart, green heart | FT01, GDZ01, TMNT01 |
| `frame/` | Whole-card medians per type | TK802 (cleanest base frame) |

What the cuts show that the template does not do yet:
- **The stat numbers change typeface per set.** FT01, SF62X and KF0X share a rounded heavy italic (the house default). TK802 uses a squared Tekken face, GDZ01 a splattered one, TMNT01 a graffiti one. So "per-set number fonts" on the roadmap is real, and the default style should copy FT01.
- The digits are not flat gradients: they carry fine speed lines (difficulty) or highlights (check). Our `num-d-*.png` and `num-c-*.png` are flat. The FT01 cuts can replace them directly, since the template already draws numbers as images.
- **The shield's `+` tab marks the zone**: the zone's slot (top high, middle mid, bottom low) is filled with the zone color and holds a white `+`. Checked 2026-11-12: our shields already match.
- Speed arrow direction encodes the zone (see the table). Redrawn 2026-11-12 to match: big zone-colored arrow toward the zone, grey arrowheads on the other two.
- Known bad cuts: GDZ01 speed circles (only the digit survived), FT01 damage 4 (cleanup kept the wrong piece). Some FT01 cuts carry a scrap of the black ink border; crop before use.
- Not cut: ability label boxes (they move with the text) and inline icons. The resource symbols already come from the How to Play vector page.

### Fonts

Fonts cannot be pulled out of JPG scans; scans only hold pixels. Options, in order:
1. Use the official digit images (above) for difficulty, check, block, speed, damage, hand size and health. That covers every number on the card without needing the font.
2. For the name, keyword line and rules text keep the closest free matches already in `templates/fonts` (Figtree, Barlow Condensed), and compare them at 2x against FT01 (`study/uvs-cards/ft01`).
3. The real font files: only the fan-template pack in Tam's Discord ("Fan Made Cards" channel) is known to have them. You would have to fetch that; Claude cannot open Discord.

## 4. How the game works (How to Play + Rules Reference)

- **Win:** take the rival from starting health to 0. Also, when the deck empties you cycle (shuffle the discard pile into a new deck and remove its top 10 from the game); if nothing is left after cycling, you lose. Best of 3.
- **Deck:** 1 character + at least 60 cards, at most 4 copies each, suggested 30% attacks. Pick one of the character's symbols as the deck's primary symbol; every card must have it. **Attuned** (square) symbols are playable if the character has that symbol at all. **Infinity** goes in any deck.
- **Turn:** Start (ready stage, review = discard 1, draw up to hand size), Combat, End. The first player starts with the character committed, skips their first Start Phase, and nobody attacks on their first turn. The second player draws 1 extra on turn one.
- **Playing a card = a check.** Put it in the card pool, then flip the top card of your deck: its **check** value must be at least difficulty + cards already in the pool (**progressive difficulty**). Short? Commit exactly enough ready foundations (or your character), +1 each. Fail: the card is discarded and your turn ends. So every check also mills a card, which makes deck size a clock.
- **Attack sequence:** Blitz (attacker, at the start), then both players alternate Enhances until both pass, then Block, then Damage.
- **Block:** play a card from hand as a block with the attack's speed + your block modifier + your progressive difficulty as the difficulty. A matching zone stops everything; an adjacent zone (mid touches both) takes half, rounded up. High and low do not touch.
- **End Phase:** foundations, assets and backups you played build into your stage (that is how your resource base grows); attacks that dealt damage may go to **momentum**, the spendable resource (EX, Powerful, Spend N momentum).
- **Ability anatomy:** `Label [restrictions] cost: effect`. Labels: Blitz (start of Enhance step, own attack), Enhance (once per attack per ability), Response (after a trigger), Form (your Combat Phase, outside attacks). Continuous text has no label. Committed cards cannot use abilities unless **Tenacious**.
- **Keyword abilities** (Appendix A): Breaker (after you block with it, rival's next check -X), Combo (needs the printed traits of the card(s) just before), Deflect, Desperation, Echo, Elusive, EX (+X speed per momentum), Flash (skip Enhance step), Frenzy, Gauge, Multiple, Powerful (+X damage per momentum), Reversal, Safe, Shift (build it flipped), Stun (rival commits X foundations), Tension, Terrain, Throw (half damage even when fully blocked), Unique. **Keyword traits** (no rules, only referenced): Ally, Armor, Charge, Fury, Kick, Punch, Ranged, Slam, Spell, Taunt, Tech, Titan, Weapon, XP and a few set ones.

## 5. Balance rules (from the numbers)

Figures are for the 1,022 2026-era cards unless marked "all".

**Check value is fixed by type.** Attacks check **3** (95%); foundations, assets, backups **5** (92%); actions mostly 5, sometimes 4 or 6. Cheaper or pricier cards do not change it. This is the main cost of an attack: it is a poor card to flip.

**Attacks:** difficulty 4 or 5 (82%). Speed + damage grows about 1.8 per difficulty point:

| Difficulty | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|
| Speed + damage | 5.8 | 7.6 | 9.3 | 11.2 | 11.3 | 12.8 |
| Typical split | 3 / 3 | 4 / 4 | 4 / 5 | 5 / 6 | 5 / 7 | 6 / 7 |

- Damage runs 0.5 to 1.5 above speed, more so at higher difficulty. Zones: mid 43%, high 31%, low 26%; low attacks are slightly weaker (-0.2).
- What a keyword costs in speed + damage (all eras, against the line above): **Throw -1.4**, Stun -0.6, Echo -0.6, Tension -0.5, an Enhance ability -0.2. Free or nearly so: EX, Powerful, Breaker, Weapon, Fury, Ranged, Punch, Kick (all within 0.2). No abilities at all buys only +0.3. Flash +0.7 and Quest +0.4 are the only "upside" keywords that are not paid for in stats.
- Ratings: EX 2 or 3, Powerful 2 or 3, Stun 1 (sometimes 2), Breaker 1, Quest 1.
- Attack block modifiers are small: +1 or +2 (about 1.7 on average), falling slightly at difficulty 6.

**Foundations:** difficulty 1 or 2 (85%), check 5, block +2 or +3 in any zone, mid slightly most often. Typical text is **one** ability, usually `Enhance Flip:` or `Response Flip:` with a small effect (+/-2 speed, +1 damage, draw 1, gain a keyword like Breaker: 1). Difficulty 3+ foundations carry stronger or two-part effects, or come with a discount clause ("gets -2 difficulty if ..."). Difficulty 0 exists only as plain guards (High Guard +3 high, no text).

**Costs on abilities** (2026 era): Flip 23%, Commit 17%, free 15%, Remove 5%, Sacrifice 5%, Drive N 4%, Mill N, Spend N momentum. Flip is the standard foundation cost; Commit is the standard character and asset cost.

**Actions and assets:** difficulty spread 0-5, check 5 (actions sometimes 4). Assets cannot be committed to checks, so they get stronger abilities. **Backups:** difficulty 1-5, stamina 3-6, usually Ally/Elusive/Unique, mostly a +0 mid block.

**Characters:** hand size trades against health:

| Hand size | 5 | 6 | 7 |
|---|---|---|---|
| Average health | 34 | 31 | 22 |
| Share | 12% | 60% | 26% |

Characters have 2 or 3 symbols (98%), one or two traits, and usually 2 abilities: a passive or Form plus an Enhance or Response. "First Form" and "First Enhance" (once per game openers) appear in newer sets.

**Symbols:** non-character cards have 1 symbol (68%) or 3 (25%); the 12 resources are used about evenly; Infinity is about 2%.

**Text box:** the rules text averages 130-180 characters for attacks; longer text is paid for with a higher difficulty (difficulty 8 attacks average 179).

### Quick guide for a fair custom card

- Attack: pick difficulty 4 or 5, check 3, speed + damage 7-8 (diff 4) or 9-10 (diff 5), block +1 or +2. Subtract about 1.5 for Throw or 0.5 for Stun.
- Foundation: difficulty 1-2, check 5, block +2/+3, one Flip or Commit ability with a small effect.
- Character: hand 6 and about 31 health, or hand 7 and about 22; 2-3 symbols; two abilities.

## 6. What this means for the template (proposed next steps)

1. Done 2026-11-12: the digits are the FT01 glyphs (`tools/uvs-study/make_digits.py`).
2. Done 2026-11-12: shields matched already; speed badges redrawn with the zone arrow. Damage starburst redrawn the same day: the print's uneven 11 points, traced from four FT01 cards, with the number moved to the print's spot (center 655, 756).
3. Per-set number styles as a Set info option, starting with the TK802 squared face (cuts already exist).
4. Optional: a "balance hint" in `/card` that warns when an attack's speed + damage is far off the table above.
