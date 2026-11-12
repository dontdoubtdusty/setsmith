# Builds the template's stat digit images from the official FT01 glyphs (2x scans):
#   num-d-<n>.png   difficulty digit: real fill + white line + black outline, holes transparent
#   num-c-<n>.png   check digit: same glyph with the real blue check gradient moved onto it
#   num-rim-<n>.png red silhouette 5.5 units around the outline (the template recolors it to the type color)
# Images are 260 x 320 px = 130 x 160 card units at 2 px per unit, glyph placed exactly as on the real card
# (difficulty box at card x 24, y 10). 9 is the 6 turned 180 degrees with its gradient put back right side up.
import sys, os, json, numpy as np
from PIL import Image, ImageFilter
sys.path.insert(0, os.path.dirname(__file__))
from digits_lib import lum, glyph_mask, dilate

OUT = sys.argv[1] if len(sys.argv) > 1 else 'study/work/digits'
CARDS = json.load(open('study/uvs-cards/cards.json', encoding='utf-8'))
BOX_D = (24, 10)            # difficulty image box, card units (130 x 160)
CROP_D = (4, 2, 170, 160)   # where the difficulty glyph is read from
CROP_C = (570, 800, 740, 985)
RIM = 11                    # rim width in px at 2x
os.makedirs(OUT, exist_ok=True)

def cards_with(stat, value):
    return [c for c in CARDS if c['code'] == 'ft01' and (c['stats'].get(stat) or '').strip() == value]

def median_crop(cs, crop):
    st = [np.asarray(Image.open(f"study/uvs-cards/ft01/{c['number']}.jpg").convert('RGB').crop(tuple(v * 2 for v in crop))).astype(np.float32) for c in cs]
    return np.median(np.stack(st), axis=0)

def layers(rgb):
    """Glyph mask S (outline and all it encloses), holes, and fill pixels (colored, not white or black)."""
    S, holes = glyph_mask(rgb)
    L = lum(rgb); sat = rgb.max(axis=2) - rgb.min(axis=2)
    fill = S & ~holes & (L >= 75) & (sat > 50)
    return S, holes, fill

def profile(rgb, S, fill, bins=64):
    """Mean fill color in bins down the glyph height (top = 0, bottom = 1)."""
    ys = np.nonzero(S)[0]; y0, y1 = ys.min(), ys.max()
    t = (np.arange(rgb.shape[0]) - y0) / max(1, y1 - y0)
    prof = np.zeros((bins, 3)); cnt = np.zeros(bins)
    yy, xx = np.nonzero(fill)
    b = np.clip((t[yy] * bins).astype(int), 0, bins - 1)
    np.add.at(prof, b, rgb[yy, xx]); np.add.at(cnt, b, 1)
    prof /= np.maximum(cnt, 1)[:, None]
    # fill empty bins from neighbours
    good = cnt > 0
    for i in range(bins):
        if not good[i]:
            j = np.argmin(np.abs(np.where(good)[0] - i)); prof[i] = prof[np.where(good)[0][j]]
    return prof, t

def at(prof, t):
    """Profile color at heights t (0..1), interpolated so there are no bands."""
    x = (np.arange(len(prof)) + 0.5) / len(prof)
    return np.stack([np.interp(t, x, prof[:, k]) for k in range(3)], axis=-1)

def regradient(rgb, S, fill, src_prof, dst_prof, flip=False, texture=True):
    """Swap one vertical gradient for another, keeping the speed-line texture as a brightness ratio."""
    out = rgb.copy()
    ys = np.nonzero(S)[0]; y0, y1 = ys.min(), ys.max()
    t = np.clip((np.arange(rgb.shape[0]) - y0) / max(1, y1 - y0), 0, 1)
    yy, xx = np.nonzero(fill)
    ts = 1 - t[yy] if flip else t[yy]
    ratio = lum(rgb[yy, xx]) / np.maximum(lum(at(src_prof, ts)), 1) if texture else 1.0
    out[yy, xx] = np.clip(at(dst_prof, t[yy]) * np.atleast_1d(ratio)[:, None], 0, 255)
    return out

def soft(mask, blur=0.7):
    return np.asarray(Image.fromarray((mask * 255).astype(np.uint8)).filter(ImageFilter.GaussianBlur(blur))).astype(np.float32)

def paste(dst, src, ox, oy):
    """Copy src into dst with its top-left at (ox, oy), clipping at the edges."""
    H, W = dst.shape[:2]; h, w = src.shape[:2]
    x0, y0 = max(0, ox), max(0, oy); x1, y1 = min(W, ox + w), min(H, oy + h)
    if x1 > x0 and y1 > y0: dst[y0:y1, x0:x1] = src[y0 - oy:y1 - oy, x0 - ox:x1 - ox]

