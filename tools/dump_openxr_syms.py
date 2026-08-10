#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Dump Pxr_ eye-tracking / foveation exported symbols from libopenxr_api.so
import sys
from elftools.elf.elffile import ELFFile

path = r"C:\Users\wzy\.openclaw\workspace\paper-etfr\svclib\libopenxr_api.so"
with open(path, 'rb') as f:
    elf = ELFFile(f)
    dynsym = elf.get_section_by_name('.dynsym')
    if dynsym is None:
        print("no .dynsym"); sys.exit(1)
    print("=== Pxr eye/foveation/gaze exported symbols ===")
    for sym in dynsym.iter_symbols():
        n = sym.name or ''
        if any(k in n for k in ('Eye', 'Gaze', 'Foveat', 'Pupil')):
            print(f"  0x{sym['st_value']:08x}  size={sym['st_size']:<6} {n}")
print("done")
