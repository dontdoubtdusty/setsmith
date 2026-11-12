# Summarizes stats and text patterns across all scraped cards, split into the boxed-label era and older.
import json, re, collections, statistics as S
C=json.load(open('study/uvs-cards/cards.json',encoding='utf-8'))
NEW={'tmnt01','sf60x','ggs01','kf0x','gdz01','mha09','tk802','sf62x','ft01','bl01','promo2026'}
def n(s):
    m=re.match(r'\s*([+-]?\d+)',s or ''); return int(m[1]) if m else None
def z(s):
    m=re.search(r'\{(high|mid|low)\}',s or ''); return m[1] if m else None
def tab(title,cnt,top=40):
    print(f'\n## {title}'); tot=sum(cnt.values())
    for k,v in cnt.most_common(top): print(f'  {k}: {v} ({100*v/tot:.0f}%)')
for era,sel in [('NEW (boxed labels)',lambda c:c['code'] in NEW),('ALL',lambda c:True)]:
    cs=[c for c in C if sel(c)]
    print('\n\n########',era,len(cs),'cards')
    tab('types',collections.Counter(c['type'] for c in cs))
    for t in ['attack','foundation','action','asset','backup']:
        g=[c for c in cs if c['type']==t]
        if not g: continue
        d=collections.Counter(n(c['stats'].get('Difficulty')) for c in g)
        k=collections.Counter(n(c['stats'].get('Control')) for c in g)
        b=collections.Counter((z(c['stats'].get('Block')),n(c['stats'].get('Block'))) for c in g)
        print(f'\n## {t} ({len(g)})\n  difficulty {sorted(d.items(),key=lambda x:(x[0] is None,x[0] or 0))}\n  check {sorted(k.items(),key=lambda x:(x[0] is None,x[0] or 0))}\n  block {sorted(b.items(),key=lambda x:str(x))}')
        # check by difficulty
        bd=collections.defaultdict(list)
        for c in g:
            dd,kk=n(c['stats'].get('Difficulty')),n(c['stats'].get('Control'))
            if dd is not None and kk is not None: bd[dd].append(kk)
        print('  mean check by difficulty', {x:round(S.mean(v),2) for x,v in sorted(bd.items())})
    at=[c for c in cs if c['type']=='attack']
    rows=[]
    for c in at:
        m=re.match(r'\s*(-?\d+)\s*\{(high|mid|low)\}\s*/\s*(-?\d+)',c['stats'].get('Attack',''))
        dd=n(c['stats'].get('Difficulty'))
        if m and dd is not None: rows.append((dd,int(m[1]),m[2],int(m[3]),c))
    print('\n## attacks: speed/damage by difficulty')
    by=collections.defaultdict(list)
    for r in rows: by[r[0]].append(r)
    for dd,v in sorted(by.items()):
        print(f'  diff {dd}: n={len(v)} speed {S.mean(x[1] for x in v):.2f} dmg {S.mean(x[3] for x in v):.2f} sum {S.mean(x[1]+x[3] for x in v):.2f}  textlen {S.mean(len(x[4]["text"]) for x in v):.0f}')
    tab('attack zone',collections.Counter(r[2] for r in rows))
    tab('attack speed',collections.Counter(r[1] for r in rows)); tab('attack damage',collections.Counter(r[3] for r in rows))
    ch=[c for c in cs if c['type']=='character']
    hs=[(n(c['stats'].get('Hand size')),n(c['stats'].get('Vitality'))) for c in ch]
    hs=[h for h in hs if None not in h]
    tab('character hand size',collections.Counter(h[0] for h in hs)); tab('character health',collections.Counter(h[1] for h in hs))
    print('  health by hand size',{k:round(S.mean(v[1] for v in hs if v[0]==k),1) for k in sorted({h[0] for h in hs})})
    tab('symbols per card (non-character)',collections.Counter(len(c['symbols']) for c in cs if c['type']!='character'))
    tab('symbols per character',collections.Counter(len(c['symbols']) for c in ch))
    tab('symbol use',collections.Counter(s for c in cs for s in c['symbols']))
    # ability labels and keywords (bold terms)
    bold=collections.Counter(re.sub(r'[:\s]+$','',b).strip() for c in cs for b in re.findall(r'\*\*(.*?)\*\*',c['text']))
    tab('bold terms (labels, keywords, traits)',bold,70)
    costs=collections.Counter()
    for c in cs:
        for m in re.finditer(r'\*\*(Enhance|Response|Blitz|Form)\s*\*\*\s*(\[[^\]]*\]\s*)*([^:\n]*):',c['text']):
            cost=m[3].strip() or '(free)'; cost=re.sub(r'\d+','N',cost)
            costs[cost[:50]]+=1
    tab('ability costs',costs,40)