def save(rgb, S, holes, name_d, name_rim=None, crop=CROP_D):
    """Write the glyph (and its rim) into the 260 x 320 box, placed as on the real card."""
    W, H = 260, 320
    # the box sits at the same spot relative to the glyph for difficulty and check
    bx, by = crop[0] + BOX_D[0] - CROP_D[0], crop[1] + BOX_D[1] - CROP_D[1]
    ox, oy = (crop[0] - bx) * 2, (crop[1] - by) * 2
    img = np.zeros((H, W, 4), np.float32)
    paste(img, np.dstack([rgb, soft(S & ~holes)]), ox, oy)
    Image.fromarray(img.astype(np.uint8), 'RGBA').save(f'{OUT}/{name_d}')
    if name_rim:
        r = np.zeros((H, W, 4), np.float32); r[..., 0] = 255
        rim = soft(dilate(S, RIM), 0.9)
        paste(r[..., 3:4], rim[..., None], ox, oy)
        Image.fromarray(r.astype(np.uint8), 'RGBA').save(f'{OUT}/{name_rim}')

# Real difficulty glyphs, 0-8
glyphs = {}
for d in '012345678':
    cs = cards_with('Difficulty', d)
    rgb = median_crop(cs, CROP_D)
    glyphs[d] = (rgb,) + layers(rgb)
    print('digit', d, 'from', len(cs), 'cards')

# Gradients: difficulty from the well-sampled 4 and 5; check from the real check 3 and 5
dprof = np.mean([profile(glyphs[d][0], glyphs[d][1], glyphs[d][3])[0] for d in '45'], axis=0)
cprofs = []
for v in '35':
    rgb = median_crop(cards_with('Control', v), CROP_C); S, holes, fill = layers(rgb)
    cprofs.append(profile(rgb, S, fill)[0])
    save(rgb, S, holes, f'real-check-{v}.png', crop=CROP_C)
cprof = np.mean(cprofs, axis=0)

# 9 = 6 turned 180 degrees, gradient put back
rgb6, S6, h6, f6 = glyphs['6']
rot = lambda m: m[::-1, ::-1]
rgb9 = regradient(rot(rgb6), rot(S6), rot(f6), dprof, dprof, flip=True)
# keep the glyph where the 6 sat: shift so its bounding box matches the 6's
glyphs['9'] = (rgb9, rot(S6), rot(h6), rot(f6))

def drawn_glyph(ch, ref):
    """X and * never appear on real cards: draw them in Figtree Black Italic with the real layer widths
    (white line 5 px, black outline 11 px), the same height and center as the reference digit,
    and the reference digit's speed-line texture."""
    from PIL import ImageDraw, ImageFont
    rgb_r, S_r, holes_r, fill_r = ref
    h, w = S_r.shape
    ys, xs = np.nonzero(S_r); cy, cx = (ys.min() + ys.max()) / 2, (xs.min() + xs.max()) / 2
    target = ys.max() - ys.min() - 2 * 16          # fill height inside white + black
    if ch == '*': target *= 0.62                    # a star is shorter than a digit
    font = ImageFont.truetype('templates/fonts/Figtree-BlackItalic.ttf', 400)
    big = Image.new('L', (900, 900), 0); ImageDraw.Draw(big).text((200, 100), ch, font=font, fill=255)
    big = big.crop(big.getbbox())
    maxw = xs.max() - xs.min() - 2 * 16            # no wider than the reference digit
    kh = target * (0.85 if ch == 'X' else 1) / big.size[1]
    kw = min(kh, maxw / big.size[0])                # narrow it rather than shrink it
    g = big.resize((max(1, round(big.size[0] * kw)), max(1, round(big.size[1] * kh))), Image.LANCZOS)
    F = np.zeros((h, w), bool); gm = np.asarray(g) > 127
    oy, ox = int(cy - gm.shape[0] / 2), int(cx - gm.shape[1] / 2)
    F[oy:oy + gm.shape[0], ox:ox + gm.shape[1]] = gm
    Wt = dilate(F, 5); S = dilate(Wt, 11)
    rgb = np.zeros((h, w, 3), np.float32)
    rgb[Wt] = 255
        # texture: the reference fill's brightness ratio, sampled at the same spot
    ratio = np.ones((h, w))
    ref_ratio = lum(rgb_r) / np.maximum(lum(at(dprof, np.clip((np.arange(h)[:, None] - ys.min()) / (ys.max() - ys.min()), 0, 1))), 1)
    ratio[fill_r] = ref_ratio[fill_r]
    t = np.clip((np.arange(h)[:, None] - ys.min()) / (ys.max() - ys.min()), 0, 1) * np.ones((1, w))
    rgb[F] = np.clip(at(dprof, t[F]) * np.clip(ratio[F], 0.75, 1.2)[:, None], 0, 255)
    return rgb, S, np.zeros_like(S), F

glyphs['x'] = drawn_glyph('X', glyphs['4'])
glyphs['star'] = drawn_glyph('*', glyphs['4'])

for d, (rgb, S, holes, fill) in sorted(glyphs.items()):
    # line the 9 up with the 6 (rotation moves it inside the crop)
    if d == '9':
        y6, x6 = [v.min() for v in np.nonzero(S6)]; y9, x9 = [v.min() for v in np.nonzero(S)]
        dy, dx = y6 - y9, x6 - x9
        sh = lambda m: np.roll(np.roll(m, dy, 0), dx, 1)
        rgb, S, holes, fill = sh(rgb), sh(S), sh(holes), sh(fill)
    save(rgb, S, holes, f'num-d-{d}.png', f'num-rim-{d}.png')
    save(regradient(rgb, S, fill, dprof, cprof), S, holes, f'num-c-{d}.png')
print('done ->', OUT)
