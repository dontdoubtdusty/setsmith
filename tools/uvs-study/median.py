# Builds per-set, per-type median and spread images: fixed frame pixels have low spread.
import csv, sys, numpy as np
from PIL import Image
rows=list(csv.DictReader(open('study/uvs-cards/manifest.tsv',encoding='utf-8'),delimiter='\t'))
sets=sys.argv[1].split(','); types=sys.argv[2].split(',')
for s in sets:
  for t in types:
    fs=[r['file'] for r in rows if r['code']==s and r['type']==t]
    if len(fs)<3: continue
    ims=[]
    for f in fs:
      im=Image.open('study/uvs-cards/'+f).convert('RGB')
      if im.size!=(744,1039): 
        if abs(im.size[0]/im.size[1]-744/1039)>0.01: continue
        im=im.resize((744,1039),Image.LANCZOS)
      ims.append(np.asarray(im))
    a=np.stack(ims)
    med=np.empty(a.shape[1:],np.uint8); sd=np.empty(a.shape[1:3],np.float32)
    for y in range(0,1039,64):
      blk=a[:,y:y+64].astype(np.float32)
      med[y:y+64]=np.median(blk,axis=0).astype(np.uint8)
      sd[y:y+64]=blk.std(axis=0).mean(axis=2)
    Image.fromarray(med).save(f'study/work/med-{s}-{t}.png')
    Image.fromarray(np.clip(255-sd*3,0,255).astype(np.uint8)).save(f'study/work/sd-{s}-{t}.png')
    print(s,t,len(ims))
