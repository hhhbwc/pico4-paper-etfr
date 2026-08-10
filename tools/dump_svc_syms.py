#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Full symbol inventory of libpxreyetrackingservice.so (mangled + demangled)
from elftools.elf.elffile import ELFFile
import re

path = r"C:\Users\wzy\.openclaw\workspace\paper-etfr\svclib\libpxreyetrackingservice.so"
def demangle(n):
    # crude itanium demangle: pull args template
    s = re.sub(r'^_ZN\d+', '', n)
    # split into [ns, cls, method] by \d+len
    def parse(s):
        parts = []
        i = 0
        while i < len(s):
            # read digits
            m = re.match(r'(\d+)(.*)', s[i:])
            if not m: 
                # rest is e.g. suffix E, or qualifiers
                break
            ln = int(m.group(1)); body = m.group(2)
            parts.append(body[:ln]); i += ln
            s = f"{body[ln:]}"
        return parts
    parts = parse(s)
    if len(parts) >= 1:
        return "::".join(parts)
    return n

with open(path,'rb') as f:
    elf = ELFFile(f)
    print("=== arch", elf.get_machine_arch(), "===")
    for secname in ('.dynsym','.symtab'):
        sec = elf.get_section_by_name(secname)
        if not sec: continue
        print(f"--- {secname} ---")
        for sym in sec.iter_symbols():
            n = sym.name or ''
            if not n: continue
            if any(k in n for k in ('TrackingService','DataBuffer','CameraManager','Algorithm','hasEyeCamera','GetData','SetData','OpenCamera','StartAlgorithm','Foveat','gaze','Eye','Support','CameraImageListener','SharedMemory','Parcelable','Gaze','Pupil','Driver','GetTrackingData')):
                a = sym['st_value']; sz = sym['st_size']
                print(f"  0x{a:08x} sz={sz:<5} {demangle(n)}")
    print("done")
