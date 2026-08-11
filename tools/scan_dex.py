#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Scan systemext dex files for NS_APP / virtual display / resolution strings
import os, re, sys

base = r"C:\Users\wzy\Documents\pico4-paper-etfr\tools\SystemExt_x"
targets = [b'NS_APP', b'NSApp', b'1602', b'1600', b'VirtualDisplay', b'createVirtualDisplay',
           b'window_size', b'windowSize', b'2d', b'flat', b'resWidth', b'resHeight',
           b'globalWindowWidth', b'displayWidth', b'NS_WINDOW', b'caption', b'NS_']
hit = {t:0 for t in targets}
files = []
for f in os.listdir(base):
    if f.endswith('.dex'):
        files.append(os.path.join(base,f))
print("scanning", files)
for p in files:
    d = open(p,'rb').read()
    for t in targets:
        c = d.count(t)
        if c: hit[t]+=c
print("\n=== hit counts ===")
for t,c in hit.items():
    if c: print(f"  {t.decode('ascii','replace')}: {c}")
# show context of a few interesting ones
print("\n=== context around NS_APP / 1602 ===")
d = open(os.path.join(base,'classes.dex'),'rb').read()
for key in (b'NS_APP', b'1602', b'createVirtualDisplay', b'NS_WINDOW'):
    idx = 0
    n=0
    while n<4:
        i = d.find(key, idx)
        if i<0: break
        ctx = d[i-40:i+80]
        print(f"[{key.decode()}] ...{ctx!r}...")
        idx=i+1; n+=1
print("done")
