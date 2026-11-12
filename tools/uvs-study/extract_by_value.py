# Cuts official frame graphics out of the scans, grouping cards by their real stat values
# (from cards.json), so each group shows the identical badge over different art.
# Color = per-pixel median of the group; alpha = how steady the pixel is across the group.
import json, sys, os, re, collections, numpy as np
from PIL import Image
ROOT='study/uvs-cards'
cards=json.load(open(f'{ROOT}/cards.json',encoding='utf-8'))
SETS=sys.argv[1].split(',')
MIN=int(sys.argv[2]) if len(sys.argv)>2 else 4

def num(s):
    m=re.match(r'\s*([+-]?\d+|X|\*)',s or ''); return m[1] if m else None
def zone(s):
    m=re.search(r'\{(high|mid|low)\}',s or ''); return m[1] if m else None

# Region of each element in 744 x 1039 card units, and the key that groups identical badges.
def jobs(c):
    st=c['stats']; t=c['type']; out=[]
    if t in ('attack','foundation','action','asset','backup'):
        d=num(st.get('Difficulty')); 
        if d: out.append((f'difficulty/{t}-{d}',(4,2,170,160)))
        k=num(st.get('Control'))
        if k: out.append((f'check/{t}-{k}',(570,800,740,985)))
        b=st.get('Block','')
        if num(b) and zone(b): out.append((f'block/{t}-{zone(b)}-{num(b).lstrip("+")}',(596,10,742,140)))
    if t=='attack':
        a=st.get('Attack','')
        m=re.match(r'\s*(\S+)\s*\{(high|mid|low)\}\s*/\s*(\S+)',a)
        if m:
            out.append((f'speed/{m[2]}-{m[1]}',(585,585,742,712)))
            out.append((f'damage/{m[3]}',(585,690,742,815)))
    if t=='character':
        h=num(st.get('Hand size')); v=num(st.get('Vitality'))
        if h: out.append((f'hand-size/{h}',(560,540,742,720)))
        if v: out.append((f'health/{v}',(560,660,742,850)))
    if t=='backup':
        s=num(st.get('Stamina'))
        if s: out.append((f'stamina/{s}',(560,620,742,860)))
    return out

for code in SETS:
    groups=collections.defaultdict(list)
    for c in cards:
        if c['code']!=code: continue
        for key,box in jobs(c): groups[key].append((c,box))
    for key,items in sorted(groups.items()):
        if len(items)<MIN: continue
        stack=[]
        for c,box in items:
            f=f"{ROOT}/{c['code']}/{c['number']}.jpg"
            if not os.path.exists(f): continue
            im=Image.open(f).convert('RGB'); s=im.size[0]/744
            if abs(im.size[1]/s-1039)>3: continue
            cr=im.crop(tuple(round(v*s) for v in box))
            if stack: cr=cr.resize((stack[0].shape[1],stack[0].shape[0]),Image.LANCZOS)
            stack.append(np.asarray(cr).astype(np.float32))
        if len(stack)<MIN: continue
        st=np.stack(stack)
        med=np.median(st,axis=0); sd=st.std(axis=0).mean(axis=2)
        alpha=np.clip((24-sd)/(24-7),0,1)
        os.makedirs(f'study/work/gfx/{code}/'+os.path.dirname(key),exist_ok=True)
        Image.fromarray(np.dstack([med,alpha*255]).astype(np.uint8),'RGBA').save(f'study/work/gfx/{code}/{key}-n{len(stack)}.png')
    print(code,'done',flush=True)
