#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Recover pxr_eyepose field offsets from the logging code in GetEyeTrackingData
# The log reads specific float offsets and fcvtz to double -> these ARE the struct fields
from elftools.elf.elffile import ELFFile

path = r"C:\Users\wzy\.openclaw\workspace\paper-etfr\svclib\libeyetrackingclient.pxr.so"
with open(path,'rb') as f:
    elf = ELFFile(f)
    # pull all rodata strings to hint field names
    for sec in elf.iter_sections():
        if sec['sh_type'] == 'SHT_STRTAB' or sec.name in ('.rodata','.data.rel.ro'):
            try:
                d = sec.data()
                # find gaze-related ascii
                idx = 0
                while True:
                    idx = d.find(b'gaze', idx)
                    if idx < 0: break
                    s = d[idx:idx+64].split(b'\x00')[0]
                    print(f"  [{sec.name}] 0x{sec['sh_addr']+idx:x}: {s.decode('ascii','replace')}")
                    idx += 1
            except: pass

print("\n=== float field offsets read in the log block (pxr_eyepose) ===")
print("""
From disasm of GetEyeTrackingData log section (0x13658-0x136c8), float fields:
  [x19+0x50] s16  , [x19+0x54] s1   -> pair (offset 0x50)
  [x19+0x58] s5                       -> 0x58
  [x19+0x64] s0  , [x19+0x68] s1      -> pair (offset 0x64)
  [x19+0x70] s3  , [x19+0x74] s4      -> pair (offset 0x70)
  [x19+0x7c] s6  , [x19+0x80] s7      -> pair+ (offset 0x7c)
  [x19+0x84] s17                      -> 0x84
  [x19+0x48] s18 , [x19+0x4c] s19     -> pair (offset 0x48)
These are the fields logged (eye pose / gaze vectors)
""")
print("done")
