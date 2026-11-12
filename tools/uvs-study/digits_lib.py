# Shared helpers for cutting UVS stat digits out of 2x scans: find the black-outlined glyph,
# its counters (holes), and build the fill / outline / rim layers.
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

def lum(a): return a[...,0]*0.299+a[...,1]*0.587+a[...,2]*0.114

def glyph_mask(rgb, alpha=None, dark=75):
    """Return (S, counters): S = black outline plus everything it encloses (holes included),
    counters = enclosed areas that are not the fill (show the rim color on real cards)."""
    L=lum(rgb); h,w=L.shape
    nondark=(L>=dark)
    if alpha is not None: nondark|=(alpha<128)
    # flood the outside from every border pixel over non-dark pixels
    m=Image.fromarray(np.where(nondark,255,0).astype(np.uint8))
    m=m.copy()
    pad=Image.new('L',(w+2,h+2),255); pad.paste(m,(1,1))
    ImageDraw.floodfill(pad,(0,0),128)
    outside=np.asarray(pad)[1:-1,1:-1]==128
    inner=~outside
    # keep the component containing the center
    im=Image.fromarray(np.where(inner,255,0).astype(np.uint8)).copy()
    ys,xs=np.nonzero(inner); cy,cx=h/2,w/2
    i=np.argmin((xs-cx)**2+(ys-cy)**2)
    ImageDraw.floodfill(im,(int(xs[i]),int(ys[i])),128)
    S=np.asarray(im)==128
    # counters: non-dark pieces inside S with almost no near-white pixels
    white=(L>215)&((rgb.max(axis=2)-rgb.min(axis=2))<40)
    cand=Image.fromarray(np.where(S&nondark,255,0).astype(np.uint8))
    counters=np.zeros_like(S)
    arr=np.asarray(cand).copy()
    while True:
        ys,xs=np.nonzero(arr==255)
        if len(xs)==0: break
        im2=Image.fromarray(arr).copy(); ImageDraw.floodfill(im2,(int(xs[0]),int(ys[0])),100); arr2=np.asarray(im2)
        comp=arr2==100
        if white[comp].mean()<0.02 and comp.sum()>30: counters|=comp
        arr=np.where(comp,50,arr2).astype(np.uint8)
    return S, counters

def bbox(m):
    ys,xs=np.nonzero(m); return xs.min(),ys.min(),xs.max()+1,ys.max()+1

def dilate(mask, r):
    """Round dilation by r pixels (OR of shifted copies over a disk)."""
    h,w=mask.shape; out=np.zeros_like(mask)
    p=np.pad(mask,r)
    for dy in range(-r,r+1):
        for dx in range(-r,r+1):
            if dx*dx+dy*dy<=r*r: out|=p[r+dy:r+dy+h, r+dx:r+dx+w]
    return out
