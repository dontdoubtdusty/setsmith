# Keeps only the badge: the steady region connected to the crop's center, grown a little, trimmed to its bounds.
import glob, os, numpy as np
from PIL import Image, ImageDraw, ImageFilter
for f in glob.glob('study/work/gfx/*/*/*.png'):
    out=f.replace('study/work/gfx/','study/uvs-graphics/')
    im=Image.open(f); a=np.asarray(im).copy()
    solid=Image.fromarray(((a[:,:,3]>140)*255).astype(np.uint8))
    h,w=a.shape[:2]; cx,cy=w//2,h//2
    # seed at the nearest solid pixel to the center
    ys,xs=np.nonzero(np.asarray(solid)); 
    if len(xs)==0: continue
    i=np.argmin((xs-cx)**2+(ys-cy)**2)
    m=solid.convert('L'); ImageDraw.floodfill(m,(int(xs[i]),int(ys[i])),128)
    keep=Image.fromarray(((np.asarray(m)==128)*255).astype(np.uint8)).filter(ImageFilter.MaxFilter(5)).filter(ImageFilter.GaussianBlur(0.8))
    a[:,:,3]=np.minimum(a[:,:,3],np.asarray(keep))
    res=Image.fromarray(a,'RGBA'); bb=res.getbbox()
    os.makedirs(os.path.dirname(out),exist_ok=True)
    res.crop(bb).save(out)
print('ok')
