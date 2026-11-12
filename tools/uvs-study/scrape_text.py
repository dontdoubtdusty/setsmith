# Pulls card text and stats for every set in the manifest from uvsultra.online listing pages into cards.json.
import csv, re, json, time, html, urllib.request, urllib.parse
ROOT='study/uvs-cards'
rows=list(csv.DictReader(open(f'{ROOT}/manifest.tsv',encoding='utf-8'),delimiter='\t'))
sets=sorted({(r['set_id'],r['set_name']) for r in rows},key=lambda s:int(s[0]))
def fetch(sid,page):
    data=urllib.parse.urlencode([('extension[]',sid),('format','all'),('page',page)]).encode()
    req=urllib.request.Request('https://uvsultra.online/listing_cards.php',data=data,headers={'User-Agent':'Mozilla/5.0'})
    return urllib.request.urlopen(req,timeout=60).read().decode('utf-8','replace')
def clean(s):
    s=re.sub(r'<strong>\s*(.*?)\s*</strong>',r'**\1**',s,flags=re.S)
    s=re.sub(r'<img[^>]*alt="([^"]*)"[^>]*>',r'{\1}',s)
    s=re.sub(r'<img[^>]*title="([^"]*)"[^>]*>',r'{\1}',s)
    s=re.sub(r'<br\s*/?>','\n',s)
    s=re.sub(r'<[^>]+>','',s)
    return html.unescape(re.sub(r'[ \t]+',' ',s)).strip()
out=[]; tips={}
for sid,name in sets:
    page=0; seen=set()
    while True:
        h=fetch(sid,page)
        blocks=re.split(r'<div class="card (?:even|odd)">',h)[1:]
        new=0
        for b in blocks:
            m=re.search(r"preview2\('([^']*)', '([^']*)', '([^']*)'\)",b)
            if not m or (m[1],m[2]) in seen: continue
            seen.add((m[1],m[2])); new+=1
            title=re.search(r'<h1>(.*?)</h1>',b,re.S)
            imp=re.search(r'card-important-info">(.*?)</div>',b,re.S)
            cd2=re.search(r'card_division cd2"[^>]*>(.*?)</div>',b,re.S)
            cd3=re.search(r'card_division cd3"[^>]*>(.*?)</div>',b,re.S)
            for t in re.findall(r'title="([^"]{25,})"',b): 
                k=t.split(':')[0][:40]; tips.setdefault(html.unescape(t),0); tips[html.unescape(t)]+=1
            stats={}; syms=[]
            if cd3:
                txt=cd3[1]
                syms=re.findall(r'icons/([a-z0-9_]+)\.png',txt.split('<br')[0])
                for line in clean(txt).split('\n'):
                    if ':' in line: k,v=line.split(':',1); stats[k.strip()]=v.strip()
            out.append({'set_id':sid,'set':name,'code':m[1],'number':m[2],'type':m[3],
                'name':clean(title[1]) if title else '','rarity':clean(imp[1]).split('\n')[-1] if imp else '',
                'text':clean(cd2[1]) if cd2 else '','symbols':syms,'stats':stats})
        if new==0: break
        page+=1; time.sleep(0.6)
    print(sid,name,len(seen),flush=True)
json.dump(out,open(f'{ROOT}/cards.json','w',encoding='utf-8'),ensure_ascii=False,indent=1)
json.dump(tips,open(f'{ROOT}/tooltips.json','w',encoding='utf-8'),ensure_ascii=False,indent=1)
print('DONE',len(out))
